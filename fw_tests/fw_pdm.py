#!/usr/bin/env python3
#
# Tests for the PDM module
#
import argparse
from rich.traceback import install
import time
import unittest
from interface import *


class RX_pdm_status(RXMessage):
    """
    Status message from the PDM module.
    """
    _format = '>HBBB'
    _bits = {
        'brake_applied': (2, 0),
        'lights_on': (2, 1),
        'rain_on': (2, 2),
        'can_idle': (2, 3),
        'keypad_active': (2, 4),
        'start_inhibited': (2, 5),
        'start_waiting': (2, 6),
        'starting': (2, 7),
        'debug_enable': (3, 0),
    }

    def __init__(self, raw):
        super().__init__(expected_id=PDM_STATUS_ID,
                         raw=raw)
        self.engine_rpm = self._values[0]
        self.selected_gear = self._values[1]


class RX_bk_reset_all(RXMessage):
    """
    Reset-all message for a Blink keypad.
    """
    _format = '>BB'
    _filter = [(True, 0x81), (True, 0x80)]

    def __init__(self, raw):
        super().__init__(expected_id=0x00, raw=raw)


class PDMTests(unittest.TestCase):
    def setUp(self):
        self.interface = Interface(g_args)
        self.interface.start()
        # wait for a status message (in addition to the module sign-on)
        rsp = self.interface.recv(2, filter=[PDM_STATUS_ID])

        # enable debug mode
        self.interface.send(TX_debug(True))

        # module-specific outputs
        self.kl30 = self.interface.out_t30
        self.kl15 = self.interface.out_t15
        self.s_blow = self.interface.out_3
        self.kill = self.interface.out_4

    def tearDown(self):
        self.interface.stop()

    def get_status(self):
        self.interface.drain()
        rsp = self.interface.recv(1, filter=[PDM_STATUS_ID])
        if rsp is not None:
            return RX_pdm_status(rsp)
        return None

    def get_lights(self):
        return RX_lights(self.interface.recv(1, filter=[LIGHT_CTRL_ID]))

    def s_start_mV(self):
        return self.interface.in_1.get()

    def t15_mV(self):
        return self.interface.in_2.get()

    def led_mV(self):
        return self.interface.in_3.get()

    def t30_mV(self):
        return self.interface.in_4.get()

    def led_assert_blinking(self):
        """
        verify that the LED seems to be blinking
        """
        count = 0
        total = 100
        for i in range(total):
            if (self.led_mV() > 6000):
                count += 1
            time.sleep(0.015)
        duty_cycle = int((count * 100) / total)
        self.assertGreater(duty_cycle, 40, "LED duty cycle too short")
        self.assertLess(duty_cycle, 60, "LED duty cycle too long")

    def test_00_get_status(self):
        """
        Verify that the module is sending a status message and that it looks sane immediately after reset
        """
        rsp = self.interface.recv(1, filter=[PDM_STATUS_ID])
        self.assertNotEqual(rsp, None, 'timed out waiting for status message')
        self.assertEqual(rsp.dlc, 5, f'unexpected status size {rsp.dlc}')
        status = RX_pdm_status(rsp)
        self.assertEqual(status.engine_rpm, 0)
        self.assertEqual(status.selected_gear, 0)
        self.assertFalse(status.brake_applied)
        self.assertFalse(status.lights_on)
        self.assertFalse(status.rain_on)
        self.assertFalse(status.can_idle)
        self.assertFalse(status.keypad_active)
        self.assertTrue(status.start_inhibited)
        self.assertFalse(status.start_waiting)
        self.assertFalse(status.starting)
        self.assertTrue(status.debug_enable, "debug mode not set")

    def test_01_can_idle(self):
        """
        Verify that the CAN idle status is reported after > 1s of bus inactivity
        """
        status = self.get_status()
        self.assertFalse(status.can_idle)
        time.sleep(2)
        self.interface.drain()
        status = self.get_status()
        self.assertTrue(status.can_idle, 'CAN idle bit not set as expected')

    def test_02_brake(self):
        """
        Verify correct response to the brake-on signal
        """
        # check brake not applied / light not on
        status = self.get_status()
        self.assertFalse(status.brake_applied)
        lights = self.get_lights()
        self.assertFalse(lights.brake)

        # send brake-apply message
        self.interface.send(TX_brake(True))

        # check brake applied state / light on
        status = self.get_status()
        self.assertTrue(status.brake_applied)
        lights = self.get_lights()
        self.assertTrue(lights.brake)

    def test_03_bmw_scan(self):
        """
        Verify the ISO-TP scanner setup and operation.
        """
        req = self.interface.recv(1, filter=[ISO_TP_SCANNER_ID])
        self.assertNotEqual(req, None, "no ISO-TP messages on wire")
        self.assertEqual(req.data[0], 0x12)     # target ID
        self.assertEqual(req.data[1], 0x10)     # initial frame
        self.assertEqual(req.data[2], 0x10)     # request length
        self.assertEqual(req.data[3], 0x2c)     # scan packet
        self.assertEqual(req.data[4], 0x10)     # ...

        self.interface.send_raw(can.Message(arbitration_id=0x612,
                                            is_extended_id=False,
                                            data=b'\xf1\x30\x00\x00\x00\x00\x00\x00'))

        req = self.interface.recv(1, filter=[ISO_TP_SCANNER_ID])
        self.assertEqual(req.data[0], 0x12)     # first consecutive frame
        self.assertEqual(req.data[1], 0x21)
        req = self.interface.recv(1, filter=[ISO_TP_SCANNER_ID])
        self.assertEqual(req.data[0], 0x12)     # second consecutive frame
        self.assertEqual(req.data[1], 0x22)

        self.interface.send_raw(can.Message(arbitration_id=0x612,   # initial reply frame
                                            is_extended_id=False,
                                            data=b'\xf1\x10\x0d\x00\x00\x00\x00\x00'))

        req = self.interface.recv(1, filter=[ISO_TP_SCANNER_ID])
        self.assertEqual(req.data[0], 0x12)
        self.assertEqual(req.data[1], 0x30)

        self.interface.send_raw(can.Message(arbitration_id=0x612,   # subsequent reply frame
                                            is_extended_id=False,
                                            data=b'\xf1\x21\x00\x00\x00\x00\x00\x00'))
        self.interface.send_raw(can.Message(arbitration_id=0x612,   # subsequent reply frame
                                            is_extended_id=False,
                                            data=b'\xf1\x22\x00\x00\x00\x00\x00\x00'))

        req = self.interface.recv(1, filter=[0x700])
        self.assertNotEqual(req, None, "no 0x700 re-broadcast")
        req = self.interface.recv(1, filter=[0x701])
        self.assertNotEqual(req, None, "no 0x701 re-broadcast")

        req = self.interface.recv(1, filter=[ISO_TP_SCANNER_ID])
        self.assertEqual(req.data[0], 0x12)     # target ID
        self.assertEqual(req.data[1], 0x02)     # initial frame, length = 2
        self.assertEqual(req.data[2], 0x2c)     # repeat scan packet
        self.assertEqual(req.data[3], 0x10)     # ...

        req = self.interface.recv(1, filter=[ISO_TP_SCANNER_ID])
        self.assertNotEqual(req, None, "no ISO-TP re-setup message on wire")
        self.assertEqual(req.data[0], 0x12)     # target ID
        self.assertEqual(req.data[1], 0x10)     # initial frame
        self.assertEqual(req.data[2], 0x10)     # request length
        self.assertEqual(req.data[3], 0x2c)     # scan packet
        self.assertEqual(req.data[4], 0x10)     # ...

    def test_04_keypad(self):
        """
        Verify keypad setup & operation
        """

        # wait for reset
        req = self.interface.recv(1, filter=[0x000])
        self.assertNotEqual(req, None, "no keypad reset message on wire")
        self.assertEqual(req.dlc, 2)
        self.assertEqual(req.data[0], 0x81)
        self.assertEqual(req.data[1], 0x00)

        # respond with boot message
        self.interface.send_raw(can.Message(arbitration_id=0x715,
                                            is_extended_id=False,
                                            data=b'\x00'))

        # wait for model ID query
        req = self.interface.recv(1, filter=[0x615])
        self.assertNotEqual(req, None, "no keypad ID query on wire")
        self.assertEqual(req.dlc, 8)
        self.assertEqual(req.data[0], 0x40)
        self.assertEqual(req.data[1], 0x0b)
        self.assertEqual(req.data[2], 0x10)
        req = self.interface.recv(1, filter=[0x615])
        self.assertEqual(req.dlc, 8)
        self.assertEqual(req.data[0], 0x60)
        req = self.interface.recv(1, filter=[0x615])
        self.assertEqual(req.dlc, 8)
        req = self.interface.recv(1, filter=[0x615])
        self.assertEqual(req.dlc, 8)
        req = self.interface.recv(1, filter=[0x615])
        self.assertEqual(req.dlc, 8)
        req = self.interface.recv(1, filter=[0x615])
        self.assertEqual(req.dlc, 8)
        req = self.interface.recv(1, filter=[0x615])
        self.assertEqual(req.dlc, 8)
        self.assertNotEqual(req, None, "missing one or more setup messages")

        # respond with ID data
        self.interface.send_raw(can.Message(arbitration_id=0x595,
                                            is_extended_id=False,
                                            data=b'\x00PKP2400'))

        status = self.get_status()
        self.assertTrue(status.keypad_active, "keypad not active after probe")

        # send a key down event for KEY_LIGHTS
        self.interface.send_raw(can.Message(arbitration_id=0x195,
                                            is_extended_id=False,
                                            data=b'\x10\x00\x00\x00\x00'))

        # ... and check that lights are on in the next status message
        status = self.get_status()
        self.assertTrue(status.lights_on, "lights did not turn on with keypress")

        # release the key...
        self.interface.send_raw(can.Message(arbitration_id=0x195,
                                            is_extended_id=False,
                                            data=b'\x00\x00\x00\x00\x00'))

        # ... and check that lights are still in the next status message
        status = self.get_status()
        self.assertTrue(status.lights_on, "lights did not remain on when key was released")

        # ... and then press it again
        time.sleep(0.1)
        self.interface.send_raw(can.Message(arbitration_id=0x195,
                                            is_extended_id=False,
                                            data=b'\x10\x00\x00\x00\x00'))
        status = self.get_status()
        self.assertFalse(status.lights_on, "lights did not turn off with keypress")

        # idle and wait for the firmware to give up on the keypad, should take ~1s
        time.sleep(1.5)
        status = self.get_status()
        self.assertFalse(status.keypad_active, "keypad unexpectedly still active")

    def test_05_power_basic(self):
        """
        verify power-related behaviours
        """

        # We have already verified that the module boots OK when T30 and T15 are
        # turned on at the same time by running other tests, so that's not re-tested
        # here.

        # power off the module and verify that it stops sending status messages
        self.kl15.set_off()
        self.kl30.set_off()
        status = self.get_status()
        self.assertIsNone(status, "module did not power down")
        self.assertLess(self.t15_mV(), 1000, "T15 unexpectedly asserted")
        self.assertLess(self.t30_mV(), 1000, "T30 unexpectedly asserted")
        self.assertLess(self.led_mV(), 1000, "LED unexpectedly on")

        # power on the module and verify that it doesn't send a status message
        self.kl30.set_on()
        status = self.get_status()
        self.assertIsNone(status, "module powered up unexpectedly")
        self.assertLess(self.t15_mV(), 1000, "T15 unexpectedly asserted")
        self.assertLess(self.t30_mV(), 1000, "T30 unexpectedly asserted")
        self.assertLess(self.led_mV(), 1000, "LED unexpectedly on")

        # now apply T15 and verify that we get a status message and the rails come up
        # we don't test the startup delay here as it is very short
        self.kl15.set_on()
        time.sleep(.5)  # module startup can be slow
        status = self.get_status()
        self.assertIsNotNone(status, "module did not start in response to KL15")
        time.sleep(1)   # analog inputs can be slow
        self.assertGreater(self.t15_mV(), 8000, "T15 unexpectedly not asserted")
        self.assertGreater(self.t30_mV(), 8000, "T30 unexpectedly not asserted")
        self.assertGreater(self.led_mV(), 8000, "LED unexpectedly off")

        # drop T15 and verify that the system powers off
        self.kl15.set_off()
        time.sleep(1)   # shutdown should take ~1s, plus analog settle time
        self.assertLess(self.t15_mV(), 1000, "T15 unexpectedly asserted")
        self.assertLess(self.t30_mV(), 1000, "T30 unexpectedly asserted")
        self.assertLess(self.led_mV(), 1000, "LED unexpectedly on")

    def test_06_power_kill(self):
        """
        verify kill-line behaviours
        """

        # power off the module
        self.kl15.set_off()
        self.kl30.set_off()

        # raise the kill line and verify that the module does not power on
        self.kl30.set_on()
        self.kl15.set_on()
        self.kill.set_on()
        time.sleep(1)
        status = self.get_status()
        self.assertIsNone(status, "module powered up but should be killed")
        self.assertLess(self.t15_mV(), 1000, "T15 unexpectedly asserted")
        self.assertLess(self.t30_mV(), 1000, "T30 unexpectedly asserted")
        self.led_assert_blinking()

        # power off the module and drop the kill line
        self.kl15.set_off()
        self.kl30.set_off()
        self.kill.set_off()
        time.sleep(1)

        # power up the module and let it boot
        self.kl30.set_on()
        self.kl15.set_on()
        time.sleep(1)
        status = self.get_status()
        self.assertIsNotNone(status, "module failed to power up")

        # now kill it
        self.kill.set_on()
        time.sleep(3)
        self.assertLess(self.t15_mV(), 1000, "T15 unexpectedly asserted")
        self.assertLess(self.t30_mV(), 1000, "T30 unexpectedly asserted")
        self.led_assert_blinking()

    def test_07_start(self):
        """
        Verify start behaviour
        """

        self.assertLess(self.s_start_mV(), 1000, "S_START output unexpectedly asserted")

        # verify that engine RPM > 1000 forces waiting state
        self.interface.send_raw(can.Message(arbitration_id=0xaa,
                                            is_extended_id=False,
                                            data=b'\x00\x00\x00\x00\x10\x00\x00\x00'))
        status = self.get_status()
        self.assertTrue(status.start_waiting, "start unexpectedly not waiting for engine to stop")

        # set engine RPM to zero and verify inhibited state
        self.interface.send_raw(can.Message(arbitration_id=0xaa,
                                            is_extended_id=False,
                                            data=b'\x00\x00\x00\x00\x00\x00\x00\x00'))
        status = self.get_status()
        self.assertTrue(status.start_inhibited, "start unexpectedly not inhibited")
        self.assertFalse(status.start_waiting, "start unexpectedly waiting for engine to stop")

        # send a gear status message for 'P'
        self.interface.send_raw(can.Message(arbitration_id=0x1d2,
                                            is_extended_id=False,
                                            data=b'\xe1\x00\x00\x00\x00\x00\x00\x00'))
        status = self.get_status()
        self.assertEqual(status.selected_gear, 0x50, "not in Park as expected")

        # apply the brake and wait to ensure the restart delay timer has expired
        self.interface.send(TX_brake(True))
        time.sleep(0.5)

        status = self.get_status()
        self.assertFalse(status.start_inhibited, "start unexpectedly inhibited")
        self.assertFalse(status.start_waiting, "start unexpectedly waiting for engine to stop")

        # send a keypad boot message
        self.interface.send_raw(can.Message(arbitration_id=0x715,
                                            is_extended_id=False,
                                            data=b'\x00'))
        # wait for keypad model ID query
        req = self.interface.recv(1, filter=[0x615])

        # respond with ID data and make sure we were recognised
        self.interface.send_raw(can.Message(arbitration_id=0x595,
                                            is_extended_id=False,
                                            data=b'\x00PKP2400'))
        status = self.get_status()
        self.assertTrue(status.keypad_active, "keypad not active after probe")

        # send a key down event for KEY_START
        self.interface.send_raw(can.Message(arbitration_id=0x195,
                                            is_extended_id=False,
                                            data=b'\x80\x00\x00\x00\x00'))
        status = self.get_status()
        self.assertFalse(status.starting, "engine start too soon")
        self.assertLess(self.s_start_mV(), 1000, "S_START output unexpectedly asserted")
        self.assertFalse(status.start_inhibited, "start unexpectedly inhibited")
        self.assertFalse(status.start_waiting, "start unexpectedly waiting for engine to stop")
        time.sleep(0.5)
        status = self.get_status()
        self.assertGreater(self.s_start_mV(), 8000, "S_START output unexpectedly not asserted")
        self.assertTrue(status.starting, "engine not starting")


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
    parser.add_argument('-k',
                        type=str,
                        metavar='PATTERN',
                        help='execute tests matching PATTERN')

    g_args = parser.parse_args()

    unittest.main()
