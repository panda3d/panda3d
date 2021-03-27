from direct.showbase.ShowBase import ShowBase
from direct.task import Task
from panda3d.core import Vec2


def test_taskArg():
    def test(ship, flood, task):
        ship.y += flood
        return task.cont

    ship = Vec2(2.2, 2)
    flood = 1

    base = ShowBase(windowType='none')
    task = base.add_task(test, 'test_task', extraArgs=[ship, flood], appendTask=True)
    base.taskMgr.step()
    assert ship.y == 3
    base.remove_task(task)
    task = base.add_task(task)
    base.taskMgr.step()
    assert ship.y == 4