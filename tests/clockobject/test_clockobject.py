import pytest
from panda3d.core import *
import time

def test_get_frame_time(clockobj):
    current_time = clockobj.getFrameTime()
    time.sleep(2)
    assert clockobj.getFrameTime() == current_time

def test_jump_frame_time(clockobj):
    current_time = clockobj.getFrameTime()
    clockobj.tick()
    assert clockobj.getFrameTime() == current_time + clockobj.getFrameTime()

def test_get_real_time(clockobj):
    current_time = clockobj.getRealTime()
    time.sleep(2)
    assert current_time != clockobj.getRealTime()

def test_get_Dt(clockobj):
    clockobj.tick()
    first_tick = clockobj.getFrameTime()
    clockobj.tick()
    second_tick = clockobj.getFrameTime()
    assert clockobj.getDt() == second_tick - first_tick
