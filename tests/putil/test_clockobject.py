import time
from pytest import approx
from panda3d.core import ClockObject

def test_get_frame_time(clockobj):
    current_time = clockobj.get_frame_time()
    time.sleep(2)
    assert clockobj.get_frame_time() == current_time

def test_jump_frame_time(clockobj):
    current_time = clockobj.get_frame_time()
    clockobj.tick()
    assert clockobj.get_frame_time() == current_time + clockobj.get_frame_time()

def test_get_real_time(clockobj):
    current_time = clockobj.get_real_time()
    time.sleep(2.0)
    assert clockobj.get_real_time() - current_time == approx(2.0, 0.1)

def test_get_dt(clockobj):
    clockobj.tick()
    first_tick = clockobj.get_frame_time()
    clockobj.tick()
    second_tick = clockobj.get_frame_time()
    assert clockobj.get_dt() == second_tick - first_tick

def test_get_long_time(clockobj):
    current_time = clockobj.get_long_time()
    time.sleep(2.0)
    assert clockobj.get_long_time() - current_time == approx(2.0, 0.1)

def test_get_mode(clockobj):
    clockobj.tick()
    assert clockobj.get_mode() == ClockObject.M_normal
    clockobj.set_mode(ClockObject.M_non_real_time)
    assert clockobj.get_mode() == ClockObject.M_non_real_time
    clockobj.set_mode(ClockObject.M_forced)
    assert clockobj.get_mode() == ClockObject.M_forced
    clockobj.set_mode(ClockObject.M_degrade)
    assert clockobj.get_mode() == ClockObject.M_degrade
    clockobj.set_mode(ClockObject.M_slave)
    assert clockobj.get_mode() == ClockObject.M_slave
    clockobj.set_mode(ClockObject.M_limited)
    assert clockobj.get_mode() == ClockObject.M_limited
    clockobj.set_mode(ClockObject.M_integer)
    assert clockobj.get_mode() == ClockObject.M_integer
    clockobj.set_mode(ClockObject.M_integer_limited)
    assert clockobj.get_mode() == ClockObject.M_integer_limited
