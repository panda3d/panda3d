"""Entity.py: contains the Entity class"""

import string

class Entity:
    """Entity is the base class for all objects that exist in a Level
    and can be edited with the LevelEditor."""

    # these are values that can be changed in the level editor
    Attribs = (
        # Name, PythonType, CallSetterOnInitialization
        ('name', str, 0),
        ('comment', str, 0),
        )

    def __init__(self, level, entId, attribs=None):
        self.level = level
        self.entId = entId

        self.attribs = Entity.Attribs
        # add any additional tweakable values
        if attribs is not None:
            self.attribs.update(attribs)

    def initializeEntity(self):
        """Call this once on initialization to set this entity's
        spec data"""
        self.level.initializeEntity(self)

    def destroy(self):
        del self.level
        
    def privGetSetter(self, attrib):
        setFuncName = 'set%s%s' % (string.upper(attrib[0]), attrib[1:])
        if hasattr(self, setFuncName):
            return getattr(self, setFuncName)
        return None

    def callSetters(self, *attribs):
        """call this with a list of attribs, and any that exist on the
        entity and have setters will be passed to their setter"""
        for attrib in attribs:
            if hasattr(self, attrib):
                setter = self.privGetSetter(attrib)
                if setter is not None:
                    setter(getattr(self, attrib))

    def attribChanged(self):
        """This is called when a parameter is tweaked and no setter
        is called; i.e. the value is set directly on the object.
        Some Entities might want to completely reset every time anything
        is tweaked; this is the place to do it; override this func in your
        derived class
        """
        pass

    def getAttribInfo(self):
        return self.attribs

    def privTweak(self, name, value):
        self.__dict__[name] = value

    def __str__(self):
        return 'ent%s(%s)' % (self.entId, self.level.getEntityType(self.entId))
    
    if __debug__:
        def debugPrint(self, message):
            """for debugging"""
            return self.notify.debug(
                    str(self.__dict__.get('entId', '?'))+' '+message)
