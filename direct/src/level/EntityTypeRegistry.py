"""EntityTypeRegistry module: contains the EntityTypeRegistry class"""

from pandac.PandaModules import *
from direct.directnotify import DirectNotifyGlobal
import types
import AttribDesc
import EntityTypeDesc
from direct.showbase.PythonUtil import mostDerivedLast
import os

class EntityTypeRegistry:
    notify = DirectNotifyGlobal.directNotify.newCategory('EntityTypeRegistry')

    def __init__(self, entityTypeModule):
        """pass in a module that contains EntityTypeDesc classes"""
        self.entTypeModule = entityTypeModule

        # compute the hash of the source modules as of the time of creation
        hv = HashVal()
        import EntityTypes
        reload(EntityTypes)
        reload(self.entTypeModule)

        # Convert a pyc or pyo to a py
        # If the client runs genPyCode -n then ihooks will not be installed
        # and you will get a pyc file instead of a py file. Then the AI and
        # client will be mismatched because the AI automatically installs
        # ihooks and thus will hash the py instead of the pyc. Then the
        # hashes do not match and everybody is sad. Let's just make sure
        # for once and for all that we use the .py file and not a .pyc or a
        # .pyo in case that ever happened.
        def getPyExtVersion(filename):
            base, ext = os.path.splitext(filename)
            if (ext == ".pyc") or (ext == ".pyo"):
                filename = base + ".py"
            return filename
        
        hv.hashFile(Filename.fromOsSpecific(getPyExtVersion(EntityTypes.__file__)))
        s = str(hv.asHex())
        s += '.'
        hv.hashFile(Filename.fromOsSpecific(getPyExtVersion(self.entTypeModule.__file__)))
        s += str(hv.asHex())
        self.hashStr = s

        # get a list of the EntityTypeDesc classes in the type module
        classes = []
        for key, value in entityTypeModule.__dict__.items():
            if type(value) is types.ClassType:
                if issubclass(value, EntityTypeDesc.EntityTypeDesc):
                    classes.append(value)

        self.entTypeName2typeDesc = {}

        # create an instance of each EntityType class with a typename
        # make sure that derived classes come after bases
        mostDerivedLast(classes)
        for c in classes:
            if c.__dict__.has_key('type'):
                if self.entTypeName2typeDesc.has_key(c.type):
                    # a more-derived class is replacing a less-derived class
                    # to implement a particular entity type
                    EntityTypeRegistry.notify.info(
                        "replacing %s with %s for entity type '%s'" %
                        (self.entTypeName2typeDesc[c.type].__class__,
                         c, c.type))
                self.entTypeName2typeDesc[c.type] = c()

        # create mapping of entity output types to list of concrete entity
        # typenames with that output type
        self.output2typeNames = {}
        for typename, typeDesc in self.entTypeName2typeDesc.items():
            if typeDesc.isConcrete():
                if hasattr(typeDesc, 'output'):
                    outputType = typeDesc.output
                    self.output2typeNames.setdefault(outputType, [])
                    self.output2typeNames[outputType].append(typename)

        # create list of permanent entity typenames (entity types that cannot
        # be inserted or removed in the editor)
        self.permanentTypeNames = []
        for typename, typeDesc in self.entTypeName2typeDesc.items():
            if typeDesc.isPermanent():
                assert typeDesc.isConcrete()
                self.permanentTypeNames.append(typename)

        # create mapping of entity typename (abstract or concrete) to list
        # of entity typenames are concrete and are of that type or derive
        # from that type
        self.typeName2derivedTypeNames = {}
        for typename, typeDesc in self.entTypeName2typeDesc.items():
            typenames = []
            for tn, td in self.entTypeName2typeDesc.items():
                if td.isConcrete():
                    if issubclass(td.__class__, typeDesc.__class__):
                        typenames.append(tn)
            self.typeName2derivedTypeNames[typename] = typenames

    def getTypeDesc(self, entTypeName):
        """returns EntityTypeDesc instance for concrete Entity type"""
        assert entTypeName in self.entTypeName2typeDesc,\
               "unknown entity type '%s'" % entTypeName
        # the table has descriptors for abstract entity types, but I don't
        # think there's any need for anyone outside this class to access them
        assert self.entTypeName2typeDesc[entTypeName].isConcrete(),\
               "entity type '%s' is abstract" % entTypeName
        return self.entTypeName2typeDesc[entTypeName]

    def getTypeNamesFromOutputType(self, outputType):
        """return Entity typenames for Entity types with particular output"""
        return self.output2typeNames.get(outputType, [])

    def getDerivedTypeNames(self, entTypeName):
        """return Entity typenames that are of or derive from an entity type,
        which may be concrete or abstract"""
        assert entTypeName in self.typeName2derivedTypeNames,\
               "unknown entity type '%s'" % entTypeName
        return self.typeName2derivedTypeNames[entTypeName]

    def isDerivedAndBase(self, entType, baseEntType):
        return entType in self.getDerivedTypeNames(baseEntType)

    def getPermanentTypeNames(self):
        return self.permanentTypeNames

    def getHashStr(self):
        return self.hashStr

    def __hash__(self):
        # THIS IS NOT GUARANTEED TO PRODUCE THE SAME VALUE ACROSS DIFFERENT
        # MACHINES; use getHashStr instead
        return hash(repr(self))
    def __repr__(self):
        # this is used to produce a hash value
        return str(self.entTypeName2typeDesc)
