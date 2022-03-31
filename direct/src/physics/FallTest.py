from panda3d.core import NodePath
from panda3d.physics import *


class FallTest(NodePath):
    def __init__(self):
        NodePath.__init__(self, "FallTest")

    def setup(self):
        # Connect to Physics Manager:
        self.actorNode = ActorNode("FallTestActorNode")
        #self.actorNode.getPhysicsObject().setOriented(1)
        #self.actorNode.getPhysical(0).setViscosity(0.1)

        actorNodePath = self.attachNewNode(self.actorNode)
        #self.setPos(avatarNodePath, Vec3(0))
        #self.setHpr(avatarNodePath, Vec3(0))

        avatarNodePath = base.loader.loadModel("models/misc/smiley")
        assert not avatarNodePath.isEmpty()

        camLL = base.render.find("**/camLL")
        camLL.reparentTo(avatarNodePath)
        camLL.setPosHpr(0, -10, 0, 0, 0, 0)
        avatarNodePath.reparentTo(actorNodePath)
        #avatarNodePath.setPos(Vec3(0))
        #avatarNodePath.setHpr(Vec3(0))
        #avatarNodePath.assign(physicsActor)
        #self.phys = PhysicsManager()
        self.phys = base.physicsMgr

        if 1:
            fn=ForceNode("FallTest gravity")
            fnp=NodePath(fn)
            fnp.reparentTo(self)
            fnp.reparentTo(base.render)
            gravity=LinearVectorForce(0.0, 0.0, -.5)
            fn.addForce(gravity)
            self.phys.addLinearForce(gravity)
            self.gravity = gravity

        if 0:
            fn=ForceNode("FallTest viscosity")
            fnp=NodePath(fn)
            fnp.reparentTo(self)
            fnp.reparentTo(base.render)
            self.avatarViscosity=LinearFrictionForce(0.0, 1.0, 0)
            #self.avatarViscosity.setCoef(0.9)
            fn.addForce(self.avatarViscosity)
            self.phys.addLinearForce(self.avatarViscosity)

        if 0:
            self.phys.attachLinearIntegrator(LinearEulerIntegrator())
        if 0:
            self.phys.attachAngularIntegrator(AngularEulerIntegrator())
        #self.phys.attachPhysicalNode(self.node())
        self.phys.attachPhysicalNode(self.actorNode)

        if 0:
            self.momentumForce=LinearVectorForce(0.0, 0.0, 0.0)
            fn=ForceNode("FallTest momentum")
            fnp=NodePath(fn)
            fnp.reparentTo(base.render)
            fn.addForce(self.momentumForce)
            self.phys.addLinearForce(self.momentumForce)

        if 0:
            self.acForce=LinearVectorForce(0.0, 0.0, 0.0)
            fn=ForceNode("FallTest avatarControls")
            fnp=NodePath(fn)
            fnp.reparentTo(base.render)
            fn.addForce(self.acForce)
            self.phys.addLinearForce(self.acForce)
            #self.phys.removeLinearForce(self.acForce)
            #fnp.remove()

        #avatarNodePath.reparentTo(base.render)
        self.avatarNodePath = avatarNodePath
        #self.actorNode.getPhysicsObject().resetPosition(self.avatarNodePath.getPos())
        #self.actorNode.updateTransform()

if __name__ == "__main__":
    from direct.directbase.ThreeUpStart import *
    test=FallTest()
    test.reparentTo(base.render)
    test.setup()
    base.camera.setY(-10.0)
    base.run()
