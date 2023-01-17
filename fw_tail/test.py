#!/usr/bin/env python3
#
# Test harness
#
import argparse
import can
from rich.traceback import install
import struct
import time
import unittest

BRAKE_CTRL_ID = 0x0a8
LIGHT_CTRL_ID = 0x21a
TAIL_STATUS_ID = 0x781
DEBUG_ID = 0x7ff
ACK_ID = 0x1ffffff0
CONSOLE_ID = 0x1ffffffe

# Messages that we care about receiving
RECEIVE_FILTER = [
    ACK_ID, TAIL_STATUS_ID
]


class MessageError(Exception):
    """a received message was not as expected"""
    pass


class ModuleError(Exception):
    """the module did something unexpected"""
    pass


class TXMessage(object):
    """
    Abstract for messages that will be sent.

    Concrete classes set self._format, and pass corresponding
    arguments to __init__.
    """
    def __init__(self, message_id, is_extended, *args):
        self.raw = can.Message(arbitration_id=message_id,
                               is_extended_id=is_extended,
                               data=struct.pack(self._format, *args))

    def __str__(self):
        return f'{self.raw}'


class MSG_brake(TXMessage):
    """BMW brake (etc.) status message"""
    _format = '>BHHBBB'

    def __init__(self, brake_state):
        super().__init__(BRAKE_CTRL_ID,
                         False,             # non-extended
                         0x54,              # magic
                         0,                 # actual torque
                         0,                 # rounded torque
                         240,               # clutch not depressed
                         0x0f,              # magic
                         32 if brake_state else 3)


class MSG_lights(TXMessage):
    """BMW light control message"""
    _format = '>BBB'

    def __init__(self, brake_light=False, tail_light=False, rain_light=False, reverse=False):
        super().__init__(LIGHT_CTRL_ID,
                         False,
                         ((0x80 if brake_light else 0) |
                          (0x04 if tail_light else 0) |
                          (0x40 if rain_light else 0)),
                         (0x01 if reverse else 0),
                         0xf7)


class MSG_debug(TXMessage):
    """enable debug mode"""
    _format = '>BBBBBB'

    def __init__(self, enable):
        super().__init__(DEBUG_ID,
                         False,
                         ord('d'),
                         ord('e'),
                         ord('b'),
                         ord('u'),
                         ord('g'),
                         1 if enable else 0)


class RXMessage(object):
    """
    Abstract for messages that have been received.

    Concretes set self._format to struct.unpack() received bytes,
    and self._filter to a list of tuple-per-unpacked-item with each
    tuple containing True/False and, if True, the required value.
    """
    def __init__(self, expected_id, raw):
        self.raw = raw

        if raw.arbitration_id != expected_id:
            raise MessageError(f'expected reply with ID {expected_id} '
                               f'but got {raw}')
        expected_dlc = struct.calcsize(self._format)
        if raw.dlc != expected_dlc:
            raise MessageError(f'expected reply with length {expected_dlc} '
                               f'but got {raw}')

        self._values = struct.unpack(self._format, raw.data)
        for (index, (check, value)) in enumerate(self._filter):
            if check and value != self._values[index]:
                raise MessageError(f'reply field {index} is '
                                   f'0x{self._values[index]} '
                                   f'but expected {value}')

    @classmethod
    def len(self):
        return struct.calcsize(self._format)


class MSG_ack(RXMessage):
    """broadcast message sent by module on power-up, reboot or crash"""
    _format = '>BIBH'
    _filter = [(False, 0),
               (False, 0),
               (False, 0),
               (False, 0)]
    REASON_MAP = {
        0x00: 'power-on',
        0x01: 'reset',
        0x11: 'low-voltage reset',
        0x21: 'clock lost',
        0x31: 'address error',
        0x41: 'illegal opcode',
        0x51: 'watchdog timeout'
    }
    STATUS_MAP = {
        0: 'OK',
        4: 'NO PROG'
    }

    def __init__(self, raw):
        super().__init__(expected_id=ACK_ID,
                         raw=raw)
        (self.reason_code,
         self.module_id,
         self.status_code,
         self.sw_version) = self._values
        try:
            self.reason = self.REASON_MAP[self.reason_code]
        except KeyError:
            self.reason = 'unknown'
        try:
            self.status = self.STATUS_MAP[self.status_code]
        except KeyError:
            self.status = "unknown"


class MSG_tail_status(RXMessage):
    """
    Status message from the tail module.
    """
    _format = '>BB'
    _filter = [(False, 0),
               (False, 0)]
    _bits = {
        'brake_on': (0, 0),
        'brake_requested': (0, 1),
        'lights_on': (0, 2),
        'lights_requested': (0, 3),
        'rain_on': (0, 4),
        'rain_requested': (0, 5),
        'reverse_on': (0, 6),
        'reverse_requested': (0, 7),
        'engine_running': (1, 0),
        'can_idle': (1, 1),
        'debug_enable': (1, 2),
    }

    def __init__(self, raw):
        super().__init__(expected_id=TAIL_STATUS_ID,
                         raw=raw)

    def __getattr__(self, name):
        try:
            (index, shift) = self._bits[name]
            return True if (self._values[index] & (1 << shift)) else False
        except KeyError:
            raise AttributeError


class ManualPower(object):
    def set_power_off(self):
        print('POWER: turn module power off')

    def set_power_t30(self):
        print('POWER: turn module power on (T30 only)')

    def set_power_t30_t15(self):
        print('POWER: turn module power on (T30 and T15)')


class AnaGatePower(object):
    def __init__(self, can_bus):
        self._connection = can_bus.connection

    def set_power_off(self):
        self._connection.set_analog_out(1, 0)
        self._connection.set_analog_out(2, 0)
        self._power_on = False

    def set_power_t30(self):
        self._connection.set_analog_out(1, 12000)
        self._connection.set_analog_out(2, 0)
        self._power_on = True

    def set_power_t30_t15(self):
        self._connection.set_analog_out(1, 12000)
        self._connection.set_analog_out(2, 12000)
        self._power_on = True


class Interface(object):
    def __init__(self, args):

        self._power_on = False
        self._verbose = args.verbose

        self._bus = can.Bus(interface=args.interface_name,
                            channel=args.interface_channel,
                            bitrate=args.bitrate * 1000)

        if args.interface_name == 'anagate':
            self._power_agent = AnaGatePower(self._bus)
        else:
            self._power_agent = ManualPower()

    def __del__(self):
        self._bus.shutdown()

    def start(self):
        self.set_power_off()
        time.sleep(0.25)
        self.drain()
        self.set_power_t30()

        # wait for the module to sign on
        while True:
            rsp = self.recv(5)
            if rsp is None:
                raise ModuleError('no power-on message from module')
            try:
                signon = MSG_ack(rsp)
                break
            except MessageError as e:
                raise ModuleError(f'unexpected power-on message from module: {rsp}')

    def stop(self):
        self.set_power_off()

    def get_console_data(self):
        """fetch console packets"""
        while True:
            msg = self.recv(1)
            if msg is not None:
                try:
                    status = MSG_ack(msg)
                    print(f'module reset due to {status.reason}')
                except MessageError:
                    pass
                if msg.arbitration_id == CONSOLE_ID:
                    return msg.data

    def send(self, message):
        """send the message"""

        self._trace(f'CAN TX: {message}')
        self._bus.send(message.raw)

    def recv(self, timeout):
        """
        wait for a message
        """
        now = time.time()
        deadline = now + timeout
        while time.time() < deadline:
            msg = self._bus.recv(timeout=deadline - time.time())
            if msg is not None:
                if msg.arbitration_id in RECEIVE_FILTER:
                    return msg

    def set_power_off(self):
        self._power_agent.set_power_off()

    def set_power_t30(self):
        self._power_agent.set_power_t30()

    def set_power_t30_t15(self):
        self._power_agent.set_power_t30_t15()

    def drain(self):
        """
        Try to drain any buffered CAN messages - give up if the bus is
        too chatty.
        """
        count = 100
        while count:
            count -= 1
            msg = self.recv(0.001)
            if msg is None:
                return True
        return False

    def _trace(self, msg):
        if self._verbose:
            print(msg)


class TailTests(unittest.TestCase):
    def setUp(self):
        self.interface = Interface(g_args)
        self.interface.start()
        # wait for a status message (in addition to the module sign-on)
        rsp = self.interface.recv(1)
        # enable debug mode
        self.interface.send(MSG_debug(True))

    def tearDown(self):
        self.interface.stop()

    def test_00_get_status(self):
        """
        Verify that the module is sending a status message and that it looks sane immediately after reset
        """
        rsp = self.interface.recv(1)
        self.assertNotEqual(rsp, None, 'timed out waiting for status message')
        self.assertEqual(rsp.arbitration_id, TAIL_STATUS_ID, f'unexpected ID {rsp.arbitration_id:#x} on bus')
        self.assertEqual(rsp.dlc, 2, f'unexpected status size {rsp.dlc}')
        status = MSG_tail_status(rsp)
        self.assertFalse(status.brake_on)
        self.assertFalse(status.brake_requested)
        self.assertFalse(status.lights_on)
        self.assertFalse(status.lights_requested)
        self.assertFalse(status.rain_on)
        self.assertFalse(status.rain_requested)
        self.assertFalse(status.reverse_on)
        self.assertFalse(status.reverse_requested)
        self.assertFalse(status.engine_running)
        self.assertFalse(status.can_idle)
        self.assertTrue(status.debug_enable, "debug mode not set")

    def test_01_can_idle(self):
        """
        Verify that the CAN idle status is reported after > 1s of bus inactivity
        """
        status = MSG_tail_status(self.interface.recv(1))
        self.assertFalse(status.can_idle)
        time.sleep(2)
        self.interface.drain()
        status = MSG_tail_status(self.interface.recv(1))
        self.assertTrue(status.can_idle, 'CAN idle bit not set as expected')

    def test_02_brake_apply(self):
        """
        Verify that brake lights turn on.
        """

        # check brake not requested
        status = MSG_tail_status(self.interface.recv(1))
        self.assertFalse(status.brake_requested)

        # request brake
        self.interface.send(MSG_brake(True))
        time.sleep(0.1)
        self.interface.drain()

        # check brake requested but not on due to attention-getter running
        status = MSG_tail_status(self.interface.recv(1))
        self.assertTrue(status.brake_requested, 'brake not requested')
        self.assertFalse(status.brake_on, 'brake attention-getter not running')

        # clear brake request
        self.interface.send(MSG_brake(False))
        time.sleep(0.1)
        self.interface.drain()

        # check brake not requested
        status = MSG_tail_status(self.interface.recv(1))
        self.assertFalse(status.brake_requested, 'brake request stuck')

        # request brake again
        self.interface.send(MSG_brake(True))
        time.sleep(0.1)
        self.interface.drain()

        # check brake requested and on due to attention-getter being skipped
        status = MSG_tail_status(self.interface.recv(1))
        self.assertTrue(status.brake_requested, 'brake not requested')
        self.assertFalse(status.brake_on, 'brake attention-getter running unexpectedly')

    def test_03_lights(self):
        """
        Verify that lights work
        """
        status = MSG_tail_status(self.interface.recv(1))
        self.assertFalse(status.lights_on)
        self.assertFalse(status.lights_requested)
        self.assertFalse(status.rain_on)
        self.assertFalse(status.rain_requested)
        self.assertFalse(status.reverse_on)
        self.assertFalse(status.reverse_requested)

        # lights on
        self.interface.send(MSG_lights(tail_light=True))
        self.interface.drain()
        status = MSG_tail_status(self.interface.recv(1))
        self.assertTrue(status.lights_on)
        self.assertTrue(status.lights_requested)
        self.assertFalse(status.rain_on)
        self.assertFalse(status.rain_requested)
        self.assertFalse(status.reverse_on)
        self.assertFalse(status.reverse_requested)

        # reverse on
        self.interface.send(MSG_lights(reverse=True))
        self.interface.drain()
        status = MSG_tail_status(self.interface.recv(1))
        self.assertFalse(status.lights_on)
        self.assertFalse(status.lights_requested)
        self.assertFalse(status.rain_on)
        self.assertFalse(status.rain_requested)
        self.assertTrue(status.reverse_on)
        self.assertTrue(status.reverse_requested)

        self.interface.send(MSG_lights())
        self.interface.drain()
        status = MSG_tail_status(self.interface.recv(1))
        self.assertFalse(status.lights_on)
        self.assertFalse(status.lights_requested)
        self.assertFalse(status.rain_on)
        self.assertFalse(status.rain_requested)
        self.assertFalse(status.reverse_on)
        self.assertFalse(status.reverse_requested)


if __name__ == '__main__':
    parser = argparse.ArgumentParser(description='E36 tail module tester')
    parser.add_argument('--interface-name',
                        type=str,
                        default='anagate',
                        metavar='INTERFACE',
                        help='name of the interface as known to python-can')
    parser.add_argument('--interface-channel',
                        type=str,
                        default='192.168.1.178:C',
                        metavar='CHANNEL',
                        help='interface channel name (e.g. for Anagate units, hostname:portname')
    parser.add_argument('--bitrate',
                        type=int,
                        default=500,
                        metavar='BITRATE_KBPS',
                        help='CAN bitrate (kBps')
    parser.add_argument('--verbose',
                        action='store_true',
                        help='print verbose progress information')

    g_args = parser.parse_args()

    unittest.main()
