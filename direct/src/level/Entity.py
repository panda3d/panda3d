"""Entity.py: contains the Entity class"""

from DirectObject import *
from PythonUtil import lineInfo
import string
import DirectNotifyGlobal

class Entity(DirectObject):
    """Entity is the base class for all objects that exist in a Level
    and can be edited with the LevelEditor."""
    notify = DirectNotifyGlobal.directNotify.newCategory('Entity')

    # these are values that can be changed in the level editor
    Attribs = (
        # Name, PythonType, CallSetterOnInitialization
        ('name', str, 0),
        ('comment', str, 0),
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
                self.attribChanged(attrib)

        def attribChanged(self, attrib):
            """This is called when a parameter is tweaked and no setter
            is called; i.e. the value is set directly on the object.
            Some Entities might want to completely reset every time anything
            is tweaked; this is the place to do it, just override this func
            in your derived class
            """
            pass

        def getAttribInfo(self):
            return self.attribs

    if __debug__:
        def debugPrint(self, message):
            """for debugging"""
            return self.notify.debug(
                    str(self.__dict__.get('entId', '?'))+' '+message)
