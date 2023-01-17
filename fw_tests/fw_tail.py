#!/usr/bin/env python3
#
# Tests for the tail module
#
import argparse
from rich.traceback import install
import time
import unittest
from interface import *


class RX_tail_status(RXMessage):
    """
    Status message from the tail module.
    """
    _format = '>BB'
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


class TailTests(unittest.TestCase):
    def setUp(self):
        self.interface = Interface(g_args)
        self.interface.start()
        # wait for a status message (in addition to the module sign-on)
        rsp = self.interface.recv(1, filter=[TAIL_STATUS_ID])
        # enable debug mode
        self.interface.send(TX_debug(True))

    def tearDown(self):
        self.interface.stop()

    def get_status(self):
        return RX_tail_status(self.interface.recv(1, filter=[TAIL_STATUS_ID]))

    def test_00_get_status(self):
        """
        Verify that the module is sending a status message and that it looks sane immediately after reset
        """
        rsp = self.interface.recv(1, filter=[TAIL_STATUS_ID])
        self.assertNotEqual(rsp, None, 'timed out waiting for status message')
        self.assertEqual(rsp.dlc, 2, f'unexpected status size {rsp.dlc}')
        status = RX_tail_status(rsp)
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
        status = self.get_status()
        self.assertFalse(status.can_idle)
        time.sleep(2)
        self.interface.drain()
        status = self.get_status()
        self.assertTrue(status.can_idle, 'CAN idle bit not set as expected')

    def test_02_brake_apply(self):
        """
        Verify that brake lights turn on.
        """

        # check brake not requested
        status = self.get_status()
        self.assertFalse(status.brake_requested)

        # request brake
        self.interface.send(TX_brake(True))
        time.sleep(0.1)
        self.interface.drain()

        # check brake requested but not on due to attention-getter running
        status = self.get_status()
        self.assertTrue(status.brake_requested, 'brake not requested')
        self.assertFalse(status.brake_on, 'brake attention-getter not running')

        # clear brake request
        self.interface.send(TX_brake(False))
        time.sleep(0.1)
        self.interface.drain()

        # check brake not requested
        status = self.get_status()
        self.assertFalse(status.brake_requested, 'brake request stuck')

        # request brake again
        self.interface.send(TX_brake(True))
        time.sleep(0.1)
        self.interface.drain()

        # check brake requested and on due to attention-getter being skipped
        status = self.get_status()
        self.assertTrue(status.brake_requested, 'brake not requested')
        self.assertFalse(status.brake_on, 'brake attention-getter running unexpectedly')

    def test_03_lights(self):
        """
        Verify that lights work
        """
        status = self.get_status()
        self.assertFalse(status.lights_on)
        self.assertFalse(status.lights_requested)
        self.assertFalse(status.rain_on)
        self.assertFalse(status.rain_requested)
        self.assertFalse(status.reverse_on)
        self.assertFalse(status.reverse_requested)

        # lights on
        self.interface.send(TX_lights(tail_light=True))
        self.interface.drain()
        status = self.get_status()
        self.assertTrue(status.lights_on)
        self.assertTrue(status.lights_requested)
        self.assertFalse(status.rain_on)
        self.assertFalse(status.rain_requested)
        self.assertFalse(status.reverse_on)
        self.assertFalse(status.reverse_requested)

        # reverse on
        self.interface.send(TX_lights(reverse=True))
        self.interface.drain()
        status = self.get_status()
        self.assertFalse(status.lights_on)
        self.assertFalse(status.lights_requested)
        self.assertFalse(status.rain_on)
        self.assertFalse(status.rain_requested)
        self.assertTrue(status.reverse_on)
        self.assertTrue(status.reverse_requested)

        self.interface.send(TX_lights())
        self.interface.drain()
        status = self.get_status()
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
