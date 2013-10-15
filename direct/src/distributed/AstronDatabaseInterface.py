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

    def queryObject(self, databaseId, doId, callback):
        """
        Query object `doId` out of the database.

        On success, the callback will be invoked as callback(dclass, fields)
        where dclass is a DCClass instance and fields is a dict.
        On failure, the callback will be invoked as callback(None, None).
        """

        # Save the callback:
        ctx = self.air.contextAllocator.allocate()
        self._callbacks[ctx] = callback

        # Generate and send the datagram:
        dg = PyDatagram()
        dg.addServerHeader(databaseId, self.air.ourChannel, DBSERVER_OBJECT_GET_ALL)
        dg.addUint32(ctx)
        dg.addUint32(doId)
        self.air.send(dg)

    def handleQueryObjectResp(self, di):
        ctx = di.getUint32()
        success = di.getUint8()

        if ctx not in self._callbacks:
            self.notify.warning('Received unexpected DBSERVER_OBJECT_GET_ALL_RESP'
                                ' (ctx %d, doId %d)' % (ctx, doId))
            return

        try:
            if not success:
                self._callbacks[ctx](None, None)
                return

            dclassId = di.getUint16()
            dclass = self.air.dclassesByNumber.get(dclassId)

            if not dclass:
                self.notify.warning('Received bad dclass %d for %d in'
                                    ' DBSERVER_OBJECT_GET_ALL_RESP' % (dclassId, doId))
                self._callbacks[ctx](None, None)
                return

            fieldCount = di.getUint16()
            unpacker = DCPacker()
            unpacker.setUnpackData(di.getRemainingBytes())
            fields = {}
            for x in xrange(fieldCount):
                fieldId = unpacker.rawUnpackInt16()
                field = dclass.getFieldByIndex(fieldId)

                if not field:
                    self.notify.warning('Received bad field %d in query for'
                                        ' %s(%d)' % (fieldId, dclass.getName(),
                                                        doId))
                    self._callbacks[ctx](None, None)
                    return


                unpacker.beginUnpack(field)
                fields[field.getName()] = field.unpackArgs(unpacker)
                unpacker.endUnpack()

            self._callbacks[ctx](dclass, fields)

        finally:
            del self._callbacks[ctx]
            self.air.contextAllocator.free(ctx)

    def handleDatagram(self, msgType, di):
        if msgType == DBSERVER_OBJECT_CREATE_RESP:
            self.handleCreateObjectResp(di)
        elif msgType == DBSERVER_OBJECT_GET_ALL_RESP:
            self.handleQueryObjectResp(di)
