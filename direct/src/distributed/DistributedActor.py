"""DistributedActor module: contains the DistributedActor class"""

import DistributedNode
import Actor

class DistributedActor(DistributedNode.DistributedNode, Actor.Actor):
    """Distributed Actor class:"""

    def __init__(self):
        pass

    def generateInit(self, di):
        DistributedNode.DistributedNode.generateInit(self, di)
        
