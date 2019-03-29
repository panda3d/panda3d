import time

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
    time.sleep(2)
    assert current_time != clockobj.get_real_time()

def test_get_dt(clockobj):
    clockobj.tick()
    first_tick = clockobj.get_frame_time()
    clockobj.tick()
    second_tick = clockobj.get_frame_time()
    assert clockobj.get_dt() == second_tick - first_tick
