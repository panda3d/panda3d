"""ClientDistClass module: contains the ClientDistClass class"""

from PandaModules import *
import DirectNotifyGlobal

class ClientDistClass:
	
    def __init__(self, dcClass):
        self.number = dcClass.get_number()
        self.name = dcClass.get_name()
        self.atomicFields=[]
        self.molecularFields=[]
        self.parseAtomicFields(dcClass)
        self.parseMolecularFields(dcClass)
        return None

    def parseAtomicFields(dcClass):
        for i in range(0,dcClass.get_num_inherited_atomics()):
            self.atomicFields.append((dcClass.get_inherited_atomic(i))
            
        
