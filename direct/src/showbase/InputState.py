

import DirectNotifyGlobal
import DirectObject


class InputState(DirectObject.DirectObject):
    """
    InputState is for tracking the on/off state of some events.
    The initial usage is to watch some keyboard keys so that another
    task can poll the key states.  By the way, in general polling is
    not a good idea, but it is useful in some situations.  Know when
    to use it :)  If in doubt, don't use this class and listen for
    events instead.
    """
    
    notify = DirectNotifyGlobal.directNotify.newCategory("InputState")

    def __init__(self):
        self.state = {}
        assert(self.debugPrint("InputState()"))
    
    def delete(self):
        self.ignoreAll()
    
    def watch(self, name, eventOn, eventOff, default = 0):
        """
        name is any string (or actually any valid dictionary key).
        eventOn is the string name of the Messenger event that will
            set the state (set to 1).
        eventOff is the string name of the Messenger event that will
            clear the state (set to 0).
        default is the initial value (this will be returned from
            isSet() if a call is made before any eventOn or eventOff
            events occur.
        See Also: ignore()
        """
        assert(self.debugPrint(
            "watch(name=%s, eventOn=%s, eventOff=%s, default=%s)"%(
            name, eventOn, eventOff, default)))
        self.accept(eventOn, self.set, [name, 1])
        self.accept(eventOff, self.set, [name, 0])
        self.state[name] = default
    
    def ignore(self, name):
        """
        The opposite of watch(name, ...)
        See Also: watch()
        """
        self.ignore(eventOn)
        self.ignore(eventOff)
        del state[name]
    
    def set(self, name, isSet):
        assert(self.debugPrint("set(name=%s, isSet=%s)"%(name, isSet)))
        self.state[name] = isSet
    
    def isSet(self, name):
        """
        returns 0, 1, or None (if we're not tracking it at all)
        """
        #assert(self.debugPrint("isSet(name=%s)"%(name)))
        return self.state.get(name)
    
    if __debug__:
        def debugPrint(self, message):
            """for debugging"""
            return self.notify.debug(
                    "%s (%s) %s"%(id(self), len(self.state), message))
    