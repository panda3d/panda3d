import FFIConstants

class FFIEnvironment:
    def __init__(self):
        self.types = {}
        self.globalFunctions = []
        self.globalValues = []
    
    def addType(self, typeDescriptor, name):
        if self.types.has_key(name):
            FFIConstants.notify.warning('Redefining type named: ' + name)
        self.types[name] = typeDescriptor
    
    def getTypeNamed(self, name):
        try:
            self.types[name]
        except KeyError:
            raise 'Type not found in FFIEnvironment'
    
    def addGlobalFunction(self, typeDescriptor):
        self.globalFunctions.append(typeDescriptor)
    def addGlobalValue(self, typeDescriptor):
        self.globalValues.append(typeDescriptor)
