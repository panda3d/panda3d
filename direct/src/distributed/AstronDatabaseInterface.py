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
        self._dclasses = {}

    def createObject(self, databaseId, dclass, fields={}, callback=None):
        """
        Create an object in the specified database.

        databaseId specifies the control channel of the target database.
        dclass specifies the class of the object to be created.
        fields is a dict with any fields that should be stored in the object on creation.
        callback will be called with callback(doId) if specified. On failure, doId is 0.
        """

        # Save the callback:
        ctx = self.air.getContext()
        self._callbacks[ctx] = callback

        # Pack up/count valid fields.
        fieldPacker = DCPacker()
        fieldCount = 0
        for k,v in fields.items():
            field = dclass.getFieldByName(k)
            if not field:
                self.notify.error('Creation request for %s object contains '
                                  'invalid field named %s' % (dclass.getName(), k))

            fieldPacker.rawPackUint16(field.getNumber())
            fieldPacker.beginPack(field)
            field.packArgs(fieldPacker, v)
            fieldPacker.endPack()
            fieldCount += 1

        # Now generate and send the datagram:
        dg = PyDatagram()
        dg.addServerHeader(databaseId, self.air.ourChannel, DBSERVER_CREATE_OBJECT)
        dg.addUint32(ctx)
        dg.addUint16(dclass.getNumber())
        dg.addUint16(fieldCount)
        dg.appendData(fieldPacker.getString())
        self.air.send(dg)

    def handleCreateObjectResp(self, di):
        ctx = di.getUint32()
        doId = di.getUint32()

        if ctx not in self._callbacks:
            self.notify.warning('Received unexpected DBSERVER_CREATE_OBJECT_RESP'
                                ' (ctx %d, doId %d)' % (ctx, doId))
            return

        if self._callbacks[ctx]:
            self._callbacks[ctx](doId)

        del self._callbacks[ctx]

    def queryObject(self, databaseId, doId, callback, dclass=None, fieldNames=()):
        """
        Query object `doId` out of the database.

        On success, the callback will be invoked as callback(dclass, fields)
        where dclass is a DCClass instance and fields is a dict.
        On failure, the callback will be invoked as callback(None, None).
        """

        # Save the callback:
        ctx = self.air.getContext()
        self._callbacks[ctx] = callback
        self._dclasses[ctx] = dclass

        # Generate and send the datagram:
        dg = PyDatagram()

        if not fieldNames:
            dg.addServerHeader(databaseId, self.air.ourChannel,
                               DBSERVER_OBJECT_GET_ALL)
        else:
            # We need a dclass in order to convert the field names into field IDs:
            assert dclass is not None

            if len(fieldNames) > 1:
                dg.addServerHeader(databaseId, self.air.ourChannel,
                                   DBSERVER_OBJECT_GET_FIELDS)
            else:
                dg.addServerHeader(databaseId, self.air.ourChannel,
                                   DBSERVER_OBJECT_GET_FIELD)

        dg.addUint32(ctx)
        dg.addUint32(doId)
        if len(fieldNames) > 1:
            dg.addUint16(len(fieldNames))
        for fieldName in fieldNames:
            field = dclass.getFieldByName(fieldName)
            if field is None:
                self.notify.error('Bad field named %s in query for'
                                  ' %s object' % (fieldName, dclass.getName()))
            dg.addUint16(field.getNumber())
        self.air.send(dg)

    def handleQueryObjectResp(self, msgType, di):
        ctx = di.getUint32()
        success = di.getUint8()

        if ctx not in self._callbacks:
            self.notify.warning('Received unexpected %s'
                                ' (ctx %d)' % (MsgId2Names[msgType], ctx))
            return

        try:
            if not success:
                if self._callbacks[ctx]:
                    self._callbacks[ctx](None, None)
                return

            if msgType == DBSERVER_OBJECT_GET_ALL_RESP:
                dclassId = di.getUint16()
                dclass = self.air.dclassesByNumber.get(dclassId)
            else:
                dclass = self._dclasses[ctx]

            if not dclass:
                self.notify.error('Received bad dclass %d in'
                                  ' DBSERVER_OBJECT_GET_ALL_RESP' % (dclassId))

            if msgType == DBSERVER_OBJECT_GET_FIELD_RESP:
                fieldCount = 1
            else:
                fieldCount = di.getUint16()
            unpacker = DCPacker()
            unpacker.setUnpackData(di.getRemainingBytes())
            fields = {}
            for x in xrange(fieldCount):
                fieldId = unpacker.rawUnpackInt16()
                field = dclass.getFieldByIndex(fieldId)

                if not field:
                    self.notify.error('Received bad field %d in query for'
                                      ' %s object' % (fieldId, dclass.getName()))

                unpacker.beginUnpack(field)
                fields[field.getName()] = field.unpackArgs(unpacker)
                unpacker.endUnpack()

            if self._callbacks[ctx]:
                self._callbacks[ctx](dclass, fields)

        finally:
            del self._callbacks[ctx]
            del self._dclasses[ctx]

    def updateObject(self, databaseId, doId, dclass, newFields, oldFields=None, callback=None):
        """
        Update field(s) on an object, optionally with the requirement that the
        fields must match some old value.

        databaseId and doId represent the database control channel and object ID
        for the update request.
        newFields is to be a dict of fieldname->value, representing the fields
        to add/change on the database object.
        oldFields, if specified, is a similarly-formatted dict that contains the
        expected older values. If the values do not match, the database will
        refuse to process the update. This is useful for guarding against race
        conditions.

        On success, the callback is called as callback(None).
        On failure, the callback is called as callback(dict), where dict contains
        the current object values. This is so that the updater can try again,
        basing its updates off of the new values.
        """

        # Ensure that the keys in newFields and oldFields are the same if
        # oldFields is given...
        if oldFields is not None:
            if set(newFields.keys()) != set(oldFields.keys()):
                self.notify.error('newFields and oldFields must contain the same keys!')
                return

        fieldPacker = DCPacker()
        fieldCount = 0
        for k,v in newFields.items():
            field = dclass.getFieldByName(k)
            if not field:
                self.notify.error('Update for %s(%d) object contains invalid'
                                  ' field named %s' % (dclass.getName(), doId, k))

            fieldPacker.rawPackUint16(field.getNumber())

            if oldFields is not None:
                # Pack the old values:
                fieldPacker.beginPack(field)
                field.packArgs(fieldPacker, oldFields[k])
                fieldPacker.endPack()

            fieldPacker.beginPack(field)
            field.packArgs(fieldPacker, v)
            fieldPacker.endPack()

            fieldCount += 1

        # Generate and send the datagram:
        dg = PyDatagram()
        if oldFields is not None:
            ctx = self.air.getContext()
            self._callbacks[ctx] = callback
            if fieldCount == 1:
                dg.addServerHeader(databaseId, self.air.ourChannel,
                                   DBSERVER_OBJECT_SET_FIELD_IF_EQUALS)
            else:
                dg.addServerHeader(databaseId, self.air.ourChannel,
                                   DBSERVER_OBJECT_SET_FIELDS_IF_EQUALS)
            dg.addUint32(ctx)
        else:
            if fieldCount == 1:
                dg.addServerHeader(databaseId, self.air.ourChannel,
                                   DBSERVER_OBJECT_SET_FIELD)
            else:
                dg.addServerHeader(databaseId, self.air.ourChannel,
                                   DBSERVER_OBJECT_SET_FIELDS)
        dg.addUint32(doId)
        if fieldCount != 1:
            dg.addUint16(fieldCount)
        dg.appendData(fieldPacker.getString())
        self.air.send(dg)

        if oldFields is None and callback is not None:
            # Why oh why did they ask for a callback if there's no oldFields?
            # Oh well, better honor their request:
            callback(None)

    def handleUpdateObjectResp(self, di, multi):
        ctx = di.getUint32()
        success = di.getUint8()

        if ctx not in self._callbacks:
            self.notify.warning('Received unexpected'
                                ' DBSERVER_OBJECT_SET_FIELD(S)_IF_EQUALS_RESP'
                                ' (ctx %d)' % (ctx))
            return

        try:
            if success:
                if self._callbacks[ctx]:
                    self._callbacks[ctx](None)
                return

            if not di.getRemainingSize():
                # We failed due to other reasons.
                if self._callbacks[ctx]:
                    return self._callbacks[ctx]({})

            if multi:
                fieldCount = di.getUint16()
            else:
                fieldCount = 1

            unpacker = DCPacker()
            unpacker.setUnpackData(di.getRemainingBytes())
            fields = {}
            for x in xrange(fieldCount):
                fieldId = unpacker.rawUnpackInt16()
                field = self.air.getDcFile().getFieldByIndex(fieldId)

                if not field:
                    self.notify.error('Received bad field %d in update'
                                      ' failure response message' % (fieldId))

                unpacker.beginUnpack(field)
                fields[field.getName()] = field.unpackArgs(unpacker)
                unpacker.endUnpack()

            if self._callbacks[ctx]:
                self._callbacks[ctx](fields)

        finally:
            del self._callbacks[ctx]

    def handleDatagram(self, msgType, di):
        if msgType == DBSERVER_CREATE_OBJECT_RESP:
            self.handleCreateObjectResp(di)
        elif msgType in (DBSERVER_OBJECT_GET_ALL_RESP,
                         DBSERVER_OBJECT_GET_FIELDS_RESP,
                         DBSERVER_OBJECT_GET_FIELD_RESP):
            self.handleQueryObjectResp(msgType, di)
        elif msgType == DBSERVER_OBJECT_SET_FIELD_IF_EQUALS_RESP:
            self.handleUpdateObjectResp(di, False)
        elif msgType == DBSERVER_OBJECT_SET_FIELDS_IF_EQUALS_RESP:
            self.handleUpdateObjectResp(di, True)
