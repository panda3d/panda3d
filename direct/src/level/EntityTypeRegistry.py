"""EntityTypeRegistry module: contains the EntityTypeRegistry class"""

import DirectNotifyGlobal
import types
import AttribDesc
import EntityTypes
from PythonUtil import mostDerivedLast

class EntityTypeRegistry:
    notify = DirectNotifyGlobal.directNotify.newCategory('EntityTypeRegistry')

    def __init__(self, entityTypeModule=EntityTypes):
        """pass in a module that contains EntityType classes"""
        # get a list of the entity type classes in the type module
        classes = []
        for key, value in entityTypeModule.__dict__.items():
            if type(value) is types.ClassType:
                if issubclass(value, EntityTypes.Entity):
                    classes.append(value)

        # put derived classes after their bases
        mostDerivedLast(classes)

        # scrub through the class heirarchy and compile complete
        # attribute descriptor lists for each concrete Entity type class
        typeName2class = {}
        for c in classes:
            # if this is a concrete Entity type, add it to the dict
            if c.__dict__.has_key('type'):
                if typeName2class.has_key(c.type):
                    EntityTypeRegistry.notify.debug(
                        "replacing %s with %s for type '%s'" %
                        (typeName2class[c.type], c, c.type))
                typeName2class[c.type] = c

            self.privCompileAttribDescs(c)

        # maps entity typename to ordered list of attrib names
        self.typeName2attribNames = {}
        # maps entity typename to dict of attrib name -> attrib descriptor
        self.typeName2attribDescDict = {}
        
        for typeName, c in typeName2class.items():
            self.typeName2attribNames[typeName] = []
            self.typeName2attribDescDict[typeName] = {}

            # ordered list of attrib descriptors
            attribDescs = c._attribDescs

            for desc in attribDescs:
                attribName = desc.getName()
                self.typeName2attribNames[typeName].append(attribName)
                self.typeName2attribDescDict[typeName][attribName] = desc

    def getAttribNames(self, entityTypeName):
        """ returns ordered list of attribute names for entity type """
        assert entityTypeName in self.typeName2attribNames
        return self.typeName2attribNames[entityTypeName]

    def getAttribDescDict(self, entityTypeName):
        """ returns dict of attribName -> attribDescriptor """
        assert entityTypeName in self.typeName2attribDescDict
        return self.typeName2attribDescDict[entityTypeName]

    def __hash__(self):
        return hash(str(self))
    def __str__(self):
        return str(self.typeName2attribNames)+str(self.typeName2attribDescDict)

    def privCompileAttribDescs(self, entTypeClass):
        # has someone already compiled the info?
        if entTypeClass.__dict__.has_key('_attribDescs'):
            return

        c = entTypeClass
        EntityTypeRegistry.notify.debug('compiling attrib descriptors for %s' %
                                        c.__name__)

        # make sure all of our base classes have their complete list of
        # attribDescs
        for base in c.__bases__:
            self.privCompileAttribDescs(base)

        # aggregate the attribute descriptors from our direct base classes
        delAttribs = c.__dict__.get('delAttribs', [])
        baseADs = []

        bases = list(c.__bases__)
        # make sure that Entity comes first
        if EntityTypes.Entity in bases:
            bases.remove(EntityTypes.Entity)
            bases = [EntityTypes.Entity] + bases

        for base in bases:
            for desc in base._attribDescs:
                # are we blocking this attribute?
                if desc.getName() in delAttribs:
                    continue
                    
                # make sure we haven't already picked up this attribute
                # from an earlier base class
                for d in baseADs:
                    if desc.getName() == d.getName():
                        break
                else:
                    baseADs.append(desc)

        # now that we have all of the descriptors from our base classes,
        # add the descriptors from this class
        attribDescs = []
        if c.__dict__.has_key('attribs'):
            for attrib in c.attribs:
                desc = AttribDesc.AttribDesc(*attrib)
                
                # if we picked up an attribute with the same name from a base
                # class, this overrides it
                for ad in baseADs:
                    if ad.getName() == desc.getName():
                        baseADs.remove(ad)
                        # there ought to be no more than one desc with
                        # this name from the base classes
                        break
                    
                attribDescs.append(desc)

        c._attribDescs = baseADs + attribDescs
