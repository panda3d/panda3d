
from PythonUtil import *
from direct.directnotify import DirectNotifyGlobal
import types

class Messenger:

    notify = DirectNotifyGlobal.directNotify.newCategory("Messenger")

    def __init__(self):
        """
        One is keyed off the event name. It has the following structure:
            {event1 : {object1: [method, extraArgs, persistent],
                       object2: [method, extraArgs, persistent]},
             event2 : {object1: [method, extraArgs, persistent],
                       object2: [method, extraArgs, persistent]}}
                       
        This dictionary allow for efficient callbacks when the messenger
        hears an event.

        A second dictionary remembers which objects are accepting which
        events. This allows for efficient ignoreAll commands.


        Or, for an example with more real data:
            {'mouseDown' : {avatar : [avatar.jump, [2.0], 1]}}
        """
        self.__callbacks = {}
        self.__objectEvents = {}

        if __debug__:
            self.isWatching=0
            self.watching={}
        # I'd like this to be in the __debug__, but I fear that someone will
        # want this in a release build.  If you're sure that that will not be
        # then please remove this comment and put the quiet/verbose stuff
        # under __debug__.
        self.quieting={"NewFrame":1, "avatarMoving":1} # see def quiet()

        
    def accept(self, event, object, method, extraArgs=[], persistent=1):
        """ accept(self, string, DirectObject, Function, List, Boolean)
        
        Make this object accept this event. When the event is
        sent (using Messenger.send or from C++), method will be executed,
        optionally passing in extraArgs.
        
        If the persistent flag is set, it will continue to respond
        to this event, otherwise it will respond only once.
        """
        if Messenger.notify.getDebug():
            Messenger.notify.debug('object: ' + `object`
                                   + '\n now accepting: ' + `event`
                                   + '\n method: ' + `method`
                                   + '\n extraArgs: ' + `extraArgs`
                                   + '\n persistent: ' + `persistent`)
            
        acceptorDict = self.__callbacks.setdefault(event, {})
        acceptorDict[object] = [method, extraArgs, persistent]

        # Remember that this object is listening for this event
        eventDict = self.__objectEvents.setdefault(object, {})
        eventDict.setdefault(event, None)

    def ignore(self, event, object):
        """ ignore(self, string, DirectObject)
        Make this object no longer respond to this event.
        It is safe to call even if it was not already accepting
        """
        if Messenger.notify.getDebug():
            Messenger.notify.debug(`object` + '\n now ignoring: ' + `event`)

        # Find the dictionary of all the objects accepting this event
        acceptorDict = self.__callbacks.get(event)
        # If this object is there, delete it from the dictionary
        if acceptorDict and acceptorDict.has_key(object):
            del acceptorDict[object]
            # If this dictionary is now empty, remove the event
            # entry from the Messenger alltogether
            if (len(acceptorDict) == 0):
                del self.__callbacks[event]

        # This object is no longer listening for this event
        eventDict = self.__objectEvents.get(object)
        if eventDict and eventDict.has_key(event):
            del eventDict[event]
            if (len(eventDict) == 0):
                del self.__objectEvents[object]

    def ignoreAll(self, object):
        """ ignoreAll(self, DirectObject)
        Make this object no longer respond to any events it was accepting
        Useful for cleanup
        """
        if Messenger.notify.getDebug():
            Messenger.notify.debug(`object` + '\n now ignoring all events')            
        # Get the list of events this object is listening to
        eventDict = self.__objectEvents.get(object)        
        if eventDict:
            for event in eventDict.keys():
                # Find the dictionary of all the objects accepting this event
                acceptorDict = self.__callbacks.get(event)
                # If this object is there, delete it from the dictionary
                if acceptorDict and acceptorDict.has_key(object):
                    del acceptorDict[object]
                    # If this dictionary is now empty, remove the event
                    # entry from the Messenger alltogether
                    if (len(acceptorDict) == 0):
                        del self.__callbacks[event]
            del self.__objectEvents[object]

    def isAccepting(self, event, object):
        """ isAccepting(self, string, DirectOject)        
        Is this object accepting this event?
        """
        acceptorDict = self.__callbacks.get(event)
        if acceptorDict and acceptorDict.has_key(object):
            # Found it, return true
            return 1
        # If we looked in both dictionaries and made it here
        # that object must not be accepting that event.
        return 0

    def whoAccepts(self, event):
        """
        Return objects accepting the given event
        """
        return self.__callbacks.get(event)
        
    def isIgnoring(self, event, object):
        """ isIgnorning(self, string, DirectObject)
        Is this object ignoring this event?
        """
        return (not self.isAccepting(event, object))

    def send(self, event, sentArgs=[]):
        """
        event is usually a string.
        sentArgs is a list of any data that you want passed along to the
            handlers listening to this event.
        
        Send this event, optionally passing in arguments
        """
        if Messenger.notify.getDebug() and not self.quieting.get(event):
            Messenger.notify.debug('sent event: ' + event + ' sentArgs: ' + `sentArgs`)
        if __debug__:
            foundWatch=0
            if self.isWatching:
                for i in self.watching.keys():
                    if str(event).find(i) >= 0:
                        foundWatch=1
                        break
        acceptorDict = self.__callbacks.get(event)
        if not acceptorDict:
            if __debug__:
                if foundWatch:
                    print "Messenger: \"%s\" was sent, but no function in Python listened."%(event,)
            return
        for object in acceptorDict.keys():
            # We have to make this apparently redundant check, because
            # it is possible that one object removes its own hooks
            # in response to a handler called by a previous object.
            #
            # NOTE: there is no danger of skipping over objects due to
            # modifications to acceptorDict, since the for..in above
            # iterates over a list of objects that is created once at
            # the start
            callInfo = acceptorDict.get(object)
            if callInfo:
                method, extraArgs, persistent = callInfo
                # If this object was only accepting this event once,
                # remove it from the dictionary
                if not persistent:
                    # This object is no longer listening for this event
                    eventDict = self.__objectEvents.get(object)
                    if eventDict and eventDict.has_key(event):
                        del eventDict[event]
                        if (len(eventDict) == 0):
                            del self.__objectEvents[object]

                    del acceptorDict[object]
                    # If the dictionary at this event is now empty, remove
                    # the event entry from the Messenger altogether
                    if (self.__callbacks.has_key(event) and (len(self.__callbacks[event]) == 0)):
                        del self.__callbacks[event]

                if __debug__:
                    if foundWatch:
                        print "Messenger: \"%s\" --> %s%s"%(
                            event,
                            self.__methodRepr(method),
                            tuple(extraArgs + sentArgs))

                # It is important to make the actual call here, after
                # we have cleaned up the accept hook, because the
                # method itself might call accept() or acceptOnce()
                # again.
                apply(method, (extraArgs + sentArgs))

    def clear(self):
        """
        Start fresh with a clear dict
        """
        self.__callbacks.clear()
        self.__objectEvents.clear()

    def isEmpty(self):
        return (len(self.__callbacks) == 0)

    def getEvents(self):
        return self.__callbacks.keys()
        
    def replaceMethod(self, oldMethod, newFunction):
        """
        This is only used by Finder.py - the module that lets
        you redefine functions with Control-c-Control-v
        """
        import new
        retFlag = 0
        for entry in self.__callbacks.items():
            event, objectDict = entry
            for objectEntry in objectDict.items():
                object, params = objectEntry
                method = params[0]
                if (type(method) == types.MethodType):
                    function = method.im_func
                else:
                    function = method
                #print ('function: ' + `function` + '\n' +
                #       'method: ' + `method` + '\n' +
                #       'oldMethod: ' + `oldMethod` + '\n' +
                #       'newFunction: ' + `newFunction` + '\n')
                if (function == oldMethod):
                    newMethod = new.instancemethod(newFunction,
                                                   method.im_self,
                                                   method.im_class)
                    params[0] = newMethod
                    # Found it retrun true
                    retFlag += 1
        # didn't find that method, return false
        return retFlag
    
    def toggleVerbose(self):
        isVerbose = 1 - Messenger.notify.getDebug()
        Messenger.notify.setDebug(isVerbose)
        if isVerbose:
            print "Verbose mode true.  quiet list = %s"%(self.quieting.keys(),)

    if __debug__:
        def watch(self, needle):
            """
            return a matching event (needle) if found (in haystack).
            This is primarily a debugging tool.
            See Also: unwatch
            """
            if not self.watching.get(needle):
                self.isWatching += 1
                self.watching[needle]=1

        def unwatch(self, needle):
            """
            return a matching event (needle) if found (in haystack).
            This is primarily a debugging tool.
            See Also: watch
            """
            if self.watching.get(needle):
                self.isWatching -= 1
                del self.watching[needle]

        def quiet(self, message):
            """
            When verbose mode is on, don't spam the output with messages
            marked as quiet.
            This is primarily a debugging tool.
            See Also: unquiet
            """
            if not self.quieting.get(message):
                self.quieting[message]=1

        def unquiet(self, message):
            """
            Remove a message from the list of messages that are not reported
            in verbose mode.
            This is primarily a debugging tool.
            See Also: quiet
            """
            if self.quieting.get(message):
                del self.quieting[message]

    def find(self, needle):
        """
        return a matching event (needle) if found (in haystack).
        This is primarily a debugging tool.
        """
        keys = self.__callbacks.keys()
        keys.sort()
        for event in keys:
            if `event`.find(needle) >= 0:
                print self.__eventRepr(event),
                return {event: self.__callbacks[event]}

    def findAll(self, needle, limit=None):
        """
        return a dict of events (needle) if found (in haystack).
        limit may be None or an integer (e.g. 1).
        This is primarily a debugging tool.
        """
        matches = {}
        keys = self.__callbacks.keys()
        keys.sort()
        for event in keys:
            if `event`.find(needle) >= 0:
                print self.__eventRepr(event),
                matches[event] = self.__callbacks[event]
                # if the limit is not None, decrement and
                # check for break:
                if limit > 0:
                    limit -= 1
                    if limit == 0:
                        break
        return matches

    def __methodRepr(self, method):
        """
        return string version of class.method or method.
        """
        if (type(method) == types.MethodType):
            functionName = method.im_class.__name__ + '.' + method.im_func.__name__
        else:
            functionName = method.__name__
        return functionName

    def __eventRepr(self, event):
        """
        Compact version of event, acceptor pairs
        """
        str = event.ljust(32) + '\t'
        acceptorDict = self.__callbacks[event]
        for object, (method, extraArgs, persistent) in acceptorDict.items():
            str = str + self.__methodRepr(method) + ' '
        str = str + '\n'
        return str

    def __repr__(self):
        """
        Compact version of event, acceptor pairs
        """
        str = "The messenger is currently handling:\n" + "="*64 + "\n"
        keys = self.__callbacks.keys()
        keys.sort()
        for event in keys:
            str += self.__eventRepr(event)
        # Print out the object : event dictionary too
        str += "="*64 + "\n"
        for object, eventDict in self.__objectEvents.items():
            str += "%s:\n" % object
            for event in eventDict.keys():
                str += "     %s\n" % event
            
        str += "="*64 + "\n" + "End of messenger info.\n"
        return str

    def detailedRepr(self):
        """
        Print out the table in a detailed readable format
        """
        import types
        str = 'Messenger\n'
        str = str + '='*50 + '\n'
        keys = self.__callbacks.keys()
        keys.sort()
        for event in keys:
            acceptorDict = self.__callbacks[event]
            str = str + 'Event: ' + event + '\n'
            for object in acceptorDict.keys():
                function, extraArgs, persistent = acceptorDict[object]
                if (type(object) == types.InstanceType):
                    className = object.__class__.__name__
                else:
                    className = "Not a class"
                functionName = function.__name__
                str = (str + '\t' +
                       'Acceptor:     ' + className + ' instance' + '\n\t' +
                       'Function name:' + functionName + '\n\t' +
                       'Extra Args:   ' + `extraArgs` + '\n\t' +
                       'Persistent:   ' + `persistent` + '\n')
                # If this is a class method, get its actual function
                if (type(function) == types.MethodType):
                    str = (str + '\t' +
                           'Method:       ' + `function` + '\n\t' +
                           'Function:     ' + `function.im_func` + '\n')
                else:
                    str = (str + '\t' +
                           'Function:     ' + `function` + '\n')
        str = str + '='*50 + '\n'
        return str

