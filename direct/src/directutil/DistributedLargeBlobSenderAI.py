"""DistributedLargeBlobSenderAI module: contains the DistributedLargeBlobSenderAI class"""

from direct.distributed import DistributedObjectAI
from direct.directnotify import DirectNotifyGlobal
from . import LargeBlobSenderConsts

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
        s = str(data)
        if useDisk:
            # write the data to a file and tell the client where to get it
            import os
            import random
            origDir = os.getcwd()
            bPath = LargeBlobSenderConsts.getLargeBlobPath()
            try:
                os.chdir(bPath)
            except OSError:
                DistributedLargeBlobSenderAI.notify.error(
                    'could not access %s' % bPath)
            # find an unused temp filename
            while 1:
                num = random.randrange((1 << 30)-1)
                filename = LargeBlobSenderConsts.FilePattern % num
                try:
                    os.stat(filename)
                except OSError:
                    break
            # NOTE: there's a small chance of a race condition here, if
            # the file is created by another AI just after the stat fails
            f = open(filename, 'wb')
            f.write(s)
            f.close()
            os.chdir(origDir)
            self.sendUpdateToAvatarId(self.targetAvId,
                                      'setFilename', [filename])
        else:
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
        assert self.air.getAvatarIdFromSender() == self.targetAvId
        self.requestDelete()
