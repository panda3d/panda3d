"""ClientDistClass module: contains the ClientDistClass class"""

from PandaModules import *
import DirectNotifyGlobal
import ClientDistUpdate
import sys
import ihooks

# These are stored here so that the distributed classes we load on the fly
# can be exec'ed in the module namespace as if we imported them normally.
# This is important for redefine to work, and is a good idea anyways.
moduleGlobals = globals()
moduleLocals = locals()

class ClientDistClass:
    notify = DirectNotifyGlobal.directNotify.newCategory("ClientDistClass")

    def __init__(self, dcClass):
        self.number = dcClass.getNumber()
        self.name = dcClass.getName()

        # Import the class, and store the constructor
        try:
             exec("import " + self.name, moduleGlobals, moduleLocals)
        except ImportError, e:
            self.notify.warning("Unable to import %s.py: %s" % (self.name, e))
            self.constructor = None
            return
        self.constructor = eval(self.name + "." + self.name)

        """
        # This does not seem to work on the client publish
        stuff = ihooks.current_importer.get_loader().find_module(self.name)
        if not stuff:
            self.notify.warning("Unable to import %s.py" % (self.name))
            self.constructor = None
            return
        module = __import__(self.name, moduleGlobals, moduleLocals)
        # The constructor is really the classObj, which is of course callable
        self.constructor = getattr(module, self.name, None)
        """

        self.allFields = self.parseFields(dcClass)
        self.allCDU = self.createAllCDU(self.allFields, self.constructor)
        self.number2CDU = self.createNumber2CDUDict(self.allCDU)
        self.name2CDU = self.createName2CDUDict(self.allCDU)
        self.broadcastRequiredCDU = self.listBroadcastRequiredCDU(self.allCDU)
        self.allRequiredCDU = self.listRequiredCDU(self.allCDU)

        # If this assertion fails, you probably had an import error in
        # a file named in your toon.dc file, or in some file included
        # in a file named in your toon.dc file.
        assert(self.constructor != None)

    def parseFields(self, dcClass):
        fields=[]
        for i in range(0,dcClass.getNumInheritedFields()):
            fields.append(dcClass.getInheritedField(i))
        return fields

    def createAllCDU(self, allFields, classObj):
        allCDU = []
        for i in allFields:
            allCDU.append(ClientDistUpdate.ClientDistUpdate(self, i, classObj))
        return allCDU

    def createNumber2CDUDict(self, allCDU):
        dict={}
        for i in allCDU:
            dict[i.number] = i
        return dict

    def createName2CDUDict(self, allCDU):
        dict={}
        for i in allCDU:
            dict[i.name] = i
        return dict

    def listBroadcastRequiredCDU(self, allCDU):
        requiredCDU = []
        for i in allCDU:
            atom = i.field.asAtomicField()
            if atom:
                if (atom.isRequired() and atom.isBroadcast()):
                    requiredCDU.append(i)
        return requiredCDU

    def listRequiredCDU(self, allCDU):
        requiredCDU = []
        for i in allCDU:
            atom = i.field.asAtomicField()
            if atom:
                if (atom.isRequired()):
                    requiredCDU.append(i)
        return requiredCDU

    def updateField(self, do, di):
        # Get the update field id
        fieldId = di.getArg(STUint16)
        # look up the CDU
        assert(self.number2CDU.has_key(fieldId))
        cdu = self.number2CDU[fieldId]
        # Let the cdu finish the job
        cdu.updateField(self, do, di)
        return None

    def sendUpdate(self, cr, do, fieldName, args, sendToId = None):
        # Look up the cdu
        assert(self.name2CDU.has_key(fieldName))
        cdu = self.name2CDU[fieldName]
        # Let the cdu finish the job
        cdu.sendUpdate(cr, do, args, sendToId)
        

    
            
        
