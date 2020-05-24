import time


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
    assert clockobj.get_real_time() - current_time >= 0.4


def test_clock_get_long_time(clockobj):
    current_time = clockobj.get_long_time()
    time.sleep(0.4)
    assert clockobj.get_long_time() - current_time >= 0.4


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
    assert clockobj.get_real_time() < 0.01
