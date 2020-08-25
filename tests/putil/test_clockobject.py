import math
import time
import sys


# We must account for clock inaccuracy
if sys.platform == 'win32' or sys.platform == 'cygwin':
    # Assume 17 milliseconds inaccuracy on Windows (worst case)
    # 16 milliseconds plus 1 millisecond for execution time
    CLOCK_INACCURACY = 0.017
else:
    # On other platforms, assume 1 milliseconds, allowing for execution time
    # (1000 times higher than their actual 1 microsecond accuracy)
    CLOCK_INACCURACY = 0.001


def test_clock_get_frame_time(clockobj):
    current_time = clockobj.get_frame_time()
    time.sleep(0.2)
    assert clockobj.get_frame_time() == current_time


def test_clock_jump_frame_time(clockobj):
    current_time = clockobj.get_frame_time()
    clockobj.tick()
    assert clockobj.get_frame_time() == current_time + clockobj.get_frame_time()


def test_clock_get_real_time(clockobj):
    current_time = clockobj.get_real_time()
    time.sleep(0.4)
    assert math.isclose(clockobj.get_real_time() - current_time, 0.4, abs_tol=CLOCK_INACCURACY)


def test_clock_get_long_time(clockobj):
    current_time = clockobj.get_long_time()
    time.sleep(0.4)
    assert math.isclose(clockobj.get_long_time() - current_time, 0.4, abs_tol=CLOCK_INACCURACY)


def test_clock_get_dt(clockobj):
    clockobj.tick()
    first_tick = clockobj.get_frame_time()
    clockobj.tick()
    second_tick = clockobj.get_frame_time()
    assert clockobj.get_dt() == second_tick - first_tick


def test_clock_reset(clockobj):
    clockobj.reset()
    assert clockobj.get_dt() == 0
    assert clockobj.get_frame_time() == 0
    assert clockobj.get_real_time() <= CLOCK_INACCURACY
