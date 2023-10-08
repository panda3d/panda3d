from panda3d import core
from direct.task import Task


def test_TaskManager():
    tm = Task.TaskManager()
    tm.mgr = core.AsyncTaskManager("Test manager")
    tm.setClock(core.ClockObject())
    tm.setupTaskChain("default", tickClock = True)

    tm._startTrackingMemLeaks = lambda: None
    tm._stopTrackingMemLeaks = lambda: None
    tm._checkMemLeaks = lambda: None

    # check for memory leaks after every test
    tm._startTrackingMemLeaks()
    tm._checkMemLeaks()

    # run-once task
    l = []

    def _testDone(task, l=l):
        l.append(None)
        return task.done
    tm.add(_testDone, 'testDone')
    tm.step()
    assert len(l) == 1
    tm.step()
    assert len(l) == 1
    _testDone = None
    tm._checkMemLeaks()

    # remove by name
    def _testRemoveByName(task):
        return task.done
    tm.add(_testRemoveByName, 'testRemoveByName')
    assert tm.remove('testRemoveByName') == 1
    assert tm.remove('testRemoveByName') == 0
    _testRemoveByName = None
    tm._checkMemLeaks()

    # duplicate named tasks
    def _testDupNamedTasks(task):
        return task.done
    tm.add(_testDupNamedTasks, 'testDupNamedTasks')
    tm.add(_testDupNamedTasks, 'testDupNamedTasks')
    assert tm.remove('testRemoveByName') == 0
    _testDupNamedTasks = None
    tm._checkMemLeaks()

    # continued task
    l = []

    def _testCont(task, l = l):
        l.append(None)
        return task.cont
    tm.add(_testCont, 'testCont')
    tm.step()
    assert len(l) == 1
    tm.step()
    assert len(l) == 2
    tm.remove('testCont')
    _testCont = None
    tm._checkMemLeaks()

    # continue until done task
    l = []

    def _testContDone(task, l = l):
        l.append(None)
        if len(l) >= 2:
            return task.done
        else:
            return task.cont
    tm.add(_testContDone, 'testContDone')
    tm.step()
    assert len(l) == 1
    tm.step()
    assert len(l) == 2
    tm.step()
    assert len(l) == 2
    assert not tm.hasTaskNamed('testContDone')
    _testContDone = None
    tm._checkMemLeaks()

    # hasTaskNamed
    def _testHasTaskNamed(task):
        return task.done
    tm.add(_testHasTaskNamed, 'testHasTaskNamed')
    assert tm.hasTaskNamed('testHasTaskNamed')
    tm.step()
    assert not tm.hasTaskNamed('testHasTaskNamed')
    _testHasTaskNamed = None
    tm._checkMemLeaks()

    # task sort
    l = []

    def _testPri1(task, l = l):
        l.append(1)
        return task.cont

    def _testPri2(task, l = l):
        l.append(2)
        return task.cont
    tm.add(_testPri1, 'testPri1', sort = 1)
    tm.add(_testPri2, 'testPri2', sort = 2)
    tm.step()
    assert len(l) == 2
    assert l == [1, 2,]
    tm.step()
    assert len(l) == 4
    assert l == [1, 2, 1, 2,]
    tm.remove('testPri1')
    tm.remove('testPri2')
    _testPri1 = None
    _testPri2 = None
    tm._checkMemLeaks()

    # task extraArgs
    l = []

    def _testExtraArgs(arg1, arg2, l=l):
        l.extend([arg1, arg2,])
        return Task.done
    tm.add(_testExtraArgs, 'testExtraArgs', extraArgs=[4,5])
    tm.step()
    assert len(l) == 2
    assert l == [4, 5,]
    _testExtraArgs = None
    tm._checkMemLeaks()

    # task appendTask
    l = []

    def _testAppendTask(arg1, arg2, task, l=l):
        l.extend([arg1, arg2,])
        return task.done
    tm.add(_testAppendTask, '_testAppendTask', extraArgs=[4,5], appendTask=True)
    tm.step()
    assert len(l) == 2
    assert l == [4, 5,]
    _testAppendTask = None
    tm._checkMemLeaks()

    # task uponDeath
    l = []

    def _uponDeathFunc(task, l=l):
        l.append(task.name)

    def _testUponDeath(task):
        return Task.done
    tm.add(_testUponDeath, 'testUponDeath', uponDeath=_uponDeathFunc)
    tm.step()
    assert len(l) == 1
    assert l == ['testUponDeath']
    _testUponDeath = None
    _uponDeathFunc = None
    tm._checkMemLeaks()

    # task owner
    class _TaskOwner:
        def _addTask(self, task):
            self.addedTaskName = task.name

        def _clearTask(self, task):
            self.clearedTaskName = task.name
    to = _TaskOwner()
    l = []

    def _testOwner(task):
        return Task.done
    tm.add(_testOwner, 'testOwner', owner=to)
    tm.step()
    assert getattr(to, 'addedTaskName', None) == 'testOwner'
    assert getattr(to, 'clearedTaskName', None) == 'testOwner'
    _testOwner = None
    del to
    _TaskOwner = None
    tm._checkMemLeaks()

    doLaterTests = [0,]

    # doLater
    l = []

    def _testDoLater1(task, l=l):
        l.append(1)

    def _testDoLater2(task, l=l):
        l.append(2)

    def _monitorDoLater(task, tm=tm, l=l, doLaterTests=doLaterTests):
        if task.time > .03:
            assert l == [1, 2,]
            doLaterTests[0] -= 1
            return task.done
        return task.cont
    tm.doMethodLater(.01, _testDoLater1, 'testDoLater1')
    tm.doMethodLater(.02, _testDoLater2, 'testDoLater2')
    doLaterTests[0] += 1
    # make sure we run this task after the doLaters if they all occur on the same frame
    tm.add(_monitorDoLater, 'monitorDoLater', sort=10)
    _testDoLater1 = None
    _testDoLater2 = None
    _monitorDoLater = None
    # don't check until all the doLaters are finished
    #tm._checkMemLeaks()

    # doLater sort
    l = []

    def _testDoLaterPri1(task, l=l):
        l.append(1)

    def _testDoLaterPri2(task, l=l):
        l.append(2)

    def _monitorDoLaterPri(task, tm=tm, l=l, doLaterTests=doLaterTests):
        if task.time > .02:
            assert l == [1, 2,]
            doLaterTests[0] -= 1
            return task.done
        return task.cont
    tm.doMethodLater(.01, _testDoLaterPri1, 'testDoLaterPri1', sort=1)
    tm.doMethodLater(.01, _testDoLaterPri2, 'testDoLaterPri2', sort=2)
    doLaterTests[0] += 1
    # make sure we run this task after the doLaters if they all occur on the same frame
    tm.add(_monitorDoLaterPri, 'monitorDoLaterPri', sort=10)
    _testDoLaterPri1 = None
    _testDoLaterPri2 = None
    _monitorDoLaterPri = None
    # don't check until all the doLaters are finished
    #tm._checkMemLeaks()

    # doLater extraArgs
    l = []

    def _testDoLaterExtraArgs(arg1, l=l):
        l.append(arg1)

    def _monitorDoLaterExtraArgs(task, tm=tm, l=l, doLaterTests=doLaterTests):
        if task.time > .02:
            assert l == [3,]
            doLaterTests[0] -= 1
            return task.done
        return task.cont
    tm.doMethodLater(.01, _testDoLaterExtraArgs, 'testDoLaterExtraArgs', extraArgs=[3,])
    doLaterTests[0] += 1
    # make sure we run this task after the doLaters if they all occur on the same frame
    tm.add(_monitorDoLaterExtraArgs, 'monitorDoLaterExtraArgs', sort=10)
    _testDoLaterExtraArgs = None
    _monitorDoLaterExtraArgs = None
    # don't check until all the doLaters are finished
    #tm._checkMemLeaks()

    # doLater appendTask
    l = []

    def _testDoLaterAppendTask(arg1, task, l=l):
        assert task.name == 'testDoLaterAppendTask'
        l.append(arg1)

    def _monitorDoLaterAppendTask(task, tm=tm, l=l, doLaterTests=doLaterTests):
        if task.time > .02:
            assert l == [4,]
            doLaterTests[0] -= 1
            return task.done
        return task.cont
    tm.doMethodLater(.01, _testDoLaterAppendTask, 'testDoLaterAppendTask',
                        extraArgs=[4,], appendTask=True)
    doLaterTests[0] += 1
    # make sure we run this task after the doLaters if they all occur on the same frame
    tm.add(_monitorDoLaterAppendTask, 'monitorDoLaterAppendTask', sort=10)
    _testDoLaterAppendTask = None
    _monitorDoLaterAppendTask = None
    # don't check until all the doLaters are finished
    #tm._checkMemLeaks()

    # doLater uponDeath
    l = []

    def _testUponDeathFunc(task, l=l):
        assert task.name == 'testDoLaterUponDeath'
        l.append(10)

    def _testDoLaterUponDeath(arg1, l=l):
        return Task.done

    def _monitorDoLaterUponDeath(task, tm=tm, l=l, doLaterTests=doLaterTests):
        if task.time > .02:
            assert l == [10,]
            doLaterTests[0] -= 1
            return task.done
        return task.cont
    tm.doMethodLater(.01, _testDoLaterUponDeath, 'testDoLaterUponDeath',
                        uponDeath=_testUponDeathFunc)
    doLaterTests[0] += 1
    # make sure we run this task after the doLaters if they all occur on the same frame
    tm.add(_monitorDoLaterUponDeath, 'monitorDoLaterUponDeath', sort=10)
    _testUponDeathFunc = None
    _testDoLaterUponDeath = None
    _monitorDoLaterUponDeath = None
    # don't check until all the doLaters are finished
    #tm._checkMemLeaks()

    # doLater owner
    class _DoLaterOwner:
        def _addTask(self, task):
            self.addedTaskName = task.name

        def _clearTask(self, task):
            self.clearedTaskName = task.name
    doLaterOwner = _DoLaterOwner()
    l = []

    def _testDoLaterOwner(l=l):
        pass

    def _monitorDoLaterOwner(task, tm=tm, l=l, doLaterOwner=doLaterOwner,
                                doLaterTests=doLaterTests):
        if task.time > .02:
            assert getattr(doLaterOwner, 'addedTaskName', None) == 'testDoLaterOwner'
            assert getattr(doLaterOwner, 'clearedTaskName', None) == 'testDoLaterOwner'
            doLaterTests[0] -= 1
            return task.done
        return task.cont
    tm.doMethodLater(.01, _testDoLaterOwner, 'testDoLaterOwner',
                        owner=doLaterOwner)
    doLaterTests[0] += 1
    # make sure we run this task after the doLaters if they all occur on the same frame
    tm.add(_monitorDoLaterOwner, 'monitorDoLaterOwner', sort=10)
    _testDoLaterOwner = None
    _monitorDoLaterOwner = None
    del doLaterOwner
    _DoLaterOwner = None
    # don't check until all the doLaters are finished
    #tm._checkMemLeaks()

    # run the doLater tests
    while doLaterTests[0] > 0:
        tm.step()
    del doLaterTests
    tm._checkMemLeaks()

    # getTasks
    def _testGetTasks(task):
        return task.cont
    # No doLaterProcessor in the new world.
    assert len(tm.getTasks()) == 0
    tm.add(_testGetTasks, 'testGetTasks1')
    assert len(tm.getTasks()) == 1
    assert (tm.getTasks()[0].name == 'testGetTasks1' or
            tm.getTasks()[1].name == 'testGetTasks1')
    tm.add(_testGetTasks, 'testGetTasks2')
    tm.add(_testGetTasks, 'testGetTasks3')
    assert len(tm.getTasks()) == 3
    tm.remove('testGetTasks2')
    assert len(tm.getTasks()) == 2
    tm.remove('testGetTasks1')
    tm.remove('testGetTasks3')
    assert len(tm.getTasks()) == 0
    _testGetTasks = None
    tm._checkMemLeaks()

    # getDoLaters
    def _testGetDoLaters():
        pass
    assert len(tm.getDoLaters()) == 0
    tm.doMethodLater(.1, _testGetDoLaters, 'testDoLater1')
    assert len(tm.getDoLaters()) == 1
    assert tm.getDoLaters()[0].name == 'testDoLater1'
    tm.doMethodLater(.1, _testGetDoLaters, 'testDoLater2')
    tm.doMethodLater(.1, _testGetDoLaters, 'testDoLater3')
    assert len(tm.getDoLaters()) == 3
    tm.remove('testDoLater2')
    assert len(tm.getDoLaters()) == 2
    tm.remove('testDoLater1')
    tm.remove('testDoLater3')
    assert len(tm.getDoLaters()) == 0
    _testGetDoLaters = None
    tm._checkMemLeaks()

    # duplicate named doLaters removed via taskMgr.remove
    def _testDupNameDoLaters():
        pass
    # the doLaterProcessor is always running
    tm.doMethodLater(.1, _testDupNameDoLaters, 'testDupNameDoLater')
    tm.doMethodLater(.1, _testDupNameDoLaters, 'testDupNameDoLater')
    assert len(tm.getDoLaters()) == 2
    tm.remove('testDupNameDoLater')
    assert len(tm.getDoLaters()) == 0
    _testDupNameDoLaters = None
    tm._checkMemLeaks()

    # duplicate named doLaters removed via remove()
    def _testDupNameDoLatersRemove():
        pass
    # the doLaterProcessor is always running
    dl1 = tm.doMethodLater(.1, _testDupNameDoLatersRemove, 'testDupNameDoLaterRemove')
    dl2 = tm.doMethodLater(.1, _testDupNameDoLatersRemove, 'testDupNameDoLaterRemove')
    assert len(tm.getDoLaters()) == 2
    dl2.remove()
    assert len(tm.getDoLaters()) == 1
    dl1.remove()
    assert len(tm.getDoLaters()) == 0
    _testDupNameDoLatersRemove = None
    # nameDict etc. isn't cleared out right away with task.remove()
    tm._checkMemLeaks()

    # getTasksNamed
    def _testGetTasksNamed(task):
        return task.cont
    assert len(tm.getTasksNamed('testGetTasksNamed')) == 0
    tm.add(_testGetTasksNamed, 'testGetTasksNamed')
    assert len(tm.getTasksNamed('testGetTasksNamed')) == 1
    assert tm.getTasksNamed('testGetTasksNamed')[0].name == 'testGetTasksNamed'
    tm.add(_testGetTasksNamed, 'testGetTasksNamed')
    tm.add(_testGetTasksNamed, 'testGetTasksNamed')
    assert len(tm.getTasksNamed('testGetTasksNamed')) == 3
    tm.remove('testGetTasksNamed')
    assert len(tm.getTasksNamed('testGetTasksNamed')) == 0
    _testGetTasksNamed = None
    tm._checkMemLeaks()

    # removeTasksMatching
    def _testRemoveTasksMatching(task):
        return task.cont
    tm.add(_testRemoveTasksMatching, 'testRemoveTasksMatching')
    assert len(tm.getTasksNamed('testRemoveTasksMatching')) == 1
    tm.removeTasksMatching('testRemoveTasksMatching')
    assert len(tm.getTasksNamed('testRemoveTasksMatching')) == 0
    tm.add(_testRemoveTasksMatching, 'testRemoveTasksMatching1')
    tm.add(_testRemoveTasksMatching, 'testRemoveTasksMatching2')
    assert len(tm.getTasksNamed('testRemoveTasksMatching1')) == 1
    assert len(tm.getTasksNamed('testRemoveTasksMatching2')) == 1
    tm.removeTasksMatching('testRemoveTasksMatching*')
    assert len(tm.getTasksNamed('testRemoveTasksMatching1')) == 0
    assert len(tm.getTasksNamed('testRemoveTasksMatching2')) == 0
    tm.add(_testRemoveTasksMatching, 'testRemoveTasksMatching1a')
    tm.add(_testRemoveTasksMatching, 'testRemoveTasksMatching2a')
    assert len(tm.getTasksNamed('testRemoveTasksMatching1a')) == 1
    assert len(tm.getTasksNamed('testRemoveTasksMatching2a')) == 1
    tm.removeTasksMatching('testRemoveTasksMatching?a')
    assert len(tm.getTasksNamed('testRemoveTasksMatching1a')) == 0
    assert len(tm.getTasksNamed('testRemoveTasksMatching2a')) == 0
    _testRemoveTasksMatching = None
    tm._checkMemLeaks()

    # create Task object and add to mgr
    l = []

    def _testTaskObj(task, l=l):
        l.append(None)
        return task.cont
    t = Task.Task(_testTaskObj)
    tm.add(t, 'testTaskObj')
    tm.step()
    assert len(l) == 1
    tm.step()
    assert len(l) == 2
    tm.remove('testTaskObj')
    tm.step()
    assert len(l) == 2
    _testTaskObj = None
    tm._checkMemLeaks()

    # remove Task via task.remove()
    l = []

    def _testTaskObjRemove(task, l=l):
        l.append(None)
        return task.cont
    t = Task.Task(_testTaskObjRemove)
    tm.add(t, 'testTaskObjRemove')
    tm.step()
    assert len(l) == 1
    tm.step()
    assert len(l) == 2
    t.remove()
    tm.step()
    assert len(l) == 2
    del t
    _testTaskObjRemove = None
    tm._checkMemLeaks()

    # set/get Task sort
    l = []
    def _testTaskObjSort(arg, task, l=l):
       l.append(arg)
       return task.cont
    t1 = Task.Task(_testTaskObjSort)
    t2 = Task.Task(_testTaskObjSort)
    tm.add(t1, 'testTaskObjSort1', extraArgs=['a',], appendTask=True, sort=1)
    tm.add(t2, 'testTaskObjSort2', extraArgs=['b',], appendTask=True, sort=2)
    tm.step()
    assert len(l) == 2
    assert l == ['a', 'b']
    assert t1.getSort() == 1
    assert t2.getSort() == 2
    t1.setSort(3)
    assert t1.getSort() == 3
    tm.step()
    assert len(l) == 4
    assert l == ['a', 'b', 'b', 'a',]
    t1.remove()
    t2.remove()
    tm.step()
    assert len(l) == 4
    del t1
    del t2
    _testTaskObjSort = None
    tm._checkMemLeaks()

    del l
    tm.destroy()
    del tm
