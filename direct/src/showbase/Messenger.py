
from PythonUtil import *
from DirectNotifyGlobal import *


class Messenger:

    notify = None
    
    def __init__(self):
        """ __init__(self)
        One dictionary does it all. It has the following structure:
            {event1 : {object1: [method, extraArgs, persistent],
                       object2: [method, extraArgs, persistent]},
             event2 : {object1: [method, extraArgs, persistent],
                       object2: [method, extraArgs, persistent]}}

        Or, for an example with more real data:
            {'mouseDown' : {avatar : [avatar.jump, [2.0], 1]}}
        """
        self.dict = {}

        if (Messenger.notify == None):
            Messenger.notify = directNotify.newCategory("Messenger")

        # Messenger.notify.setDebug(1)
            
        
    def accept(self, event, object, method, extraArgs=[], persistent=1):
        """ accept(self, string, DirectObject, Function, List, Boolean)
        
        Make this object accept this event. When the event is
        sent (using Messenger.send or from C++), method will be executed,
        optionally passing in extraArgs.
        
        If the persistent flag is set, it will continue to respond
        to this event, otherwise it will respond only once.
        """

        Messenger.notify.debug('object: ' + `object`
                  + '\n accept: ' + `event`
                  + '\n method: ' + `method`
                  + '\n extraArgs: ' + `extraArgs`
                  + '\n persistent: ' + `persistent`)
            
        acceptorDict = ifAbsentPut(self.dict, event, {})
        acceptorDict[object] = [method, extraArgs, persistent]

    def ignore(self, event, object):
        """ ignore(self, string, DirectObject)
        Make this object no longer respond to this event.
        It is safe to call even if it was not alread
        """

        Messenger.notify.debug(`object` + '\n ignore: ' + `event`)
            
        if self.dict.has_key(event):
            # Find the dictionary of all the objects accepting this event
            acceptorDict = self.dict[event]
            # If this object is there, delete it from the dictionary
            if acceptorDict.has_key(object):
                del acceptorDict[object]
            # If this dictionary is now empty, remove the event
            # entry from the Messenger alltogether
            if (len(acceptorDict) == 0):
                del self.dict[event]

    def isAccepting(self, event, object):
        """ isAccepting(self, string, DirectOject)        
        Is this object accepting this event?
        """
        if self.dict.has_key(event):
            if self.dict[event].has_key(object):
                # Found it, return true
                return 1
                
        # If we looked in both dictionaries and made it here
        # that object must not be accepting that event.
        return 0
        
    def isIgnoring(self, event, object):
        """ isIgnorning(self, string, DirectObject)
        Is this object ignoring this event?
        """
        return (not self.isAccepting(event, object))

    def send(self, event, sentArgs=[]):
        """ send(self, string, [arg1, arg2,...])
        Send this event, optionally passing in arguments
        """
        
        # Do not print the new frame debug, it is too noisy!
        if (event != 'NewFrame'):
            Messenger.notify.debug('sent event: ' + event + ' sentArgs: ' + `sentArgs`)

        if self.dict.has_key(event):
            acceptorDict = self.dict[event]
            for object in acceptorDict.keys():
                method, extraArgs, persistent = acceptorDict[object]
                apply(method, (extraArgs + sentArgs))
                # If this object was only accepting this event once,
                # remove it from the dictionary
                if not persistent:
                    del acceptorDict[object]
                    # If this dictionary is now empty, remove the event
                    # entry from the Messenger alltogether
                    if (len(acceptorDict) == 0):
                        del self.dict[event]

    def clear(self):
        """clear(self)
        Start fresh with a clear dict
        """
        self.dict.clear()

    def __repr__(self):
        """__repr__(self)
        Print out the table in a readable format
        """
        str = 'Messenger\n'
        str = str + '='*50 + '\n'
        for event in self.dict.keys():
            acceptorDict = self.dict[event]
            str = str + event + '\n'
            for object in acceptorDict.keys():
                method, extraArgs, persistent = acceptorDict[object]
                str = str + '\t' + `object` + '\n\t' + `method` + '\n\t' + `extraArgs` + ' ' + `persistent` + '\n'
        str = str + '='*50 + '\n'
        return str


