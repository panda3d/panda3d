
class RotationTest(NodePath):
    def __init__(self):
        NodePath.__init__(self, "RotationTest")

    def setup(self):
        # Connect to Physics Manager:
        self.actorNode=ActorNode("RotationTestActorNode")
        #self.actorNode.getPhysicsObject().setOriented(1)
        #self.actorNode.getPhysical(0).setViscosity(0.1)

        actorNodePath=self.attachNewNode(self.actorNode)
        #self.setPos(avatarNodePath, Vec3(0))
        #self.setHpr(avatarNodePath, Vec3(0))

        avatarNodePath=loader.loadModel("models/misc/smiley")
        assert not avatarNodePath.isEmpty()

        camLL=render.find("**/camLL")
        camLL.reparentTo(avatarNodePath)
        camLL.setPosHpr(0, -10, 0, 0, 0, 0)
        avatarNodePath.reparentTo(actorNodePath)
        #avatarNodePath.setPos(Vec3(0))
        #avatarNodePath.setHpr(Vec3(0))
        #avatarNodePath.assign(physicsActor)
        #self.phys=PhysicsManager()
        self.phys=base.physicsMgr

        if 0:
            fn=ForceNode("RotationTest gravity")
            fnp=NodePath(fn)
            fnp.reparentTo(self)
            fnp.reparentTo(render)
            gravity=LinearVectorForce(0.0, 0.0, -.5)
            fn.addForce(gravity)
            self.phys.addLinearForce(gravity)
            self.gravity = gravity

        if 1:
            fn=ForceNode("RotationTest spin")
            fnp=NodePath(fn)
            fnp.reparentTo(self)
            fnp.reparentTo(render)
            spin=AngularVectorForce(0.0, 0.0, 0.5)
            fn.addForce(spin)
            self.phys.addAngularForce(spin)
            self.spin = spin

        if 0:
            fn=ForceNode("RotationTest viscosity")
            fnp=NodePath(fn)
            fnp.reparentTo(self)
            fnp.reparentTo(render)
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
            fn=ForceNode("RotationTest momentum")
            fnp=NodePath(fn)
            fnp.reparentTo(render)
            fn.addForce(self.momentumForce)
            self.phys.addLinearForce(self.momentumForce)

        if 0:
            self.acForce=LinearVectorForce(0.0, 0.0, 0.0)
            fn=ForceNode("RotationTest avatarControls")
            fnp=NodePath(fn)
            fnp.reparentTo(render)
            fn.addForce(self.acForce)
            self.phys.addLinearForce(self.acForce)
            #self.phys.removeLinearForce(self.acForce)
            #fnp.remove()

        #avatarNodePath.reparentTo(render)
        self.avatarNodePath = avatarNodePath
        #self.actorNode.getPhysicsObject().resetPosition(self.avatarNodePath.getPos())
        #self.actorNode.updateTransform()

if __name__ == "__main__":
    from direct.directbase.ThreeUpStart import *
    test=RotationTest()
    test.reparentTo(render)
    test.setup()
    camera.setY(-10.0)
    run()
