"""RelatedObjectMgr module: contains the RelatedObjectMgr class"""

# from direct.showbase.ShowBaseGlobal import *
from direct.showbase import DirectObject
from direct.directnotify import DirectNotifyGlobal

class RelatedObjectMgr(DirectObject.DirectObject):
    """
    This class manages a relationship between DistributedObjects that
    know about each other, and are expected to be generated together.
    Ideally, we should be able to guarantee the ordering of the
    generate calls, but there are certain cases in which the objects
    may not be generated in the correct order as defined by the
    toon.dc file.

    To handle cases like these robustly, it is necessary for each
    object to deal with the possibility that its companion object has
    not yet been generated.  This may mean deferring some operations
    until the expected companion object has been generated.

    This class helps manage that process.  To use it, an object should
    register its desire to be associated with the other object's doId.
    When the other object is generated (or immediately, if the object
    already exists), the associated callback will be called.  There is
    also a timeout callback in case the object never appears.
    """
    notify = DirectNotifyGlobal.directNotify.newCategory('RelatedObjectMgr')

    doLaterSequence = 1

    def __init__(self, cr):
        self.cr = cr
        self.pendingObjects = {}

    def destroy(self):
        self.abortAllRequests()
        del self.cr
        del self.pendingObjects

    def requestObjects(self, doIdList, allCallback = None, eachCallback = None,
                       timeout = None, timeoutCallback = None):
        """
        Requests a callback to be called when the objects in the
        doIdList are generated.  The allCallback will be called only
        when all the objects have been generated (and it receives a
        list of objects, in the order given in doIdList).  The
        eachCallback is called as each object is generated, and
        receives only the object itself.

        If the objects already exist, the appropriate callback is
        called immediately.

        If all of the objects are not generated within the indicated
        timeout time, the timeoutCallback is called instead, with the
        original doIdList as the parameter.  If the timeoutCallback is
        None, then allCallback is called on timeout, with the list of
        objects that have been generated so far, and None for objects
        that have not been generated.

        If any element of doIdList is None or 0, it is ignored, and
        None is passed in its place in the object list passed to the
        callback.

        The return value may be saved and passed to a future call to
        abortRequest(), in order to abort a pending request before the
        timeout expires.

        Actually, you should be careful to call abortRequest() if you
        have made a call to requestObjects() that has not been resolved.
        To find examples, do a search for abortRequest() to find out
        how other code is using it.  A common idiom is to store the
        result from requestObjects() and call abortRequest() if delete()
        or destroy() is called on the requesting object.

        See Also: abortRequest()
        """
        assert self.notify.debug("requestObjects(%s, timeout=%s)" % (doIdList, timeout))

        # First, see if we have all of the objects already.
        objects, doIdsPending = self.__generateObjectList(doIdList)

        # Call the eachCallback immediately on any objects we already
        # have.
        if eachCallback:
            for object in objects:
                if object:
                    eachCallback(object)

        if len(doIdsPending) == 0:
            # All the objects exist, so just call the callback
            # immediately.
            assert self.notify.debug("All objects already exist.")
            if allCallback:
                allCallback(objects)
            return

        # Some objects don't exist yet, so start listening for them, and
        # also set a timeout in case they don't come.
        assert self.notify.debug("Some objects pending: %s" % (doIdsPending))

        # Make a copy of the original doIdList, so we can save it over
        # a period of time without worrying about the caller modifying
        # it.
        doIdList = doIdList[:]

        doLaterName = None
        if timeout != None:
            doLaterName = "RelatedObject-%s" % (RelatedObjectMgr.doLaterSequence)
            assert self.notify.debug("doLaterName = %s" % (doLaterName))

            RelatedObjectMgr.doLaterSequence += 1

        tuple = (allCallback, eachCallback, timeoutCallback,
                 doIdsPending, doIdList, doLaterName)

        for doId in doIdsPending:
            pendingList = self.pendingObjects.get(doId)
            if pendingList == None:
                pendingList = []
                self.pendingObjects[doId] = pendingList
                self.__listenFor(doId)

            pendingList.append(tuple)

        if doLaterName:
            # Now spawn a do-later to catch the timeout.
            taskMgr.doMethodLater(timeout, self.__timeoutExpired, doLaterName,
                                  extraArgs = [tuple])

        return tuple

    def abortRequest(self, tuple):
        """
        Aborts a previous request.  The parameter is the return value
        from a previous call to requestObjects().  The pending request
        is removed from the queue and no further callbacks will be called.

        See Also: requestObjects()
        """
        if tuple:
            allCallback, eachCallback, timeoutCallback, doIdsPending, doIdList, doLaterName = tuple
            assert self.notify.debug("aborting request for %s (remaining: %s)" % (doIdList, doIdsPending))

            if doLaterName:
                taskMgr.remove(doLaterName)
            self.__removePending(tuple, doIdsPending)

    def abortAllRequests(self):
        """
        Call this method to abruptly abort all pending requests, but
        leave the RelatedObjectMgr in a state for accepting more
        requests.
        """

        # Stop listening for all events.
        self.ignoreAll()

        # Iterate through all the pendingObjects and stop any pending
        # tasks.
        for pendingList in self.pendingObjects.values():
            for tuple in pendingList:
                allCallback, eachCallback, timeoutCallback, doIdsPending, doIdList, doLaterName = tuple
                if doLaterName:
                    taskMgr.remove(doLaterName)

        self.pendingObjects = {}


    def __timeoutExpired(self, tuple):
        allCallback, eachCallback, timeoutCallback, doIdsPending, doIdList, doLaterName = tuple
        assert self.notify.debug("timeout expired for %s (remaining: %s)" % (doIdList, doIdsPending))

        self.__removePending(tuple, doIdsPending)

        if timeoutCallback:
            timeoutCallback(doIdList)
        else:
            objects, doIdsPending = self.__generateObjectList(doIdList)
            if allCallback:
                allCallback(objects)

    def __removePending(self, tuple, doIdsPending):
        # Removes all the pending events for the doIdsPending list.
        while len(doIdsPending) > 0:
            # We pop doId's off the list instead of simply iterating
            # through the list, so that we will shorten the list (and
            # all other outstanding instances of the list) as we go.
            doId = doIdsPending.pop()
            pendingList = self.pendingObjects[doId]
            pendingList.remove(tuple)
            if len(pendingList) == 0:
                del self.pendingObjects[doId]
                self.__noListenFor(doId)


    def __listenFor(self, doId):
        # Start listening for the indicated object to be generated.
        assert self.notify.debug("Now listening for generate from %s" % (doId))
        announceGenerateName = "generate-%s" % (doId)
        self.acceptOnce(announceGenerateName, self.__generated)

    def __noListenFor(self, doId):
        # Stop listening for the indicated object to be generated.
        assert self.notify.debug("No longer listening for generate from %s" % (doId))
        announceGenerateName = "generate-%s" % (doId)
        self.ignore(announceGenerateName)

    def __generated(self, object):
        # The indicated object has been generated.
        doId = object.doId
        assert self.notify.debug("Got generate from %s" % (doId))
        pendingList = self.pendingObjects[doId]
        del self.pendingObjects[doId]

        for tuple in pendingList:
            allCallback, eachCallback, timeoutCallback, doIdsPending, doIdList, doLaterName = tuple

            # Here we are depending on Python to unify this one list
            # across all objects that share it.  When we remove our
            # doId from our reference to the list, it is also removed
            # from all the other references.
            doIdsPending.remove(doId)

            if eachCallback:
                eachCallback(object)

            if len(doIdsPending) == 0:
                # That was the last doId on the list.  Call the
                # allCallback!
                assert self.notify.debug("All objects generated on list: %s" % (doIdList,))
                if doLaterName:
                    taskMgr.remove(doLaterName)

                objects, doIdsPending = self.__generateObjectList(doIdList)
                if None in objects:
                    assert self.notify.warning('calling %s with None.\n objects=%s\n doIdsPending=%s\n doIdList=%s\n' % (allCallback,objects,doIdsPending,doIdList))
                if allCallback:
                    allCallback(objects)

            else:
                assert self.notify.debug("Objects still pending: %s" % (doIdsPending))

    def __generateObjectList(self, doIdList):
        objects = []
        doIdsPending = []

        for doId in doIdList:
            if doId:
                object = self.cr.doId2do.get(doId)
                objects.append(object)
                if object == None:
                    doIdsPending.append(doId)
            else:
                objects.append(None)

        return objects, doIdsPending

