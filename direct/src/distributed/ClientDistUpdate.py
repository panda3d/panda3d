"""ClientDistUpdate module: contains the ClientDistUpdate class"""

import DirectNotifyGlobal
import Datagram
from MsgTypes import *

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
        self.compress = 0
        self.deriveTypesFromParticle(dcField)
        # Figure out our function pointer
        exec("import " + cdc.name, moduleGlobals, moduleLocals)
        try:
            self.func = eval(cdc.name + "." + cdc.name + "." + self.name)
        # Only catch name and attribute errors
        # as all other errors are legit errors
        except (NameError, AttributeError), e:
            #ClientDistUpdate.notify.warning(cdc.name + "." + self.name +
            #                                " does not exist")
            self.func = None
        return None

    def deriveTypesFromParticle(self, dcField):
        dcFieldAtomic = dcField.asAtomicField()
        dcFieldMolecular = dcField.asMolecularField()
        if dcFieldAtomic:
            for i in range(0, dcFieldAtomic.getNumElements()):
                self.types.append(dcFieldAtomic.getElementType(i))
                self.divisors.append(dcFieldAtomic.getElementDivisor(i))
            if dcFieldAtomic.isCompress():
                self.compress = 1
                
        elif dcFieldMolecular:
            for i in range(0, dcFieldMolecular.getNumAtomics()):
                componentField = dcFieldMolecular.getAtomic(i)
                for j in range(0, componentField.getNumElements()):
                    self.types.append(componentField.getElementType(j))
                    self.divisors.append(componentField.getElementDivisor(j))
                if componentField.isCompress():
                    self.compress = 1
        else:
            ClientDistUpdate.notify.error("field is neither atom nor molecule")
        return None

    def updateField(self, cdc, do, di, allowCompress):
        # Get the arguments into a list
        args = self.extractArgs(di, allowCompress)

        assert(self.notify.debug("Received update for %d: %s.%s(%s)" % (do.doId, cdc.name, self.name, args)))

        # Apply the function to the object with the arguments
        if self.func != None:
            apply(self.func, [do] + args)
        return None

    def extractArgs(self, di, allowCompress):
        if allowCompress and self.compress:
            # Technically, the datagram pointer is const, so this is
            # invalid.  Ignore that for now.
            argsAt = di.getCurrentIndex()
            di.getDatagram().uncompress(argsAt)

        args = []
        assert(len(self.types) == len(self.divisors))
        numTypes = len(self.types)
        for i in range(numTypes):
            args.append(di.getArg(self.types[i], self.divisors[i]))
        return args

    def addArgs(self, datagram, args):
        # Add the args to the datagram
        numElems = len(args)
        assert (numElems == len(self.types) == len(self.divisors))
        for i in range(0, numElems):
            datagram.putArg(args[i], self.types[i], self.divisors[i])
    
    def sendUpdate(self, cr, do, args, sendToId = None):
        if sendToId == None:
            sendToId = do.doId

        assert(self.notify.debug("Sending update for %d: %s(%s)" % (sendToId, self.name, args)))
        datagram = Datagram.Datagram()
        # Add message type
        datagram.addUint16(CLIENT_OBJECT_UPDATE_FIELD)
        # Add the DO id
        datagram.addUint32(sendToId)
        # Add the field id
        datagram.addUint16(self.number)
        argsAt = datagram.getLength()
        # Add the arguments
        self.addArgs(datagram, args)
        if self.compress:
            datagram.compress(argsAt)
        # send the datagram
        cr.send(datagram)
