#
# Test harness module interface
#
import can
import struct
import time

BRAKE_CTRL_ID = 0x0a8
LIGHT_CTRL_ID = 0x21a
PDM_STATUS_ID = 0x780
TAIL_STATUS_ID = 0x781
ISO_TP_SCANNER_ID = 0x6f1
ISO_TP_DDE_ID = 0x612
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
        if "_filter" in dir(self):
            for (index, (check, value)) in enumerate(self._filter):
                if check and value != self._values[index]:
                    raise MessageError(f'reply field {index} is '
                                       f'0x{self._values[index]} '
                                       f'but expected {value}')

    def __getattr__(self, name):
        if "_bits" in dir(self):
            try:
                (index, shift) = self._bits[name]
                return True if (self._values[index] & (1 << shift)) else False
            except KeyError:
                raise AttributeError

    @classmethod
    def len(self):
        return struct.calcsize(self._format)


class TX_brake(TXMessage):
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


class TX_lights(TXMessage):
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


class RX_lights(RXMessage):
    """BMW light control message"""
    _format = '>BBB'
    _bits = {
        'brake': (0, 7),
        'tail': (0, 2),
        'rain': (0, 6),
        'reverse': (1, 0),
    }

    def __init__(self, raw):
        super().__init__(expected_id=LIGHT_CTRL_ID,
                         raw=raw)


class TX_debug(TXMessage):
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


class RX_ack(RXMessage):
    """broadcast message sent by module on power-up, reboot or crash"""
    _format = '>BIBH'
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
                signon = RX_ack(rsp)
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
                    status = RX_ack(msg)
                    print(f'module reset due to {status.reason}')
                except MessageError:
                    pass
                if msg.arbitration_id == CONSOLE_ID:
                    return msg.data

    def send(self, message):
        """send the message"""

        self._trace(f'CAN TX: {message}')
        self._bus.send(message.raw)

    def send_raw(self, message):
        """send the message"""

        self._trace(f'CAN TX: {message}')
        self._bus.send(message)

    def recv(self, timeout, filter=None):
        """
        wait for a message
        """
        if filter is None:
            filter = RECEIVE_FILTER
        now = time.time()
        deadline = now + timeout
        while time.time() < deadline:
            msg = self._bus.recv(timeout=deadline - time.time())
            if msg is not None:
                if msg.arbitration_id in filter:
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

