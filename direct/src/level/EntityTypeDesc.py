"""EntityTypeDesc module: contains the EntityTypeDesc class"""

from direct.directnotify import DirectNotifyGlobal
import AttribDesc
from direct.showbase.PythonUtil import mostDerivedLast

class EntityTypeDesc:
    """This class is meta-data that describes an Entity type."""
    notify = DirectNotifyGlobal.directNotify.newCategory('EntityTypeDesc')

    output = None

    def __init__(self):
        self.__class__.privCompileAttribDescs(self.__class__)

        self.attribNames = []
        self.attribDescDict = {}

        # ordered list of attrib descriptors
        attribDescs = self.__class__._attribDescs

        # create ordered list of attrib names, dict of attrib name to attribDesc
        for desc in attribDescs:
            attribName = desc.getName()
            self.attribNames.append(attribName)
            self.attribDescDict[attribName] = desc

    def isConcrete(self):
        """ means that entity of this exact type can be created """
        return not self.__class__.__dict__.has_key('abstract')

    def isPermanent(self):
        """ means that entity of this exact type cannot be inserted or
        removed in the editor """
        return self.__class__.__dict__.has_key('permanent')

    def getOutputType(self):
        return self.output

    def getAttribNames(self):
        """ returns ordered list of attribute names for this entity type """
        return self.attribNames

    def getAttribDescDict(self):
        """ returns dict of attribName -> attribDescriptor """
        return self.attribDescDict

    def getAttribsOfType(self, type):
        """returns list of attrib names of the given type"""
        names = []
        for attribName, desc in self.attribDescDict.items():
            if desc.getDatatype() == type:
                names.append(attribName)
        return names

    def privCompileAttribDescs(entTypeClass):
        """this compiles an ordered list of attribDescs for the Entity class
        passed in. The attribute descriptors describe the properties of each
        of the Entity type's attributes"""
        # has someone already compiled the info?
        if entTypeClass.__dict__.has_key('_attribDescs'):
            return

        c = entTypeClass
        EntityTypeDesc.notify.debug('compiling attrib descriptors for %s' %
                                    c.__name__)

        # make sure all of our base classes have their complete list of
        # attribDescs
        for base in c.__bases__:
            EntityTypeDesc.privCompileAttribDescs(base)

        # aggregate the attribute descriptors from our direct base classes
        blockAttribs = c.__dict__.get('blockAttribs', [])
        baseADs = []

        bases = list(c.__bases__)
        # make sure base-class attribs show up before derived-class attribs
        mostDerivedLast(bases)

        for base in bases:
            for desc in base._attribDescs:
                # are we blocking this attribute?
                if desc.getName() in blockAttribs:
                    continue
                    
                # make sure we haven't already picked up this attribute
                # from an earlier base class
                for d in baseADs:
                    if desc.getName() == d.getName():
                        EntityTypeDesc.notify.warning(
                            '%s inherits attrib %s from multiple bases' %
                            (c.__name__, desc.getName()))
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
                        assert ad not in baseADs
                        break
                    
                attribDescs.append(desc)

        c._attribDescs = baseADs + attribDescs
    privCompileAttribDescs = staticmethod(privCompileAttribDescs)

    def __str__(self):
        return str(self.__class__)
    def __repr__(self):
        # this is used to produce a hash value
        return (str(self.__class__.__dict__.get('type',None))+
                str(self.output)+
                str(self.attribDescDict))
