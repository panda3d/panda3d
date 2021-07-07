from direct.distributed.DistributedSmoothNode import DistributedSmoothNode
from panda3d.core import NodePath

from direct.actor.Actor import Actor

class DistributedSmoothActor(DistributedSmoothNode, Actor):
    def __init__(self, cr):
        Actor.__init__(self, "models/ralph",
            {"run": "models/ralph-run",
            "walk": "models/ralph-walk"})
        DistributedSmoothNode.__init__(self, cr)
        self.setCacheable(1)
        self.setScale(.2)

    def generate(self):
        DistributedSmoothNode.generate(self)
        self.activateSmoothing(True, False)
        self.startSmooth()

    def announceGenerate(self):
        DistributedSmoothNode.announceGenerate(self)
        self.reparentTo(render)

    def disable(self):
        # remove all anims, on all parts and all lods
        self.stopSmooth()
        if (not self.isEmpty()):
            Actor.unloadAnims(self, None, None, None)
        DistributedSmoothNode.disable(self)

    def delete(self):
        try:
            self.DistributedActor_deleted
        except:
            self.DistributedActor_deleted = 1
            DistributedSmoothNode.delete(self)
            Actor.delete(self)

    def start(self):
        # Let the DistributedSmoothNode take care of broadcasting the
        # position updates several times a second.
        self.startPosHprBroadcast()

    def loop(self, animName):
        self.sendUpdate("loop", [animName])
        return Actor.loop(self, animName)

    def pose(self, animName, frame):
        self.sendUpdate("pose", [animName, frame])
        return Actor.pose(self, animName, frame)
