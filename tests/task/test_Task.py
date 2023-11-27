import pytest
from panda3d import core
from direct.task import Task


TASK_NAME = 'Arbitrary task name'
TASK_CHAIN_NAME = 'Arbitrary task chain name'


def DUMMY_FUNCTION(*_):
    pass


@pytest.fixture
def task_manager():
    manager = Task.TaskManager()
    manager.mgr = core.AsyncTaskManager('Test manager')
    manager.clock = core.ClockObject()
    manager.setupTaskChain('default', tickClock=True)
    manager.finalInit()
    yield manager
    manager.destroy()


def test_sequence(task_manager):
    numbers = []

    def append_1(task):
        numbers.append(1)

    def append_2(task):
        numbers.append(2)

    sequence = Task.sequence(core.PythonTask(append_1), core.PythonTask(append_2))
    task_manager.add(sequence)
    for _ in range(3):
        task_manager.step()
    assert not task_manager.getTasks()
    assert numbers == [1, 2]


def test_loop(task_manager):
    numbers = []

    def append_1(task):
        numbers.append(1)

    def append_2(task):
        numbers.append(2)

    loop = Task.loop(core.PythonTask(append_1), core.PythonTask(append_2))
    task_manager.add(loop)
    for _ in range(5):
        task_manager.step()
    assert numbers == [1, 2, 1, 2]


def test_get_current_task(task_manager):
    def check_current_task(task):
        assert task_manager.getCurrentTask().name == TASK_NAME

    task_manager.add(check_current_task, TASK_NAME)
    assert len(task_manager.getTasks()) == 1
    assert task_manager.getCurrentTask() is None

    task_manager.step()
    assert len(task_manager.getTasks()) == 0
    assert task_manager.getCurrentTask() is None


def test_has_task_chain(task_manager):
    assert not task_manager.hasTaskChain(TASK_CHAIN_NAME)
    task_manager.setupTaskChain(TASK_CHAIN_NAME)
    assert task_manager.hasTaskChain(TASK_CHAIN_NAME)


def test_done(task_manager):
    # run-once task
    tm = task_manager
    l = []

    def _testDone(task, l=l):
        l.append(None)
        return task.done
    tm.add(_testDone, 'testDone')
    tm.step()
    assert len(l) == 1
    tm.step()
    assert len(l) == 1


def test_remove_by_name(task_manager):
    # remove by name
    tm = task_manager
    def _testRemoveByName(task):
        return task.done
    tm.add(_testRemoveByName, 'testRemoveByName')
    assert tm.remove('testRemoveByName') == 1
    assert tm.remove('testRemoveByName') == 0


def test_duplicate_named_tasks(task_manager):
    # duplicate named tasks
    tm = task_manager
    def _testDupNamedTasks(task):
        return task.done
    tm.add(_testDupNamedTasks, 'testDupNamedTasks')
    tm.add(_testDupNamedTasks, 'testDupNamedTasks')
    assert tm.remove('testRemoveByName') == 0


def test_continued_task(task_manager):
    # continued task
    tm = task_manager
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


def test_continue_until_done(task_manager):
    # continue until done task
    tm = task_manager
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


def test_has_task_named(task_manager):
    # hasTaskNamed
    tm = task_manager
    def _testHasTaskNamed(task):
        return task.done
    tm.add(_testHasTaskNamed, 'testHasTaskNamed')
    assert tm.hasTaskNamed('testHasTaskNamed')
    tm.step()
    assert not tm.hasTaskNamed('testHasTaskNamed')


def test_task_sort(task_manager):
    # task sort
    tm = task_manager
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


def test_extra_args(task_manager):
    # task extraArgs
    tm = task_manager
    l = []

    def _testExtraArgs(arg1, arg2, l=l):
        l.extend([arg1, arg2,])
        return Task.done
    tm.add(_testExtraArgs, 'testExtraArgs', extraArgs=[4,5])
    tm.step()
    assert len(l) == 2
    assert l == [4, 5,]


def test_append_task(task_manager):
    # task appendTask
    tm = task_manager
    l = []

    def _testAppendTask(arg1, arg2, task, l=l):
        l.extend([arg1, arg2,])
        return task.done
    tm.add(_testAppendTask, '_testAppendTask', extraArgs=[4,5], appendTask=True)
    tm.step()
    assert len(l) == 2
    assert l == [4, 5,]


def test_task_upon_death(task_manager):
    # task uponDeath
    tm = task_manager
    l = []

    def _uponDeathFunc(task, l=l):
        l.append(task.name)

    def _testUponDeath(task):
        return Task.done
    tm.add(_testUponDeath, 'testUponDeath', uponDeath=_uponDeathFunc)
    tm.step()
    assert len(l) == 1
    assert l == ['testUponDeath']


def test_task_owner(task_manager):
    # task owner
    tm = task_manager
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


def test_do_laters(task_manager):
    tm = task_manager
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

    # run the doLater tests
    while doLaterTests[0] > 0:
        tm.step()
    del doLaterTests


def test_get_tasks(task_manager):
    tm = task_manager
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


def test_get_do_laters(task_manager):
    tm = task_manager
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


def test_get_all_tasks(task_manager):
    active_task = task_manager.add(DUMMY_FUNCTION, delay=None)
    sleeping_task = task_manager.add(DUMMY_FUNCTION, delay=1)
    assert task_manager.getTasks() == [active_task]
    assert task_manager.getDoLaters() == [sleeping_task]
    assert task_manager.getAllTasks() in ([active_task, sleeping_task], [sleeping_task, active_task])


def test_duplicate_named_do_laters(task_manager):
    tm = task_manager
    # duplicate named doLaters removed via taskMgr.remove
    def _testDupNameDoLaters():
        pass
    # the doLaterProcessor is always running
    tm.doMethodLater(.1, _testDupNameDoLaters, 'testDupNameDoLater')
    tm.doMethodLater(.1, _testDupNameDoLaters, 'testDupNameDoLater')
    assert len(tm.getDoLaters()) == 2
    tm.remove('testDupNameDoLater')
    assert len(tm.getDoLaters()) == 0


def test_duplicate_named_do_laters_remove(task_manager):
    tm = task_manager
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


def test_get_tasks_named(task_manager):
    tm = task_manager
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


def test_get_tasks_matching(task_manager):
    task_manager.add(DUMMY_FUNCTION, 'task_1')
    task_manager.add(DUMMY_FUNCTION, 'task_2')
    task_manager.add(DUMMY_FUNCTION, 'another_task')

    assert len(task_manager.getTasksMatching('task_?')) == 2
    assert len(task_manager.getTasksMatching('*_task')) == 1
    assert len(task_manager.getTasksMatching('*task*')) == 3


def test_remove_tasks_matching(task_manager):
    tm = task_manager
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


def test_task_obj(task_manager):
    tm = task_manager
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


def test_task_remove(task_manager):
    tm = task_manager
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


def test_task_get_sort(task_manager):
    tm = task_manager
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
