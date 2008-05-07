"""Undocumented Module"""

__all__ = ['ShadowPlacer']

"""
ShadowPlacer.py places a shadow.

It traces a line from a light source to the opposing surface.
Or it may do that later, right now it puts a node on the surface under
the its parent node.
"""

from direct.controls.ControlManager import CollisionHandlerRayStart
from direct.directnotify import DirectNotifyGlobal
from pandac.PandaModules import *
import DirectObject

class ShadowPlacer(DirectObject.DirectObject):
    notify = DirectNotifyGlobal.directNotify.newCategory("ShadowPlacer")
    
    if __debug__:
        count = 0
        activeCount = 0

    # special methods
    def __init__(self, cTrav, shadowNodePath,
            wallCollideMask, floorCollideMask):
        self.isActive = 0 # Is the placer "on".  This is also printed in the debugCall.
        assert self.notify.debugCall()
        DirectObject.DirectObject.__init__(self)
        self.setup(cTrav, shadowNodePath,
            wallCollideMask, floorCollideMask)
        if __debug__:
            self.count += 1
            self.debugDisplay()

    def setup(self, cTrav, shadowNodePath,
            wallCollideMask, floorCollideMask):
        """
        Set up the collisions
        """
        assert self.notify.debugCall()
        assert not shadowNodePath.isEmpty()
        assert not hasattr(self, "cTrav") # Protect from setup() being called again.

        if not cTrav:
            # set up the shadow collision traverser
            base.initShadowTrav()
            cTrav = base.shadowTrav

        self.cTrav = cTrav
        self.shadowNodePath = shadowNodePath

        floorOffset = 0.025
        # Set up the collison ray
        # This is a ray cast down to detect floor polygons
        self.cRay = CollisionRay(0.0, 0.0, CollisionHandlerRayStart, 0.0, 0.0, -1.0)
        cRayNode = CollisionNode('shadowPlacer')
        cRayNode.addSolid(self.cRay)
        self.cRayNodePath = NodePath(cRayNode)
        self.cRayBitMask = floorCollideMask
        cRayNode.setFromCollideMask(self.cRayBitMask)
        cRayNode.setIntoCollideMask(BitMask32.allOff())

        # set up floor collision mechanism
        self.lifter = CollisionHandlerFloor()
        #self.lifter.setInPattern("on-floor")
        #self.lifter.setOutPattern("off-floor")
        self.lifter.setOffset(floorOffset)
        self.lifter.setReach(4.0)

        # activate the collider with the traverser and pusher
        #self.on()
        
        self.lifter.addCollider(self.cRayNodePath, shadowNodePath)

    def delete(self):
        assert self.notify.debugCall()
        self.off()
        if __debug__:
            assert not self.isActive
            self.count -= 1
            self.debugDisplay()
        del self.cTrav

        del self.shadowNodePath

        del self.cRay
        #del self.cRayNode
        self.cRayNodePath.removeNode()
        del self.cRayNodePath

        del self.lifter

    def on(self):
        """
        Turn on the shadow placement.  The shadow z position will
        start being updated until a call to off() is made.
        """
        assert self.notify.debugCall("activeCount=%s"%(self.activeCount,))
        if self.isActive:
            assert self.cTrav.hasCollider(self.cRayNodePath)
            return
        assert not self.cTrav.hasCollider(self.cRayNodePath)
        self.cRayNodePath.reparentTo(self.shadowNodePath.getParent())
        self.cTrav.addCollider(self.cRayNodePath, self.lifter)
        self.isActive = 1
        if __debug__:
            self.activeCount += 1
            self.debugDisplay()

    def off(self):
        """
        Turn off the shadow placement.  The shadow will still be
        there, but the z position will not be updated until a call
        to on() is made.
        """
        assert self.notify.debugCall("activeCount=%s"%(self.activeCount,))
        if not self.isActive:
            assert not self.cTrav.hasCollider(self.cRayNodePath)
            return
        assert self.cTrav.hasCollider(self.cRayNodePath)
        didIt = self.cTrav.removeCollider(self.cRayNodePath)
        assert didIt
        self.cRayNodePath.detachNode()
        # Now that we have disabled collisions, make one more pass
        # right now to ensure we aren't standing in a wall.
        self.oneTimeCollide()
        self.isActive = 0
        if __debug__:
            self.activeCount -= 1
            self.debugDisplay()

    def oneTimeCollide(self):
        """
        Makes one quick collision pass for the avatar, for instance as
        a one-time straighten-things-up operation after collisions
        have been disabled.
        """
        assert self.notify.debugCall()
        tempCTrav = CollisionTraverser("oneTimeCollide")
        tempCTrav.addCollider(self.cRayNodePath, self.lifter)
        tempCTrav.traverse(render)

    def resetToOrigin(self):
        if self.shadowNodePath:
            self.shadowNodePath.setPos(0,0,0)
        
    if __debug__:
        def debugDisplay(self):
            """for debugging"""
            if self.notify.getDebug():
                message = "%d active (%d total), %d colliders"%(
                self.activeCount, self.count, self.cTrav.getNumColliders())
                self.notify.debug(message)
                onScreenDebug.add("ShadowPlacers", message)
            return 1 # to allow assert self.debugDisplay()
