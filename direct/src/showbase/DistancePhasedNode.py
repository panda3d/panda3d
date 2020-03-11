from direct.showbase.DirectObject import DirectObject
from direct.directnotify.DirectNotifyGlobal import directNotify
from panda3d.core import *
from .PhasedObject import PhasedObject


class DistancePhasedNode(PhasedObject, DirectObject, NodePath):
    """
    This class defines a PhasedObject,NodePath object that will handle
    the phasing of an object in the scene graph according to its
    distance from some other collider object(such as an avatar).

    Since it's a NodePath, you can parent it to another object in the
    scene graph, or even inherit from this class to get its functionality.

    What you will need to define to use this class:

     - The distances at which you want the phases to load/unload
     - Whether you want the object to clean itself up or not when
       exitting the largest distance sphere
     - What the load/unload functions are
     - What sort of events to listen for when a collision occurs
     - (Optional) A collision bitmask for the phase collision spheres
     - (Optional) A 'from' collision node to collide into our 'into' spheres

    You specify the distances and function names by the phaseParamMap
    parameter to `__init__()`.  For example::

        phaseParamMap = {'Alias': distance, ...}
        ...
        def loadPhaseAlias(self):
            pass
        def unloadPhaseAlias(self):
            pass

    If the 'fromCollideNode' is supplied, we will set up our own
    traverser and only traverse below this node.  It will send out
    events of the form '<enterPrefix>%in' and '<exitPrefix>%in' in
    order to match the main collision traverser's patterns.  Note
    that this will only be used after a reset or phase change in
    order to fully transition to the correct phase in a single pass.
    Most of the time, it will be reacting to events from the main
    collision traverser.

    IMPORTANT:

        The following only applies when ``autoCleanup is True``:
        If you unload the last phase, by either calling `cleanup()` or
        by exiting the last phase's distance, you will need to
        explicitly call `reset()` to get the distance phasing to work
        again. This was done so if either this node or the collider is
        removed from the scene graph (e.g. avatar teleport), the phased
        object will clean itself up automatically.
    """

    notify = directNotify.newCategory("DistancePhasedObject")
    __InstanceSequence = 0
    __InstanceDeque = []

    @staticmethod
    def __allocateId():
        """
        Give each phase node a unique id in order to filter out
        collision events from other phase nodes.  We do it in
        this manner so the client doesn't need to worry about
        giving each phase node a unique name.
        """
        if DistancePhasedNode.__InstanceDeque:
            return DistancePhasedNode.__InstanceDeque.pop(0)
        else:
            id = DistancePhasedNode.__InstanceSequence
            DistancePhasedNode.__InstanceSequence += 1
            DistancePhasedNode.__InstanceSequence &= 65535
            return id

    @staticmethod
    def __deallocateId(id):
        """
        Reuse abandoned ids.
        """
        DistancePhasedNode.__InstanceDeque.append(id)

    def __init__(self, name, phaseParamMap = {},
                 autoCleanup = True,
                 enterPrefix = 'enter', exitPrefix = 'exit',
                 phaseCollideMask = BitMask32.allOn(),
                 fromCollideNode = None):
        NodePath.__init__(self, name)
        self.phaseParamMap = phaseParamMap
        self.phaseParamList = sorted(list(phaseParamMap.items()),
                                     key = lambda x: x[1],
                                     reverse = True)
        PhasedObject.__init__(self,
                              dict([(alias,phase) for (phase,alias) in enumerate([item[0] for item in self.phaseParamList])]))
        self.__id = self.__allocateId()

        self.autoCleanup = autoCleanup
        self.enterPrefix = enterPrefix
        self.exitPrefix = exitPrefix
        self.phaseCollideMask = phaseCollideMask
        self.cTrav = base.cTrav
        self.fromCollideNode = fromCollideNode
        self._colSpheres = []

        self.reset()

    def __del__(self):
        self.__deallocateId(self.__id)

    def __repr__(self):
        outStr = 'DistancePhasedObject('
        outStr += repr(self.getName())
        for param, value in zip(('phaseParamMap', 'autoCleanup', 'enterPrefix', 'exitPrefix', 'phaseCollideMask', 'fromCollideNode'),
                                ({}, True, 'enter', 'exit', BitMask32.allOn(), None)):
            pv = getattr(self, param)
            if pv != value:
                outStr += ', %s = %r' % (param, pv)
        outStr += ')'
        return outStr

    def __str__(self):
        return '%s in phase \'%s\'' % (NodePath.__str__(self), self.getPhase())

    def cleanup(self):
        """
        Disables all collisions.
        Ignores all owned event listeners.
        Unloads all unloaded phases.
        """
        self.__disableCollisions(cleanup = True)
        for sphere in self._colSpheres:
            sphere.remove()
        self._colSpheres = []
        PhasedObject.cleanup(self)

    def setPhaseCollideMask(self, mask):
        """
        Sets the intoCollideMasks for our collision spheres.
        """
        self.phaseCollideMask = mask
        for sphere in self._colSpheres:
            self.colSphere.node().setIntoCollideMask(self.phaseCollideMask)

    def reset(self):
        """
        Unloads all loaded phases and puts the phase node
        in the startup state is if it had just been initialized.
        """
        self.cleanup()
        self.__oneTimeCollide()
        for name, dist in self.phaseParamList:
            cSphere = CollisionSphere(0.0, 0.0, 0.0, dist)
            cSphere.setTangible(0)
            cName = 'PhaseNode%s-%d' % (name, self.__id)
            cSphereNode = CollisionNode(cName)
            cSphereNode.setIntoCollideMask(self.phaseCollideMask)
            cSphereNode.setFromCollideMask(BitMask32.allOff())
            cSphereNode.addSolid(cSphere)
            cSphereNodePath = self.attachNewNode(cSphereNode)
            cSphereNodePath.stash()
            # cSphereNodePath.show() # For debugging
            self._colSpheres.append(cSphereNodePath)
        if self.fromCollideNode:
            self.cTrav = CollisionTraverser()
            cHandler = CollisionHandlerEvent()
            cHandler.addInPattern(self.enterPrefix + '%in')
            cHandler.addOutPattern(self.exitPrefix + '%in')
            self.cTrav.addCollider(self.fromCollideNode,cHandler)

        self.__enableCollisions(-1)

    def setPhase(self, aPhase):
        """
        See PhasedObject.setPhase()
        """
        phase = self.getAliasPhase(aPhase)
        PhasedObject.setPhase(self, aPhase)
        self.__disableCollisions()
        self.__enableCollisions(phase)

        if phase == -1 and self.autoCleanup:
            self.cleanup()
        else:
            self.__oneTimeCollide()

    def __getEnterEvent(self, phaseName):
        return '%sPhaseNode%s-%d' % (self.enterPrefix, phaseName, self.__id)

    def __getExitEvent(self, phaseName):
        return '%sPhaseNode%s-%d' % (self.exitPrefix, phaseName, self.__id)

    def __enableCollisions(self, phase):
        """
        Turns on collisions for the spheres bounding this
        phase zone by unstashing their geometry.  Enables
        the exit event for the larger and the enter event
        for the smaller.  Handles the  extreme(end) phases
        gracefully.
        """
        if 0 <= phase:
            phaseName = self.getPhaseAlias(phase)
            self.accept(self.__getExitEvent(phaseName),
                        self.__handleExitEvent,
                        extraArgs = [phaseName])
            self._colSpheres[phase].unstash()

        if 0 <= phase+1 < len(self._colSpheres):
            phaseName = self.getPhaseAlias(phase+1)
            self.accept(self.__getEnterEvent(phaseName),
                        self.__handleEnterEvent,
                        extraArgs = [phaseName])
            self._colSpheres[phase+1].unstash()

    def __disableCollisions(self, cleanup = False):
        """
        Disables all collision geometry by stashing
        the geometry.  If autoCleanup == True and we're
        not currently cleaning up, leave the exit event
        and collision sphere active for the largest(thus lowest)
        phase.  This is so that we can still cleanup if
        the phase node exits the largest sphere.
        """
        for x,sphere in enumerate(self._colSpheres):
            phaseName = self.getPhaseAlias(x)
            self.ignore(self.__getEnterEvent(phaseName))
            if x > 0 or not self.autoCleanup or cleanup:
                sphere.stash()
                self.ignore(self.__getExitEvent(phaseName))

    def __handleEnterEvent(self, phaseName, cEntry):
        self.setPhase(phaseName)

    def __handleExitEvent(self, phaseName, cEntry):
        phase = self.getAliasPhase(phaseName) - 1
        self.setPhase(phase)

    def __oneTimeCollide(self):
        """
        Fire off a one-time collision traversal of the
        scene graph.  This allows us to process our entire
        phasing process in one frame in the cases where
        we cross more than one phase border.
        """
        if self.cTrav:
            if self.cTrav is base.cTrav:
                # we use 'render'here since if we only try to
                # traverse ourself, we end up calling exit
                # events for the rest of the eventHandlers.
                # Consider supplying the fromCollideNode parameter.
                self.cTrav.traverse(render)
            else:
                # Only traverse ourself
                self.cTrav.traverse(self)
            base.eventMgr.doEvents()

class BufferedDistancePhasedNode(DistancePhasedNode):
    """
    This class is similar to DistancePhasedNode except you can also
    specify a buffer distance for each phase.  Upon entering that phase,
    its distance will be increased by the buffer amount.  Conversely,
    the distance will be decremented by that amount, back to its
    original size, upon leaving.  In this manner, you can avoid the problem
    of 'phase flicker' as someone repeatedly steps across a static phase
    border.

    You specify the buffer amount in the bufferParamMap parameter
    to :meth:`__init__()`.  It has this format::

        bufferParamMap = {'alias':(distance, bufferAmount), ...}
    """
    notify = directNotify.newCategory("BufferedDistancePhasedObject")

    def __init__(self, name, bufferParamMap = {}, autoCleanup = True,
                 enterPrefix = 'enter', exitPrefix = 'exit', phaseCollideMask = BitMask32.allOn(), fromCollideNode = None):
        self.bufferParamMap = bufferParamMap
        self.bufferParamList = sorted(list(bufferParamMap.items()),
                                      key = lambda x: x[1],
                                      reverse = True)

        sParams = dict(bufferParamMap)
        for key in sParams:
            sParams[key] = sParams[key][0]

        DistancePhasedNode.__init__(self, name = name,
                                    phaseParamMap = sParams,
                                    autoCleanup = autoCleanup,
                                    enterPrefix = enterPrefix,
                                    exitPrefix = exitPrefix,
                                    phaseCollideMask = phaseCollideMask,
                                    fromCollideNode = fromCollideNode)

    def __repr__(self):
        outStr = 'BufferedDistancePhasedNode('
        outStr += repr(self.getName())
        for param, value in zip(('bufferParamMap', 'autoCleanup', 'enterPrefix', 'exitPrefix', 'phaseCollideMask', 'fromCollideNode'),
                                ({}, True, 'enter', 'exit', BitMask32.allOn(), None)):
            pv = getattr(self, param)
            if pv != value:
                outStr += ', %s = %r' % (param, pv)
        outStr += ')'
        return outStr

    def __str__(self):
        return '%s in phase \'%s\'' % (NodePath.__str__(self), self.getPhase())

    def setPhase(self, aPhase):
        """
        see DistancePhasedNode.setPhase()
        """
        DistancePhasedNode.setPhase(self, aPhase)
        phase = self.getAliasPhase(aPhase)
        self.__adjustCollisions(phase)

    def __adjustCollisions(self, phase):
        for x,sphere in enumerate(self._colSpheres[:phase+1]):
            sphere.node().modifySolid(0).setRadius(self.bufferParamList[x][1][1])
            sphere.node().markInternalBoundsStale()

        for x,sphere in enumerate(self._colSpheres[phase+1:]):
            sphere.node().modifySolid(0).setRadius(self.bufferParamList[x+phase+1][1][0])
            sphere.node().markInternalBoundsStale()


if __debug__ and 0:
    cSphere = CollisionSphere(0,0,0,0.1)
    cNode = CollisionNode('camCol')
    cNode.addSolid(cSphere)
    cNodePath = NodePath(cNode)
    cNodePath.reparentTo(base.cam)
    # cNodePath.show()
    # cNodePath.setPos(25,0,0)

    base.cTrav = CollisionTraverser()

    eventHandler = CollisionHandlerEvent()
    eventHandler.addInPattern('enter%in')
    eventHandler.addOutPattern('exit%in')

    # messenger.toggleVerbose()
    base.cTrav.addCollider(cNodePath,eventHandler)

    p = BufferedDistancePhasedNode('p',{'At':(10,20),'Near':(100,200),'Far':(1000, 1020)},
                                   autoCleanup = False,
                                   fromCollideNode = cNodePath,
                                   )

    p.reparentTo(render)
    p._DistancePhasedNode__oneTimeCollide()
    base.eventMgr.doEvents()
