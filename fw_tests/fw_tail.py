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
    _format = '>HBB'
    _bits = {
        'brake_on': (1, 0),
        'brake_requested': (1, 1),
        'lights_on': (1, 2),
        'lights_requested': (1, 3),
        'rain_on': (1, 4),
        'rain_requested': (1, 5),
        'reverse_on': (1, 6),
        'reverse_requested': (1, 7),
        'engine_running': (2, 0),
        'can_idle': (2, 1),
        'debug_enable': (2, 2),
    }

    def __init__(self, raw):
        super().__init__(expected_id=TAIL_STATUS_ID,
                         raw=raw)
        self.fuel_mv = self._values[0]


class RX_tail_fuel(RXMessage):
    """
    Fuel level message from the tail module.
    """
    _format = '>B'

    def __init__(self, raw):
        super().__init__(expected_id=TAIL_FUEL_ID,
                         raw=raw)
        self.level = self._values[0]


class TailTests(unittest.TestCase):
    def setUp(self):
        self.interface = Interface(g_args)
        self.interface.start()
        # wait for a status message (in addition to the module sign-on)
        rsp = self.interface.recv(1, filter=[TAIL_STATUS_ID])
        # enable debug mode
        self.interface.send(TX_debug(True))

        # module-specific outputs
        self.kl30 = self.interface.out_t30
        self.kl15 = self.interface.out_t15
        self.fuel = self.interface.out_3

    def tearDown(self):
        self.interface.stop()

    def get_status(self):
        self.interface.drain()
        return RX_tail_status(self.interface.recv(1, filter=[TAIL_STATUS_ID]))

    def brake_r_mV(self):
        return self.interface.in_1.get()

    def brake_l_mV(self):
        return self.interface.in_2.get()

    def tail_mV(self):
        return self.interface.in_3.get()

    def reverse_mV(self):
        return self.interface.in_4.get()

    def brake_duty_cycles(self, samples=100):
        right_count = 0
        left_count = 0
        for i in range(samples):
            if self.brake_r_mV() > 5000:
                right_count += 1
            if self.brake_l_mV() > 5000:
                left_count += 1
            time.sleep(0.005)
        right_duty_cycle = int((right_count * 100) / samples)
        left_duty_cycle = int((left_count * 100) / samples)
        return (right_duty_cycle, left_duty_cycle)

    def brake_assert_off(self):
        left, right = self.brake_duty_cycles()
        self.assertLess(right, 10, "right brake light not off")
        self.assertLess(left, 10, "left brake light not off")

    def brake_assert_on(self):
        left, right = self.brake_duty_cycles()
        self.assertGreater(right, 90, "right brake light not on")
        self.assertGreater(left, 90, "left brake light not on")

    def brake_assert_flashing(self):
        left, right = self.brake_duty_cycles()
        self.assertGreater(right, 20, "right brake duty cycle too low")
        self.assertLess(right, 80, "right brake duty cycle too high")
        self.assertGreater(left, 20, "left brake duty cycle too low")
        self.assertLess(left, 80, "left brake duty cycle too high")

    def test_00_get_status(self):
        """
        Verify that the module state appears sane out of reset.
        """
        status = self.get_status()
        self.assertIsNotNone(status, 'timed out waiting for status message')
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

        self.brake_assert_off()
        self.assertLess(self.tail_mV(), 5000, "tail lights unexpectedly on")

    def test_01_can_idle(self):
        """
        Verify that the CAN idle status is reported after > 1s of bus inactivity.
        """
        status = self.get_status()
        self.assertFalse(status.can_idle)
        time.sleep(1.5)
        status = self.get_status()
        self.assertTrue(status.can_idle, 'CAN idle bit not set as expected')

    def test_02_brake_apply(self):
        """
        Verify that brake lights work.
        """
        # check brake not requested
        status = self.get_status()
        self.assertFalse(status.brake_requested, "brake unexpectedly requested")
        self.assertFalse(status.brake_on, 'brakes unexpectedly on')
        self.brake_assert_off()

        # request brake
        self.interface.send(TX_brake(True))

        # check brake requested but not on due to attention-getter running
        status = self.get_status()
        self.assertTrue(status.brake_requested, 'brake not requested')
        self.assertFalse(status.brake_on, 'brake attention-getter not running')
        self.brake_assert_flashing()

        # clear brake request
        self.interface.send(TX_brake(False))
        #time.sleep(0.1)

        # check brake not requested and lights off
        status = self.get_status()
        self.assertFalse(status.brake_requested, 'brake request stuck')
        self.assertFalse(status.brake_on, 'brakes stuck on')
        self.brake_assert_off()

        # request brake again
        self.interface.send(TX_brake(True))

        # check brake requested and on due to attention-getter being skipped
        status = self.get_status()
        self.assertTrue(status.brake_requested, 'brake not requested')
        self.assertTrue(status.brake_on, 'brake attention-getter running unexpectedly')
        self.brake_assert_on()

    def test_03_lights(self):
        """
        Verify that tail lights work.
        """
        status = self.get_status()
        self.assertFalse(status.lights_on)
        self.assertFalse(status.lights_requested)
        self.assertFalse(status.rain_on)
        self.assertFalse(status.rain_requested)
        self.assertFalse(status.reverse_on)
        self.assertFalse(status.reverse_requested)
        self.assertLess(self.tail_mV(), 5000, "tail lights unexpectedly on")
        self.assertLess(self.reverse_mV(), 5000, "reverse unexpectedly on")

        # lights on
        self.interface.send(TX_lights(tail_light=True))
        status = self.get_status()
        self.assertTrue(status.lights_on)
        self.assertTrue(status.lights_requested)
        self.assertFalse(status.rain_on)
        self.assertFalse(status.rain_requested)
        self.assertFalse(status.reverse_on)
        self.assertFalse(status.reverse_requested)
        self.assertGreater(self.tail_mV(), 5000, "tail lights unexpectedly off")

        # reverse on
        self.interface.send(TX_lights(reverse=True))
        status = self.get_status()
        self.assertFalse(status.lights_on)
        self.assertFalse(status.lights_requested)
        self.assertFalse(status.rain_on)
        self.assertFalse(status.rain_requested)
        self.assertTrue(status.reverse_on)
        self.assertTrue(status.reverse_requested)
        self.assertGreater(self.reverse_mV(), 5000, "reverse unexpectedly off")

        self.interface.send(TX_lights())
        status = self.get_status()
        self.assertFalse(status.lights_on)
        self.assertFalse(status.lights_requested)
        self.assertFalse(status.rain_on)
        self.assertFalse(status.rain_requested)
        self.assertFalse(status.reverse_on)
        self.assertFalse(status.reverse_requested)
        self.assertLess(self.tail_mV(), 5000, "tail lights unexpectedly on")
        self.assertLess(self.reverse_mV(), 5000, "reverse unexpectedly on")

    def test_04_fuel(self):
        """
        Verify fuel sender functionality.
        """
        self.fuel.set(0)
        time.sleep(1.5)
        self.interface.drain()
        rsp = RX_tail_fuel(self.interface.recv(2, filter=[TAIL_FUEL_ID]))
        self.assertLess(rsp.level, 15, "fuel level too high at 550mV")    # Anagate minimum output ~0.55V

        self.fuel.set(2500)
        time.sleep(0.5)
        self.interface.drain()
        rsp = RX_tail_fuel(self.interface.recv(2, filter=[TAIL_FUEL_ID]))
        self.assertLess(rsp.level, 55, "fuel level too high at 2.5V")
        self.assertGreater(rsp.level, 45, "fuel level too low at 2.5V")

        self.fuel.set(5000)
        time.sleep(0.5)
        self.interface.drain()
        rsp = RX_tail_fuel(self.interface.recv(2, filter=[TAIL_FUEL_ID]))
        self.assertGreater(rsp.level, 95, "fuel level too low at 5V")


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
