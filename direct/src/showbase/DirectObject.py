
from MessengerGlobal import *
from DirectNotifyGlobal import *
from PythonUtil import *

class DirectObject:
    """
    This is the class that all Direct/SAL classes should inherit from
    """
    #def __del__(self):
    #    print "Destructing: ", self.__class__.__name__

    # Event Handling
    def __initEvents(self):
        # this function exists because:
        # - DirectObject does not have a constructor, and adding one
        #   would involve touching many, many files
        # - a constructor that creates self.events would cause every
        #   DirectObject to incur the cost of an additional function
        #   call and dictionary creation
        # - DirectObjects that do not use the messenger should not have
        #   an unused dictionary taking up space
        # - the speed hit of calling this function on calls to accept,
        #   ignore, etc. is negligible, since they are not called often
        if not hasattr(self, "events"):
            # list of events that this object is accepting
            # we use a dictionary to avoid linear searches
            self.events = {}

    def accept(self, event, method, extraArgs=[]):
        self.__initEvents()
        self.events.setdefault(event, None)
        messenger.accept(event, self, method, extraArgs, 1)

    def acceptOnce(self, event, method, extraArgs=[]):
        self.__initEvents()
        self.events.setdefault(event, None)
        messenger.accept(event, self, method, extraArgs, 0)

    def _INTERNAL_acceptOnceExpired(self, event):
        """ this should only be called by the messenger """
        if self.events.has_key(event):
            del self.events[event]

    def ignore(self, event):
        self.__initEvents()
        if self.events.has_key(event):
            del self.events[event]
        messenger.ignore(event, self)

    def ignoreAll(self):
        self.__initEvents()
        for event in self.events.keys():
            messenger.ignore(event, self)
        self.events.clear()

    def isAccepting(self, event):
        self.__initEvents()
        return self.events.has_key(event)

    def isIgnoring(self, event):
        return not self.isAccepting(event)
