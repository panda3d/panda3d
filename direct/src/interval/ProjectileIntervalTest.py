"""Undocumented Module"""

__all__ = ['doTest']

from panda3d.core import *
from panda3d.direct import *
from .IntervalGlobal import *

def doTest():
    smiley = loader.loadModel('models/misc/smiley')
    smiley.reparentTo(render)

    pi = ProjectileInterval(smiley, startPos=Point3(0, 0, 0),
                            endZ = -10, wayPoint=Point3(10, 0, 0),
                            timeToWayPoint=3)
    pi.loop()
    return pi

