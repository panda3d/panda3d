"""
ShadowPlacer.py places a shadow.

It traces a line from a light source to the opposing surface.
Or it may do that later, right now it puts a node on the surface under
the its parent node.
"""

from ShowBaseGlobal import *

import DirectNotifyGlobal
import DirectObject

class ShadowPlacer(DirectObject.DirectObject):
    notify = DirectNotifyGlobal.directNotify.newCategory("ShadowPlacer")

    # special methods
    def __init__(self, cTrav, shadowNodePath, 
            wallCollideMask, floorCollideMask):
        assert self.debugPrint("ShadowPlacer()")
        DirectObject.DirectObject.__init__(self)
        self.setup(cTrav, shadowNodePath, 
            wallCollideMask, floorCollideMask)

    def setup(self, cTrav, shadowNodePath, 
            wallCollideMask, floorCollideMask):
        """
        Set up the collisions
        """
        assert self.debugPrint("setup()")
        assert not shadowNodePath.isEmpty()

        if not cTrav:
            # set up the shadow collision traverser
            if not base.shadowTrav:
                # set up the shadow collision traverser
                base.shadowTrav = CollisionTraverser()
            cTrav = base.shadowTrav

        self.cTrav = cTrav
        self.shadowNodePath = shadowNodePath

        floorOffset = 0.025
        # Set up the collison ray
        # This is a ray cast down to detect floor polygons
        self.cRay = CollisionRay(0.0, 0.0, 4.0, 0.0, 0.0, -1.0)
        self.cRayNode = CollisionNode('shadowPlacer')
        self.cRayNode.addSolid(self.cRay)
        self.cRayNodePath = shadowNodePath.getParent().attachNewNode(self.cRayNode)
        self.cRayBitMask = floorCollideMask
        self.cRayNode.setFromCollideMask(self.cRayBitMask)
        self.cRayNode.setIntoCollideMask(BitMask32.allOff())

        # set up floor collision mechanism
        self.lifter = CollisionHandlerFloor()
        #*#self.lifter.setWrtFlag(1)
        #self.lifter.setInPattern("on-floor")
        #self.lifter.setOutPattern("off-floor")
        self.lifter.setOffset(floorOffset)

        # activate the collider with the traverser and pusher
        self.on()
        
        self.lifter.addColliderNode(self.cRayNode, shadowNodePath.node())

    def delete(self):
        assert self.debugPrint("delete()")
        del self.cTrav

        del self.shadowNodePath

        del self.cRay
        del self.cRayNode
        self.cRayNodePath.removeNode()
        del self.cRayNodePath

        del self.lifter

    def off(self):
        """
        Turn off the shadow placement.  The shadow will still be
        there, but the z position will not be updated until a call
        to on() is made.
        """
        assert self.debugPrint("off()")
        self.cTrav.removeCollider(self.cRayNode)
        # Now that we have disabled collisions, make one more pass
        # right now to ensure we aren't standing in a wall.
        self.oneTimeCollide()

    def on(self):
        """
        Turn on the shadow placement.  The shadow z position will
        start being updated until a call to off() is made.
        """
        assert self.debugPrint("on()")
        self.cTrav.addCollider(self.cRayNode, self.lifter)

    def oneTimeCollide(self):
        """
        Makes one quick collision pass for the avatar, for instance as
        a one-time straighten-things-up operation after collisions
        have been disabled.
        """
        tempCTrav = CollisionTraverser()
        tempCTrav.addCollider(self.cRayNode, self.lifter)
        tempCTrav.traverse(render)
    
    if __debug__:
        def debugPrint(self, message):
            """for debugging"""
            return self.notify.debug(
                    str(id(self))+' '+message)
