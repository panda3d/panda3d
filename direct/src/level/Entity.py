"""Entity.py: contains the Entity class"""

from DirectObject import *
from PythonUtil import lineInfo
import string
import DirectNotifyGlobal

class Entity(DirectObject):
    """Entity is the base class for all objects that exist in a Level
    and can be edited with the LevelEditor."""
    notify = DirectNotifyGlobal.directNotify.newCategory('Entity')

    __attribs__ = (
        'type',
        'name',
        'comment',
        )

    def __init__(self, level=None, entId=None):
        self.initializeEntity(level, entId)

    def initializeEntity(self, level, entId):
        """Distributed entities don't know their level or entId values
        until they've been generated, so they call this after they've
        been generated. At that point, the entity is good to go."""
        self.level = level
        self.entId = entId
        if (self.level is not None) and (self.entId is not None):
            self.level.initializeEntity(self)

    def __str__(self):
        return 'ent%s(%s)' % (self.entId, self.level.getEntityType(self.entId))
    
    def destroy(self):
        self.level.onEntityDestroy(self.entId)
        del self.level
        
    def privGetSetter(self, attrib):
        setFuncName = 'set%s%s' % (string.upper(attrib[0]), attrib[1:])
        if hasattr(self, setFuncName):
            return getattr(self, setFuncName)
        return None

    def callSetters(self, *attribs):
        """call this with a list of attribs, and any that exist on the
        entity and have setters will be passed to their setter"""
        self.privCallSetters(0, *attribs)

    def callSettersAndDelete(self, *attribs):
        """same as callSetters, but also removes attribs from entity"""
        self.privCallSetters(1, *attribs)

    def privCallSetters(self, doDelete, *attribs):
        """common implementation of callSetters and callSettersAndDelete"""
        for attrib in attribs:
            if hasattr(self, attrib):
                setter = self.privGetSetter(attrib)
                if setter is not None:
                    value = getattr(self, attrib)
                    if doDelete:
                        delattr(self, attrib)
                    setter(value)

    # this will be called with each item of our spec data on initialization
    def setAttribInit(self, attrib, value):
        if hasattr(self, attrib):
            Entity.notify.warning('%s already has member %s in %s' %
                                  (self, attrib, lineInfo()[2]))
        # TODO: we should probably put this crep in a dictionary
        # rather than dump it into the entity's namespace
        self.__dict__[attrib] = value

    if __debug__:
        def getAttribDescriptors(self):
            # lazy compilation
            if not self.__class__.__dict__.has_key('_attribDescs'):
                self.compileAttribDescs()
            return self.__class__.__dict__['_attribDescs']

        def compileAttribDescs(self):
            Entity.notify.debug('compiling attrib descriptors for %s' %
                                self.__class__.__name__)
            # create a complete list of attribute descriptors, pulling in
            # the attribs from the entire class heirarchy
            def getClassList(obj, self=self):
                """returns list, ordered from most-derived to base classes,
                depth-first. Multiple inheritance base classes that do not
                derive from Entity are listed before those that do.
                """
                assert (type(obj) == types.ClassType)
                classList = [obj]

                # no need to go below Entity
                if obj == Entity:
                    return classList

                # explore the base classes
                entityBases = []
                nonEntityBases = []
                for base in obj.__bases__:
                    l = getClassList(base)
                    if Entity in l:
                        entityBases.extend(l)
                    else:
                        nonEntityBases.extend(l)
                # put bases that derive from Entity last
                classList = classList + nonEntityBases + entityBases
                return classList

            def getUniqueClassList(obj, self=self):
                classList = getClassList(obj)
                # remove duplicates, leaving the last instance
                uniqueList = []
                for i in range(len(classList)):
                    if classList[i] not in classList[(i+1):]:
                        uniqueList.append(classList[i])
                return uniqueList

            classList = getUniqueClassList(self.__class__)

            # work backwards, through the class list, from Entity to the
            # most-derived class, aggregating attribute descriptors.
            allAttribs = []

            def isDistObjAI(obj):
                # util func: is this class a DistributedObjectAI?
                lineage = getClassLineage(obj)
                for item in lineage:
                    if type(item) == types.ClassType:
                        if item.__name__ == 'DistributedObjectAI':
                            return 1
                return 0

            while len(classList):
                cl = classList.pop()
                Entity.notify.debug('looking for attribs on %s' % cl.__name__)

                def getClassAttr(cl, name, self=self):
                    """grab an attribute, such as __attribs__, off of a class"""
                    if cl.__dict__.has_key(name):
                        return cl.__dict__[name]
                    elif isDistObjAI(cl):
                        # It's a distributed AI class.
                        # Check the client-side class
                        globals = {}
                        locals = {}
                        ccn = cl.__name__[:-2] # clientClassName
                        Entity.notify.debug('importing client class %s' % ccn)
                        try:
                            exec 'import %s' % ccn in globals, locals
                        except:
                            print 'could not import %s' % ccn
                            return None
                        exec 'attr = %s.%s.__dict__.get("%s")' % (
                            ccn, ccn, name) in globals, locals
                        return locals['attr']
                    else:
                        return None

                # delete some attribs?
                delAttribs = getClassAttr(cl, '__delAttribs__')
                if delAttribs is not None:
                    assert type(delAttribs) in (types.TupleType, types.ListType)
                    Entity.notify.debug('delAttribs: %s' % list(delAttribs))
                    for attrib in delAttribs:
                        if attrib in allAttribs:
                            allAttribs.remove(attrib)

                attribs = getClassAttr(cl, '__attribs__')
                if attribs is not None:
                    assert type(attribs) in (types.TupleType, types.ListType)
                    Entity.notify.debug('attribs: %s' % list(attribs))
                    for attrib in attribs:
                        if attrib not in allAttribs:
                            allAttribs.append(attrib)

            # we now have an ordered list of all of the attribute descriptors
            # for this class. Cache it on the class object
            Entity.notify.debug('all attribs: %s' % allAttribs)
            self.__class__.__dict__['_attribDescs'] = allAttribs

        # support for level editing
        def handleAttribChange(self, attrib, value):
            # call callback function if it exists
            # otherwise set attrib directly and call notify func
            setter = self.privGetSetter(attrib)
            if setter is not None:
                # call the setter
                setter(value)
            else:
                # set the attrib directly
                self.__dict__[attrib] = value
                # and call the notify func
                self.attribChanged(attrib, value)

        def attribChanged(self, attrib, value):
            """This is called when a parameter is tweaked and no setter
            is called; i.e. the value is set directly on the object.
            Some Entities might want to completely reset every time anything
            is tweaked; this is the place to do it, just override this func
            in your derived class
            """
            pass

        """
        def getAttribInfo(self):
            return self.attribs
        """

        def debugPrint(self, message):
            """for debugging"""
            return self.notify.debug(
                    str(self.__dict__.get('entId', '?'))+' '+message)
