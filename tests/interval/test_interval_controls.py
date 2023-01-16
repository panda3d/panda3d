from direct.interval.IntervalManager import ivalMgr
from direct.interval.IntervalGlobal import *
from direct.task.TaskManagerGlobal import taskMgr
import pytest


def test_resume_sequence_no_startT():
    test_resume_sequence_no_startT.callCount = 0

    def func():
        test_resume_sequence_no_startT.callCount += 1

    sut = Sequence(Func(func))
    sut.start()
    ivalMgr.step()
    sut.resume(None)
    ivalMgr.step()
    taskMgr.step()
    assert test_resume_sequence_no_startT.callCount == 1

def test_resume_func_no_startT():
    test_resume_func_no_startT.callCount = 0

    def func():
        test_resume_func_no_startT.callCount += 1

    sut = Func(func)
    sut.start()
    ivalMgr.step()
    taskMgr.step()
    sut.resume(None)
    ivalMgr.step()
    taskMgr.step()
    assert test_resume_func_no_startT.callCount == 1
