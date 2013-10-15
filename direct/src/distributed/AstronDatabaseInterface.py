from pandac.PandaModules import *
from MsgTypes import *
from direct.directnotify import DirectNotifyGlobal
from ConnectionRepository import ConnectionRepository
from PyDatagram import PyDatagram
from PyDatagramIterator import PyDatagramIterator

class AstronDatabaseInterface:
    """
    This class is part of Panda3D's new MMO networking framework.
    It interfaces with Astron database(s) to manage objects directly, rather than
    via DB-StateServers.

    Do not create this class directly; instead, use AstronInternalRepository's
    dbInterface attribute.
    """
    notify = DirectNotifyGlobal.directNotify.newCategory("AstronDatabaseInterface")

    def __init__(self, air):
        self.air = air

        self._callbacks = {}

    def createObject(self, databaseId, dclass, fields={}, callback=None):
        """
        Create an object in the specified database.

        databaseId specifies the control channel of the target database.
        dclass specifies the class of the object to be created.
        fields is a dict with any fields that should be stored in the object on creation.
        callback will be called with callback(doId) if specified. On failure, doId is 0.
        """

        # Save the callback:
        ctx = self.air.contextAllocator.allocate()
        self._callbacks[ctx] = callback

        # Pack up/count valid fields.
        fieldPacker = DCPacker()
        fieldCount = 0
        for k,v in fields.items():
            field = dclass.getFieldByName(k)
            if not field:
                self.notify.warning('Creation request for %s object contains '
                                    'invalid field named %s' % (dclass.getName(), k))
                continue

            fieldPacker.rawPackUint16(field.getNumber())
            fieldPacker.beginPack(field)
            field.packArgs(fieldPacker, v)
            fieldPacker.endPack()
            fieldCount += 1

        # Now generate and send the datagram:
        dg = PyDatagram()
        dg.addServerHeader(databaseId, self.air.ourChannel, DBSERVER_OBJECT_CREATE)
        dg.addUint32(ctx)
        dg.addUint16(dclass.getNumber())
        dg.addUint16(fieldCount)
        dg.appendData(fieldPacker.getString())
        self.air.send(dg)

    def handleCreateObjectResp(self, di):
        ctx = di.getUint32()
        doId = di.getUint32()

        if ctx not in self._callbacks:
            self.notify.warning('Received unexpected DBSERVER_OBJECT_CREATE_RESP'
                                ' (ctx %d, doId %d)' % (ctx, doId))
            return

        self._callbacks[ctx](doId)

        del self._callbacks[ctx]
        self.air.contextAllocator.free(ctx)

    def handleDatagram(self, msgType, di):
        if msgType == DBSERVER_OBJECT_CREATE_RESP:
            self.handleCreateObjectResp(di)
