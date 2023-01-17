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
        rsp = self.interface.recv(1, filter=[PDM_STATUS_ID])
        # enable debug mode
        self.interface.send(TX_debug(True))

    def tearDown(self):
        self.interface.stop()

    def get_status(self):
        return RX_pdm_status(self.interface.recv(1, filter=[PDM_STATUS_ID]))

    def get_lights(self):
        return RX_lights(self.interface.recv(1, filter=[LIGHT_CTRL_ID]))

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
        time.sleep(0.1)
        self.interface.drain()

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
