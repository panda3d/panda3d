"""ClientDistUpdate module: contains the ClientDistUpdate class"""

import DirectNotifyGlobal
import Avatar
import DistributedToon
import Datagram
from MsgTypes import *

class ClientDistUpdate:
    notify = DirectNotifyGlobal.directNotify.newCategory("ClientDistUpdate")

    def __init__(self, dcField):
        self.field = dcField
        self.number = dcField.getNumber()
        self.name = dcField.getName()
        self.types = []
        self.divisors = []
        self.deriveTypesFromParticle(dcField)        
        return None

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
            ClientDistUpdate.notify.error("field is neither atom nor molecule")
        return None

    def updateField(self, cdc, do, di):
        # Look up the class
        #aClass = eval(cdc.name + "." + cdc.name)
        # Look up the function
        #assert(aClass.__dict__.has_key(self.name))
        #func = aClass.__dict__[self.name]
        func = eval(cdc.name + "." + cdc.name + "." + self.name)
        # Get the arguments into a list
        args = self.extractArgs(di)
        # Apply the function to the object with the arguments
        apply(func, [do] + args)
        return None

    def extractArgs(self, di):
        args = []
        for i in self.types:
            args.append(di.getArg(i))
        return args

    def addArgs(self, datagram, args):
        # Add the args to the datagram
        numElems = len(args)
        assert (numElems == len(self.types))
        for i in range(0, numElems):
            datagram.putArg(args[i], self.types[i])
    
    def sendUpdate(self, cr, do, args):
        datagram = Datagram.Datagram()
        # Add message type
        datagram.addUint16(CLIENT_OBJECT_UPDATE_FIELD)
        # Add the DO id
        datagram.addUint32(do.doId)
        # Add the field id
        datagram.addUint16(self.number)
        # Add the arguments
        self.addArgs(datagram, args)
        # send the datagram
        cr.send(datagram)
