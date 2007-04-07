from direct.showbase.DirectObject import DirectObject
from direct.directnotify import DirectNotifyGlobal
from pandac.PandaModules import *
from PhasedObject import PhasedObject

class DistancePhasedNode(NodePath, DirectObject, PhasedObject):
    """
    This class defines a PhasedObject,NodePath object that will handle the phasing
    of an object in the scene graph according to its distance from some 
    other collider object(such as an avatar).

    Since it's a NodePath, you can parent it to another object in the
    scene graph, or even inherit from this class to get its functionality.

    What you will need to define to use this class:
     - The distances at which you want the phases to load/unload
     - What the load/unload functions are
     - What sort of events to listen for to signal a collision
     - (Optional) - a collision bitmask for the phase collision spheres.

    You specify the distances and function names by the phaseParamMap
    parameter to __init__().  For example:

    phaseParamMap = {'Alias': distance, ...}
    ...
    def loadPhaseAlias(self):
        pass
    def unloadPhaseAlias(self):
        pass

    IMPORTANT!: If you unload the last phase, by either calling
                cleanup() or by exitting the last phase's distance,
                you will need to explicitly call reset() to get the
                distance phasing to work again. This was done so if
                either this node or the collider is removed from the
                scene graph(eg. avatar teleport), the phased object
                will clean itself up automatically.
    """

    notify = directNotify.newCategory("DistancePhasedObject")
    __InstanceSequence = 0
    __InstanceDeque = []
        
    @staticmethod
    def __allocateId():
        if DistancePhasedNode.__InstanceDeque:
            return DistancePhasedNode.__InstanceDeque.pop(0)
        else:
            id = DistancePhasedNode.__InstanceSequence
            DistancePhasedNode.__InstanceSequence += 1
            DistancePhasedNode.__InstanceSequence &= 65535
            return id

    @staticmethod
    def __deallocateId(id):
        DistancePhasedNode.__InstanceDeque.append(id)

    def __init__(self, name, phaseParamMap = {},
                 enterPrefix = 'enter', exitPrefix = 'exit', phaseCollideMask = BitMask32.allOn()):
        NodePath.__init__(self, name)
        self.phaseParamMap = phaseParamMap
        self.phaseParamList = sorted(phaseParamMap.items(),
                                     key = lambda x: x[1],
                                     reverse = True)
        PhasedObject.__init__(self,
                              dict([(alias,phase) for (phase,alias) in enumerate([item[0] for item in self.phaseParamList])]))
        self.__id = self.__allocateId()

        
        self.phaseCollideMask = phaseCollideMask
        self.enterPrefix = enterPrefix
        self.exitPrefix = exitPrefix
        self._colSpheres = []
        
        self.reset()

    def __del__(self):
        self.__deallocateId(self.__id)

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
            cName = 'Phase%s-%d' % (name, self.__id)
            cSphereNode = CollisionNode(cName)
            cSphereNode.setIntoCollideMask(self.phaseCollideMask)
            cSphereNode.addSolid(cSphere)
            cSphereNodePath = self.attachNewNode(cSphereNode)
            cSphereNodePath.show()
            cSphereNodePath.stash()
            self._colSpheres.append(cSphereNodePath)

        self.__enableCollisions(-1, startup = True)
        
    def setPhase(self, aPhase):
        """
        See PhasedObject.setPhase()
        """
        phase = self.getAliasPhase(aPhase)
        PhasedObject.setPhase(self, aPhase)
        self.__disableCollisions(cleanup = (phase == -1))
        self.__enableCollisions(phase)
        
        if phase == -1:
            self.cleanup()
        else:
            self.__oneTimeCollide()
        
    def __getEnterEvent(self, phaseName):
        return '%sPhase%s-%d' % (self.enterPrefix, phaseName, self.__id)

    def __getExitEvent(self, phaseName):
        return '%sPhase%s-%d' % (self.exitPrefix, phaseName, self.__id)
    
    def __enableCollisions(self, phase, startup = False):
        if startup:
            phaseName = self.getPhaseAlias(0)
            self.accept(self.__getExitEvent(phaseName),
                        self.__handleExitEvent,
                        extraArgs = [phaseName])
            self._colSpheres[0].unstash()
            
        if 0 <= phase:
            phaseName = self.getPhaseAlias(phase)
            self.accept(self.__getExitEvent(phaseName),
                        self.__handleExitEvent,
                        extraArgs = [phaseName])
            self._colSpheres[phase].unstash()
            
        if 0 <= phase < len(self._colSpheres)-1 or startup:
            phaseName = self.getPhaseAlias(phase + 1)
            self.accept(self.__getEnterEvent(phaseName),
                        self.__handleEnterEvent,
                        extraArgs = [phaseName])
            self._colSpheres[phase+1].unstash()

    def __disableCollisions(self, cleanup = False):
        for x,sphere in enumerate(self._colSpheres):
            phaseName = self.getPhaseAlias(x)
            self.ignore(self.__getEnterEvent(phaseName))
            if x > 0 or cleanup:
                sphere.stash()
                self.ignore(self.__getExitEvent(phaseName))
        
    def __handleEnterEvent(self, phaseName, cEntry):
        print cEntry
        self.setPhase(phaseName)

    def __handleExitEvent(self, phaseName, cEntry):
        print cEntry
        phase = self.getAliasPhase(phaseName) - 1
        self.setPhase(phase)

    def __oneTimeCollide(self):
        base.cTrav.traverse(self)
        base.eventMgr.doEvents()
        
class BufferedDistancePhasedNode(DistancePhasedNode):
    """
    This class is similar to DistancePhasedNode except you can also
    specify a buffer distance for each phase.  Upon entering that phase,
    its distance will be increased by the buffer amount.  Likewise,
    upon leaving the distance will be decremented by that amount, back
    to it's original size.  In this manner, you can avoid the problem
    of 'phase flicker' as someone repeatedly steps across a static phase
    border.

    You specify the buffer amount in the bufferParamMap parameter
    to __init__().  It has this format:

    bufferParamMap = {'alias':(distance, bufferAmount), ...}
    """
    notify = directNotify.newCategory("BufferedDistancePhasedObject")

    def __init__(self, name, bufferParamMap = {}):
        sParams = dict(bufferParamMap)
        for key in sParams:
            sParams[key] = sParams[key][0]
        DistancePhasedNode.__init__(self, name, sParams)
        self.bufferParamMap = bufferParamMap
        self.bufferParamList = sorted(bufferParamMap.items(),
                                      key = lambda x: x[1],
                                      reverse = True)

    def setPhase(self, aPhase):
        """
        see DistancePhasedNode.setPhase()
        """
        DistancePhasedNode.setPhase(self, aPhase)
        phase = self.getAliasPhase(aPhase)
        self.__adjustCollisions(phase)

    def __adjustCollisions(self, phase):
        for x,sphere in enumerate(self._colSpheres[:phase+1]):
            sphere.node().getSolid(0).setRadius(self.bufferParamList[x][1][1])
            sphere.node().markInternalBoundsStale()

        for x,sphere in enumerate(self._colSpheres[phase+1:]):
            sphere.node().getSolid(0).setRadius(self.bufferParamList[x+phase+1][1][0])
            sphere.node().markInternalBoundsStale()
            
if __debug__ and 0:
    cSphere = CollisionSphere(0,0,0,0.1)
    cNode = CollisionNode('camCol')
    cNode.addSolid(cSphere)
    cNodePath = NodePath(cNode)
    cNodePath.reparentTo(base.cam)
    #cNodePath.show()
    #cNodePath.setPos(25,0,0)
    
    base.cTrav = CollisionTraverser()
    
    eventHandler = CollisionHandlerEvent()
    eventHandler.addInPattern('enter%in')
    eventHandler.addOutPattern('exit%in')
    
    # messenger.toggleVerbose()
    base.cTrav.addCollider(cNodePath,eventHandler)

    p = BufferedDistancePhasedNode('p',{'At':(10,20),'Near':(100,200),'Far':(1000, 1020)})

    p.reparentTo(render)
    p._DistancePhasedNode__oneTimeCollide()
    base.eventMgr.doEvents()

