"""OrLoEntity.py: contains the OrLoEntity class"""

class OrLoEntity(Entity.Entity):
    if __debug__:
        notify = DirectNotifyGlobal.directNotify.newCategory(
                'OrLoEntity')

    def __init__(self, air, levelDoId, entId, zoneId=None):
        """entId: """
        assert(self.debugPrint(
                "DistributedDoorEntityAI(air=%s, levelDoId=%s, entId=%s, zoneId=%s)"
                %("the air", levelDoId, entId, zoneId)))
        self.doLaterTask=None
        self.isOpenInput = None
        DistributedInteractiveEntityAI.DistributedInteractiveEntityAI.__init__(
                self, air, levelDoId, entId)
        self.fsm.setName('DistributedDoorEntity')
        if zoneId is not None:
            self.generateWithRequired(zoneId)
