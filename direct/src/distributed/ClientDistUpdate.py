"""ClientDistUpdate module: contains the ClientDistUpdate class"""

import DirectNotifyGlobal
from PyDatagram import PyDatagram
from MsgTypes import *
import ihooks

# These are stored here so that the distributed classes we load on the fly
# can be exec'ed in the module namespace as if we imported them normally.
# This is important for redefine to work, and is a good idea anyways.
moduleGlobals = globals()
moduleLocals = locals()

class ClientDistUpdate:
    notify = DirectNotifyGlobal.directNotify.newCategory("ClientDistUpdate")

    def __init__(self, cdc, dcField):
        self.cdc = cdc
        self.field = dcField
        self.number = dcField.getNumber()
        self.name = dcField.getName()
        self.types = []
        self.divisors = []
        self.deriveTypesFromParticle(dcField)

        stuff = ihooks.current_importer.get_loader().find_module(cdc.name)
        if not stuff:
            # This will be printed by ClientDistClass
            # self.notify.warning("Unable to import %s.py" % (cdc.name))
            self.func = None
            return
            
        module = __import__(cdc.name, moduleGlobals, moduleLocals)
        # If there is no class here, that is an error
        classObj = getattr(module, cdc.name)
        # If there is no func, it will just be None
        self.func = getattr(classObj, self.name, None)

    def deriveTypesFromParticle(self, dcField):
        dcFieldAtomic = dcField.asAtomicField()
        dcFieldMolecular = dcField.asMolecularField()
        if dcFieldAtomic:
            for i in range(0, dcFieldAtomic.getNumElements()):
                self.types.append(dcFieldAtomic.getElementType(i))
                self.divisors.append(dcFieldAtomic.getElementDivisor(i))
        elif dcFieldMolecular:
            for i in range(0, dcFieldMolecular.getNumAtomics()):
                componentField = dcFieldMolecular.getAtomic(i)
                for j in range(0, componentField.getNumElements()):
                    self.types.append(componentField.getElementType(j))
                    self.divisors.append(componentField.getElementDivisor(j))
        else:
            self.notify.error("field is neither atom nor molecule")

    def updateField(self, cdc, do, di):
        # Get the arguments into a list
        args = map(lambda type, div: di.getArg(type,div), self.types, self.divisors)
        assert(self.notify.debug("Received update for %d: %s.%s(%s)" % (do.doId, cdc.name, self.name, args)))
        # Apply the function to the object with the arguments
        if self.func != None:
            apply(self.func, [do] + args)

    def sendUpdate(self, cr, do, args, sendToId = None):
        if sendToId == None:
            sendToId = do.doId
        assert(self.notify.debug("Sending update for %d: %s(%s)" % (sendToId, self.name, args)))
        datagram = PyDatagram()
        # Add message type
        datagram.addUint16(CLIENT_OBJECT_UPDATE_FIELD)
        # Add the DO id
        datagram.addUint32(sendToId)
        # Add the field id
        datagram.addUint16(self.number)
        # Add the arguments
        for arg, type, div in zip(args, self.types, self.divisors):
            datagram.putArg(arg, type, div)
        # send the datagram
        cr.send(datagram)
