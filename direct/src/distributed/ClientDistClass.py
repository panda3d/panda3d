"""ClientDistClass module: contains the ClientDistClass class"""

from PandaModules import *
import DirectNotifyGlobal
import ClientDistUpdate

class ClientDistClass:
	
    def __init__(self, dcClass):
        self.number = dcClass.getNumber()
        self.name = dcClass.getName()
        self.allFields = self.parseFields(dcClass)
        self.allCDU = self.createAllCDU(self.allFields)
        self.number2CDU = self.createNumber2CDUDict(self.allCDU)
        self.name2CDU = self.createName2CDUDict(self.allCDU)
        return None

    def parseFields(self, dcClass):
        fields=[]
        for i in range(0,dcClass.getNumInheritedFields()):
            fields.append(dcClass.getInheritedField(i))
        return fields

    def createAllCDU(self, allFields):
        allCDU = []
        for i in allFields:
            allCDU.append(ClientDistUpdate.ClientDistUpdate(i))
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

    def updateField(self, do, di):
        # Get the update field id
        fieldId = di.getArg(ST_uint8)
        # look up the CDU
        assert(self.number2CDU.has_key(fieldId))
        cdu = self.number2CDU[fieldId]
        # Let the cdu finish the job
        cdu.updateField(cdc, do, di)
        return None

    def sendUpdate(self, do, fieldName, args):
        # Look up the cdu
        assert(self.name2CDU.has_key(fieldName))
        cdu = self.name2CDU[fieldName]
        # Let the cdu finish the job
        cdu.sendUpdate(do, args)
        

    
            
        
