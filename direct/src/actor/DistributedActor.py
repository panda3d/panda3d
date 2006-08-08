"""DistributedActor module: contains the DistributedActor class"""

__all__ = ['DistributedActor']

from direct.distributed import DistributedNode

import Actor

class DistributedActor(DistributedNode.DistributedNode, Actor.Actor):
    def __init__(self, cr):
        try:
            self.DistributedActor_initialized
        except:
            self.DistributedActor_initialized = 1   
            Actor.Actor.__init__(self)     
            DistributedNode.DistributedNode.__init__(self, cr)
            # Since actors are probably fairly heavyweight, we'd
            # rather cache them than delete them if possible.       
            self.setCacheable(1)

    def disable(self):
        # remove all anims, on all parts and all lods
        if (not self.isEmpty()):
            Actor.Actor.unloadAnims(self, None, None, None)
        DistributedNode.DistributedNode.disable(self)

    def delete(self):
        try:
            self.DistributedActor_deleted
        except:
            self.DistributedActor_deleted = 1       
            DistributedNode.DistributedNode.delete(self)
            Actor.Actor.delete(self)


    def loop(self, animName, restart=1, partName=None, fromFrame=None, toFrame=None):       
        return Actor.Actor.loop(self, animName, restart, partName, fromFrame, toFrame)
