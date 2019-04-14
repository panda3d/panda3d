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

def test_get_mode(clockobj):
    clockobj.tick()
    assert clockobj.get_mode() == 0 #M_normal
    clockobj.set_mode(1)
    assert clockobj.get_mode() == 1 #M_non_real_time
    clockobj.set_mode(2)
    assert clockobj.get_mode() == 2 #M_forced
    clockobj.set_mode(3)
    assert clockobj.get_mode() == 3 #M_degrade
    clockobj.set_mode(4)
    assert clockobj.get_mode() == 4 #M_Slave
    clockobj.set_mode(5)
    assert clockobj.get_mode() == 5 #M_Limited
    clockobj.set_mode(6)
    assert clockobj.get_mode() == 6 #M_Integer
    clockobj.set_mode(7)
    assert clockobj.get_mode() == 7  #M_Integer_Limited

def test_get_long_time(clockobj):
    current_time = clockobj.get_long_time()
    time.sleep(2)
    assert current_time != clockobj.get_long_time()
