from pandac.PandaModules import *
from direct.directbase.DirectStart import *
from IntervalGlobal import *

smiley = loader.loadModel('models/misc/smiley')
smiley.reparentTo(render)

def doTest():
    pi = ProjectileInterval(smiley, startPos=Point3(0,0,0),
                            endZ = -10, wayPoint=Point3(10,0,0),
                            timeToWayPoint=3)
    pi.loop()
    return pi
    
