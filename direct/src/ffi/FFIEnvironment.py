import FFIConstants

class FFIEnvironment:
    def __init__(self):
        self.reset()

    def reset(self):
        self.types = {}
        self.globalFunctions = []
        self.downcastFunctions = []
        self.globalValues = []
        self.manifests = []
    
    def addType(self, typeDescriptor, name):
        if self.types.has_key(name):
            FFIConstants.notify.info('Redefining type named: ' + name)
        self.types[name] = typeDescriptor
    
    def getTypeNamed(self, name):
        try:
            self.types[name]
        except KeyError:
            raise 'Type not found in FFIEnvironment'
    
    def addGlobalFunction(self, typeDescriptors):
        self.globalFunctions.extend(typeDescriptors)
    def addDowncastFunction(self, typeDescriptor):
        self.downcastFunctions.append(typeDescriptor)
    def addGlobalValue(self, typeDescriptor):
        self.globalValues.append(typeDescriptor)
    def addManifest(self, typeDescriptor):
        self.manifests.append(typeDescriptor)
