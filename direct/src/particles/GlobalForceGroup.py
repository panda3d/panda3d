from . import ForceGroup
from direct.showbase.PhysicsManagerGlobal import physicsMgr


class GlobalForceGroup(ForceGroup.ForceGroup):

    def __init__(self, name = None):
        ForceGroup.ForceGroup.__init__(self, name)

    def addForce(self, force):
        ForceGroup.ForceGroup.addForce(self, force)
        if (force.isLinear() == 0):
            # Physics manager will need an angular integrator
            base.addAngularIntegrator()
        if (force.isLinear() == 1):
            physicsMgr.addLinearForce(force)
        else:
            physicsMgr.addAngularForce(force)

    def removeForce(self, force):
        ForceGroup.ForceGroup.removeForce(self, force)
        if (force.isLinear() == 1):
            physicsMgr.removeLinearForce(force)
        else:
            physicsMgr.removeAngularForce(force)
