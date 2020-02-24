"""This defines the Messenger class, which is responsible for most of the
event handling that happens on the Python side.
"""

__all__ = ['Messenger']


from .PythonUtil import *
from direct.directnotify import DirectNotifyGlobal
import types

from direct.stdpy.threading import Lock


class Messenger:

    notify = DirectNotifyGlobal.directNotify.newCategory("Messenger")

    def __init__(self):
        """
        One is keyed off the event name. It has the following structure::

            {event1: {object1: [method, extraArgs, persistent],
                       object2: [method, extraArgs, persistent]},
             event2: {object1: [method, extraArgs, persistent],
                       object2: [method, extraArgs, persistent]}}

        This dictionary allows for efficient callbacks when the
        messenger hears an event.

        A second dictionary remembers which objects are accepting which
        events. This allows for efficient ignoreAll commands.

        Or, for an example with more real data::

            {'mouseDown': {avatar: [avatar.jump, [2.0], 1]}}
        """
        # eventName->objMsgrId->callbackInfo
        self.__callbacks = {}
        # objMsgrId->set(eventName)
        self.__objectEvents = {}
        self._messengerIdGen = 0
        # objMsgrId->listenerObject
        self._id2object = {}

        # A mapping of taskChain -> eventList, used for sending events
        # across task chains (and therefore across threads).
        self._eventQueuesByTaskChain = {}

        # This protects the data structures within this object from
        # multithreaded access.
        self.lock = Lock()

        if __debug__:
            self.__isWatching=0
            self.__watching={}
        # I'd like this to be in the __debug__, but I fear that someone will
        # want this in a release build.  If you're sure that that will not be
        # then please remove this comment and put the quiet/verbose stuff
        # under __debug__.
        self.quieting={"NewFrame":1,
                       "avatarMoving":1,
                       "event-loop-done":1,
                       'collisionLoopFinished':1,
                       } # see def quiet()

    def _getMessengerId(self, object):
        # TODO: allocate this id in DirectObject.__init__ and get derived
        # classes to call down (speed optimization, assuming objects
        # accept/ignore more than once over their lifetime)
        # get unique messenger id for this object
        # assumes lock is held.
        if not hasattr(object, '_MSGRmessengerId'):
            object._MSGRmessengerId = (object.__class__.__name__, self._messengerIdGen)
            self._messengerIdGen += 1
        return object._MSGRmessengerId

    def _storeObject(self, object):
        # store reference-counted reference to object in case we need to
        # retrieve it later.  assumes lock is held.
        id = self._getMessengerId(object)
        if id not in self._id2object:
            self._id2object[id] = [1, object]
        else:
            self._id2object[id][0] += 1

    def _getObject(self, id):
        return self._id2object[id][1]

    def _getObjects(self):
        self.lock.acquire()
        try:
            objs = []
            for refCount, obj in self._id2object.values():
                objs.append(obj)
            return objs
        finally:
            self.lock.release()

    def _getNumListeners(self, event):
        return len(self.__callbacks.get(event, {}))

    def _getEvents(self):
        return list(self.__callbacks.keys())

    def _releaseObject(self, object):
        # assumes lock is held.
        id = self._getMessengerId(object)
        if id in self._id2object:
            record = self._id2object[id]
            record[0] -= 1
            if record[0] <= 0:
                del self._id2object[id]

    def future(self, event):
        """ Returns a future that is triggered by the given event name.  This
        will function only once. """

        return eventMgr.eventHandler.get_future(event)

    def accept(self, event, object, method, extraArgs=[], persistent=1):
        """ accept(self, string, DirectObject, Function, List, Boolean)

        Make this object accept this event. When the event is
        sent (using Messenger.send or from C++), method will be executed,
        optionally passing in extraArgs.

        If the persistent flag is set, it will continue to respond
        to this event, otherwise it will respond only once.
        """
        notifyDebug = Messenger.notify.getDebug()
        if notifyDebug:
            Messenger.notify.debug(
                "object: %s (%s)\n accepting: %s\n method: %s\n extraArgs: %s\n persistent: %s" %
                (safeRepr(object), self._getMessengerId(object), event, safeRepr(method),
                 safeRepr(extraArgs), persistent))

        # Make sure that the method is callable
        assert hasattr(method, '__call__'), (
            "method not callable in accept (ignoring): %s %s"%
            (safeRepr(method), safeRepr(extraArgs)))

        # Make sure extraArgs is a list or tuple
        if not (isinstance(extraArgs, list) or isinstance(extraArgs, tuple) or isinstance(extraArgs, set)):
            raise TypeError("A list is required as extraArgs argument")

        self.lock.acquire()
        try:
            acceptorDict = self.__callbacks.setdefault(event, {})

            id = self._getMessengerId(object)

            # Make sure we are not inadvertently overwriting an existing event
            # on this particular object.
            if id in acceptorDict:
                # TODO: we're replacing the existing callback. should this be an error?
                if notifyDebug:
                    oldMethod = acceptorDict[id][0]
                    if oldMethod == method:
                        self.notify.warning(
                            "object: %s was already accepting: \"%s\" with same callback: %s()" %
                            (object.__class__.__name__, safeRepr(event), method.__name__))
                    else:
                        self.notify.warning(
                            "object: %s accept: \"%s\" new callback: %s() supplanting old callback: %s()" %
                            (object.__class__.__name__, safeRepr(event), method.__name__, oldMethod.__name__))

            acceptorDict[id] = [method, extraArgs, persistent]

            # Remember that this object is listening for this event
            eventDict = self.__objectEvents.setdefault(id, {})
            if event not in eventDict:
                self._storeObject(object)
                eventDict[event] = None
        finally:
            self.lock.release()

    def ignore(self, event, object):
        """ ignore(self, string, DirectObject)
        Make this object no longer respond to this event.
        It is safe to call even if it was not already accepting
        """
        if Messenger.notify.getDebug():
            Messenger.notify.debug(
                safeRepr(object) + ' (%s)\n now ignoring: ' % (self._getMessengerId(object), ) + safeRepr(event))

        self.lock.acquire()
        try:
            id = self._getMessengerId(object)

            # Find the dictionary of all the objects accepting this event
            acceptorDict = self.__callbacks.get(event)
            # If this object is there, delete it from the dictionary
            if acceptorDict and id in acceptorDict:
                del acceptorDict[id]
                # If this dictionary is now empty, remove the event
                # entry from the Messenger alltogether
                if (len(acceptorDict) == 0):
                    del self.__callbacks[event]

            # This object is no longer listening for this event
            eventDict = self.__objectEvents.get(id)
            if eventDict and event in eventDict:
                del eventDict[event]
                if (len(eventDict) == 0):
                    del self.__objectEvents[id]

                self._releaseObject(object)
        finally:
            self.lock.release()

    def ignoreAll(self, object):
        """
        Make this object no longer respond to any events it was accepting
        Useful for cleanup
        """
        if Messenger.notify.getDebug():
            Messenger.notify.debug(
                safeRepr(object) + ' (%s)\n now ignoring all events' % (self._getMessengerId(object), ))

        self.lock.acquire()
        try:
            id = self._getMessengerId(object)
            # Get the list of events this object is listening to
            eventDict = self.__objectEvents.get(id)
            if eventDict:
                for event in list(eventDict.keys()):
                    # Find the dictionary of all the objects accepting this event
                    acceptorDict = self.__callbacks.get(event)
                    # If this object is there, delete it from the dictionary
                    if acceptorDict and id in acceptorDict:
                        del acceptorDict[id]
                        # If this dictionary is now empty, remove the event
                        # entry from the Messenger alltogether
                        if (len(acceptorDict) == 0):
                            del self.__callbacks[event]
                    self._releaseObject(object)
                del self.__objectEvents[id]
        finally:
            self.lock.release()

    def getAllAccepting(self, object):
        """
        Returns the list of all events accepted by the indicated object.
        """
        self.lock.acquire()
        try:
            id = self._getMessengerId(object)

            # Get the list of events this object is listening to
            eventDict = self.__objectEvents.get(id)
            if eventDict:
                return list(eventDict.keys())
            return []
        finally:
            self.lock.release()

    def isAccepting(self, event, object):
        """ isAccepting(self, string, DirectOject)
        Is this object accepting this event?
        """
        self.lock.acquire()
        try:
            acceptorDict = self.__callbacks.get(event)
            id = self._getMessengerId(object)
            if acceptorDict and id in acceptorDict:
                # Found it, return true
                return 1
            # If we looked in both dictionaries and made it here
            # that object must not be accepting that event.
            return 0
        finally:
            self.lock.release()

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

    def send(self, event, sentArgs=[], taskChain=None):
        """
        Send this event, optionally passing in arguments.

        Args:
            event (str): The name of the event.
            sentArgs (list): A list of arguments to be passed along to the
                handlers listening to this event.
            taskChain (str, optional): If not None, the name of the task chain
                which should receive the event.  If None, then the event is
                handled immediately. Setting a non-None taskChain will defer
                the event (possibly till next frame or even later) and create a
                new, temporary task within the named taskChain, but this is the
                only way to send an event across threads.
        """
        if Messenger.notify.getDebug() and not self.quieting.get(event):
            assert Messenger.notify.debug(
                'sent event: %s sentArgs = %s, taskChain = %s' % (
                event, sentArgs, taskChain))

        self.lock.acquire()
        try:
            foundWatch=0
            if __debug__:
                if self.__isWatching:
                    for i in self.__watching.keys():
                        if str(event).find(i) >= 0:
                            foundWatch=1
                            break
            acceptorDict = self.__callbacks.get(event)
            if not acceptorDict:
                if __debug__:
                    if foundWatch:
                        print("Messenger: \"%s\" was sent, but no function in Python listened."%(event,))
                return

            if taskChain:
                # Queue the event onto the indicated task chain.
                from direct.task.TaskManagerGlobal import taskMgr
                queue = self._eventQueuesByTaskChain.setdefault(taskChain, [])
                queue.append((acceptorDict, event, sentArgs, foundWatch))
                if len(queue) == 1:
                    # If this is the first (only) item on the queue,
                    # spawn the task to empty it.
                    taskMgr.add(self.__taskChainDispatch, name = 'Messenger-%s' % (taskChain),
                                extraArgs = [taskChain], taskChain = taskChain,
                                appendTask = True)
            else:
                # Handle the event immediately.
                self.__dispatch(acceptorDict, event, sentArgs, foundWatch)
        finally:
            self.lock.release()

    def __taskChainDispatch(self, taskChain, task):
        """ This task is spawned each time an event is sent across
        task chains.  Its job is to empty the task events on the queue
        for this particular task chain.  This guarantees that events
        are still delivered in the same order they were sent. """

        while True:
            eventTuple = None
            self.lock.acquire()
            try:
                queue = self._eventQueuesByTaskChain.get(taskChain, None)
                if queue:
                    eventTuple = queue[0]
                    del queue[0]
                if not queue:
                    # The queue is empty, we're done.
                    if queue is not None:
                        del self._eventQueuesByTaskChain[taskChain]

                if not eventTuple:
                    # No event; we're done.
                    return task.done

                self.__dispatch(*eventTuple)
            finally:
                self.lock.release()

        return task.done

    def __dispatch(self, acceptorDict, event, sentArgs, foundWatch):
        for id in list(acceptorDict.keys()):
            # We have to make this apparently redundant check, because
            # it is possible that one object removes its own hooks
            # in response to a handler called by a previous object.
            #
            # NOTE: there is no danger of skipping over objects due to
            # modifications to acceptorDict, since the for..in above
            # iterates over a list of objects that is created once at
            # the start
            callInfo = acceptorDict.get(id)
            if callInfo:
                method, extraArgs, persistent = callInfo
                # If this object was only accepting this event once,
                # remove it from the dictionary
                if not persistent:
                    # This object is no longer listening for this event
                    eventDict = self.__objectEvents.get(id)
                    if eventDict and event in eventDict:
                        del eventDict[event]
                        if (len(eventDict) == 0):
                            del self.__objectEvents[id]
                        self._releaseObject(self._getObject(id))

                    del acceptorDict[id]
                    # If the dictionary at this event is now empty, remove
                    # the event entry from the Messenger altogether
                    if (event in self.__callbacks \
                            and (len(self.__callbacks[event]) == 0)):
                        del self.__callbacks[event]

                if __debug__:
                    if foundWatch:
                        print("Messenger: \"%s\" --> %s%s"%(
                            event,
                            self.__methodRepr(method),
                            tuple(extraArgs + sentArgs)))

                #print "Messenger: \"%s\" --> %s%s"%(
                #            event,
                #            self.__methodRepr(method),
                #            tuple(extraArgs + sentArgs))

                # It is important to make the actual call here, after
                # we have cleaned up the accept hook, because the
                # method itself might call accept() or acceptOnce()
                # again.
                assert hasattr(method, '__call__')

                # Release the lock temporarily while we call the method.
                self.lock.release()
                try:
                    result = method (*(extraArgs + sentArgs))
                finally:
                    self.lock.acquire()

                if hasattr(result, 'cr_await'):
                    # It's a coroutine, so schedule it with the task manager.
                    taskMgr.add(result)

    def clear(self):
        """
        Start fresh with a clear dict
        """
        self.lock.acquire()
        try:
            self.__callbacks.clear()
            self.__objectEvents.clear()
            self._id2object.clear()
        finally:
            self.lock.release()

    def isEmpty(self):
        return (len(self.__callbacks) == 0)

    def getEvents(self):
        return list(self.__callbacks.keys())

    def replaceMethod(self, oldMethod, newFunction):
        """
        This is only used by Finder.py - the module that lets
        you redefine functions with Control-c-Control-v
        """
        retFlag = 0
        for entry in list(self.__callbacks.items()):
            event, objectDict = entry
            for objectEntry in list(objectDict.items()):
                object, params = objectEntry
                method = params[0]
                if (type(method) == types.MethodType):
                    function = method.__func__
                else:
                    function = method
                #print ('function: ' + repr(function) + '\n' +
                #       'method: ' + repr(method) + '\n' +
                #       'oldMethod: ' + repr(oldMethod) + '\n' +
                #       'newFunction: ' + repr(newFunction) + '\n')
                if (function == oldMethod):
                    newMethod = types.MethodType(
                        newFunction, method.__self__, method.__self__.__class__)
                    params[0] = newMethod
                    # Found it retrun true
                    retFlag += 1
        # didn't find that method, return false
        return retFlag

    def toggleVerbose(self):
        isVerbose = 1 - Messenger.notify.getDebug()
        Messenger.notify.setDebug(isVerbose)
        if isVerbose:
            print("Verbose mode true.  quiet list = %s"%(
                list(self.quieting.keys()),))

    if __debug__:
        def watch(self, needle):
            """
            return a matching event (needle) if found (in haystack).
            This is primarily a debugging tool.

            This is intended for debugging use only.
            This function is not defined if python is ran with -O (optimize).

            See Also: `unwatch`
            """
            if not self.__watching.get(needle):
                self.__isWatching += 1
                self.__watching[needle]=1

        def unwatch(self, needle):
            """
            return a matching event (needle) if found (in haystack).
            This is primarily a debugging tool.

            This is intended for debugging use only.
            This function is not defined if python is ran with -O (optimize).

            See Also: `watch`
            """
            if self.__watching.get(needle):
                self.__isWatching -= 1
                del self.__watching[needle]

        def quiet(self, message):
            """
            When verbose mode is on, don't spam the output with messages
            marked as quiet.
            This is primarily a debugging tool.

            This is intended for debugging use only.
            This function is not defined if python is ran with -O (optimize).

            See Also: `unquiet`
            """
            if not self.quieting.get(message):
                self.quieting[message]=1

        def unquiet(self, message):
            """
            Remove a message from the list of messages that are not reported
            in verbose mode.
            This is primarily a debugging tool.

            This is intended for debugging use only.
            This function is not defined if python is ran with -O (optimize).

            See Also: `quiet`
            """
            if self.quieting.get(message):
                del self.quieting[message]

    def find(self, needle):
        """
        return a matching event (needle) if found (in haystack).
        This is primarily a debugging tool.
        """
        keys = list(self.__callbacks.keys())
        keys.sort()
        for event in keys:
            if repr(event).find(needle) >= 0:
                return {event: self.__callbacks[event]}

    def findAll(self, needle, limit=None):
        """
        return a dict of events (needle) if found (in haystack).
        limit may be None or an integer (e.g. 1).
        This is primarily a debugging tool.
        """
        matches = {}
        keys = list(self.__callbacks.keys())
        keys.sort()
        for event in keys:
            if repr(event).find(needle) >= 0:
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
            functionName = method.__self__.__class__.__name__ + '.' + \
                method.__func__.__name__
        else:
            if hasattr(method, "__name__"):
                functionName = method.__name__
            else:
                return ""
        return functionName

    def __eventRepr(self, event):
        """
        Compact version of event, acceptor pairs
        """
        str = event.ljust(32) + '\t'
        acceptorDict = self.__callbacks[event]
        for key, (method, extraArgs, persistent) in list(acceptorDict.items()):
            str = str + self.__methodRepr(method) + ' '
        str = str + '\n'
        return str

    def __repr__(self):
        """
        Compact version of event, acceptor pairs
        """
        str = "The messenger is currently handling:\n" + "="*64 + "\n"
        keys = list(self.__callbacks.keys())
        keys.sort()
        for event in keys:
            str += self.__eventRepr(event)
        # Print out the object: event dictionary too
        str += "="*64 + "\n"
        for key, eventDict in list(self.__objectEvents.items()):
            object = self._getObject(key)
            str += "%s:\n" % repr(object)
            for event in list(eventDict.keys()):
                str += "     %s\n" % repr(event)

        str += "="*64 + "\n" + "End of messenger info.\n"
        return str

    def detailedRepr(self):
        """
        Print out the table in a detailed readable format
        """
        import types
        str = 'Messenger\n'
        str = str + '='*50 + '\n'
        keys = list(self.__callbacks.keys())
        keys.sort()
        for event in keys:
            acceptorDict = self.__callbacks[event]
            str = str + 'Event: ' + event + '\n'
            for key in list(acceptorDict.keys()):
                function, extraArgs, persistent = acceptorDict[key]
                object = self._getObject(key)
                objectClass = getattr(object, '__class__', None)
                if objectClass:
                    className = objectClass.__name__
                else:
                    className = "Not a class"
                functionName = function.__name__
                str = (str + '\t' +
                       'Acceptor:     ' + className + ' instance' + '\n\t' +
                       'Function name:' + functionName + '\n\t' +
                       'Extra Args:   ' + repr(extraArgs) + '\n\t' +
                       'Persistent:   ' + repr(persistent) + '\n')
                # If this is a class method, get its actual function
                if (type(function) == types.MethodType):
                    str = (str + '\t' +
                           'Method:       ' + repr(function) + '\n\t' +
                           'Function:     ' + repr(function.__func__) + '\n')
                else:
                    str = (str + '\t' +
                           'Function:     ' + repr(function) + '\n')
        str = str + '='*50 + '\n'
        return str

    #snake_case alias:
    get_events = getEvents
    is_ignoring = isIgnoring
    who_accepts = whoAccepts
    find_all = findAll
    replace_method = replaceMethod
    ignore_all = ignoreAll
    is_accepting = isAccepting
    is_empty = isEmpty
    detailed_repr = detailedRepr
    get_all_accepting = getAllAccepting
    toggle_verbose = toggleVerbose
