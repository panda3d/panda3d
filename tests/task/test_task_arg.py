from direct.showbase.ShowBase import ShowBase
from direct.task import Task
from panda3d.core import Vec2

def test_task_arg(base):
    def test(ship, flood, task):
        ship.y += flood
        return task.done

    ship = Vec2(2.2, 2)
    flood = 1

    task = base.addTask(test, 'test_task', extraArgs=[ship, flood], appendTask=True)
    base.taskMgr.step()
    assert ship.y == 3
    base.remove_task(task)
    task = base.addTask(task)
    base.taskMgr.step()
    assert ship.y == 4
    task = base.taskMgr.add(test, 'test_task', extraArgs=[ship, flood], appendTask=True)
    base.taskMgr.step()
    assert ship.y == 5
    base.remove_task(task)
    task = base.taskMgr.add(task)
    base.taskMgr.step()
    assert ship.y == 6
