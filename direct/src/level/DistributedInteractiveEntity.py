""" DistributedInteractiveEntity module: contains the DistributedInteractiveEntity
    class, the client side representation of a 'landmark door'."""

from ShowBaseGlobal import *
from ClockDelta import *

import DirectNotifyGlobal
import FSM
import DistributedEntity

class DistributedInteractiveEntity(DistributedEntity.DistributedEntity):
    """
    DistributedInteractiveEntity class:  The client side representation of any
    simple animated prop.
    """

    if __debug__:
        notify = DirectNotifyGlobal.directNotify.newCategory('DistributedInteractiveEntity')

    def __init__(self, cr):
        """constructor for the DistributedInteractiveEntity"""
        DistributedEntity.DistributedEntity.__init__(self, cr)
        assert(self.debugPrint("__init()"))

        self.fsm = FSM.FSM('DistributedInteractiveEntity',
                           [State.State('off',
                                        self.enterOff,
                                        self.exitOff,
                                        ['playing',
                                        'attract']),
                            State.State('attract',
                                        self.enterAttract,
                                        self.exitAttract,
                                        ['playing']),
                            State.State('playing',
                                        self.enterPlaying,
                                        self.exitPlaying,
                                        ['attract'])],
                           # Initial State
                           'off',
                           # Final State
                           'off',
                          )
        self.fsm.enterInitialState()
        # self.generate will be called automatically.
        
    def generate(self):
        """generate(self)
        This method is called when the DistributedEntity is introduced
        to the world, either for the first time or from the cache.
        """
        assert(self.debugPrint("generate()"))
        DistributedEntity.DistributedEntity.generate(self)

    def disable(self):
        assert(self.debugPrint("disable()"))
        # Go to the off state when the object is put in the cache
        self.fsm.request("off")
        DistributedEntity.DistributedEntity.disable(self)
        # self.delete() will automatically be called.
    
    def delete(self):
        assert(self.debugPrint("delete()"))
        del self.fsm
        DistributedEntity.DistributedEntity.delete(self)
    
    def setAvatarInteract(self, avatarId):
        """
        required dc field.
        """
        assert(self.debugPrint("setAvatarInteract(%s)"%(avatarId,)))
        assert(not self.__dict__.has_key(avatarId))
        self.avatarId=avatarId
    
    def setOwnerDoId(self, ownerDoId):
        """
        required dc field.
        """
        assert(self.debugPrint("setOwnerDoId(%s)"%(ownerDoId,)))
        assert(not self.__dict__.has_key("ownerDoId"))
        self.ownerDoId=ownerDoId
    
    def setInitialState(self, state, timestamp):
        """
        required dc field.
        """
        assert(self.debugPrint("setInitialState(%s, %d)" % (state, timestamp)))
        assert(not self.__dict__.has_key("initialState"))
        self.initialState = state
        self.initialStateTimestamp = timestamp
        
    def setState(self, state, timestamp):
        assert(self.debugPrint("setState(%s, %d)" % (state, timestamp)))
        self.fsm.request(state, [globalClockDelta.localElapsedTime(timestamp)])
    
    #def __getPropNodePath(self):
    #    assert(self.debugPrint("__getPropNodePath()"))
    #    if (not self.__dict__.has_key('propNodePath')):
    #        self.propNodePath=self.cr.playGame.hood.loader.geom.find(
    #                "**/prop"+self.entID+":*_DNARoot")
    #    return self.propNodePath
    
    def enterTrigger(self, args=None):
        assert(self.debugPrint("enterTrigger(args="+str(args)+")"))
        messenger.send("DistributedInteractiveEntity_enterTrigger")
        self.sendUpdate("requestInteract")
        # the AI server will reply with toonInteract or rejectInteract.
    
    def exitTrigger(self, args=None):
        assert(self.debugPrint("exitTrigger(args="+str(args)+")"))
        messenger.send("DistributedInteractiveEntity_exitTrigger")
        self.sendUpdate("requestExit")
        # the AI server will reply with avatarExit.
    
    def rejectInteract(self):
        """Server doesn't let the avatar interact with prop"""
        assert(self.debugPrint("rejectInteract()"))
        self.cr.playGame.getPlace().setState('walk')
        
    def avatarExit(self, avatarId):
        assert(self.debugPrint("avatarExit(avatarId=%s)"%(avatarId,)))
    
    ##### off state #####
    
    def enterOff(self):
        assert(self.debugPrint("enterOff()"))
    
    def exitOff(self):
        assert(self.debugPrint("exitOff()"))
    
    ##### attract state #####
    
    def enterAttract(self, ts):
        assert(self.debugPrint("enterAttract()"))
    
    def exitAttract(self):
        assert(self.debugPrint("exitAttract()"))
    
    ##### playing state #####
    
    def enterPlaying(self, ts):
        assert(self.debugPrint("enterPlaying()"))
    
    def exitPlaying(self):
        assert(self.debugPrint("exitPlaying()"))
    
    if __debug__:
        def debugPrint(self, message):
            """for debugging"""
            return self.notify.debug(
                    str(self.__dict__.get('entId', '?'))+' '+message)
