"""DistributedLargeBlobSenderAI module: contains the DistributedLargeBlobSenderAI class"""

import DistributedObjectAI
import DirectNotifyGlobal
import LargeBlobSenderConsts

class DistributedLargeBlobSenderAI(DistributedObjectAI.DistributedObjectAI):
    """DistributedLargeBlobSenderAI: for sending large chunks of data through
    the DC system to a specific avatar"""
    notify = DirectNotifyGlobal.directNotify.newCategory('DistributedLargeBlobSenderAI')

    def __init__(self, air, zoneId, targetAvId, data, useDisk=0):
        DistributedObjectAI.DistributedObjectAI.__init__(self, air)
        self.targetAvId = targetAvId

        self.mode = 0
        if useDisk:
            self.mode |= LargeBlobSenderConsts.USE_DISK

        self.generateWithRequired(zoneId)

        # send the data
        if useDisk:
            DistributedLargeBlobSenderAI.notify.error(
                'large blob transfer by file not yet implemented')
        else:
            s = str(data)
            chunkSize = LargeBlobSenderConsts.ChunkSize
            while len(s):
                self.sendUpdateToAvatarId(self.targetAvId,
                                          'setChunk', [s[:chunkSize]])
                s = s[chunkSize:]
            # send final empty string
            self.sendUpdateToAvatarId(self.targetAvId, 'setChunk', [''])

    def getMode(self):
        return self.mode
        
    def getTargetAvId(self):
        return self.targetAvId

    def setAck(self):
        DistributedLargeBlobSenderAI.notify.debug('setAck')
        self.requestDelete()
