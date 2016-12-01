from direct.distributed.ClockDelta import *
from panda3d.core import *
from direct.distributed import DistributedObjectAI

class TimeManagerAI(DistributedObjectAI.DistributedObjectAI):
    notify = DirectNotifyGlobal.directNotify.newCategory("TimeManagerAI")

    def __init__(self, air):
        DistributedObjectAI.DistributedObjectAI.__init__(self, air)

    def requestServerTime(self, context):
        """requestServerTime(self, int8 context)

        This message is sent from the client to the AI to initiate a
        synchronization phase.  The AI should immediately report back
        with its current time.  The client will then measure the round
        trip.
        """
        timestamp = globalClockDelta.getRealNetworkTime(bits=32)
        requesterId = self.air.getAvatarIdFromSender()
        print("requestServerTime from %s" % (requesterId))
        self.sendUpdateToAvatarId(requesterId, "serverTime",
                                  [context, timestamp])
