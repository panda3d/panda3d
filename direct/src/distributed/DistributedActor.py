"""DistributedActor module: contains the DistributedActor class"""

import DistributedNode
import Actor

class DistributedActor(DistributedNode.DistributedNode, Actor.Actor):
    """Distributed Actor class:"""

    def __init__(self, cr):
        try:
            self.DistributedActor_initialized
        except:
            self.DistributedActor_initialized = 1
            DistributedNode.DistributedNode.__init__(self, cr)
        return None

        
