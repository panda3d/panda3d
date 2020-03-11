#from otp.ai.AIBaseGlobal import *
from direct.directnotify import DirectNotifyGlobal
from direct.showbase.DirectObject import DirectObject
from .ConnectionRepository import *
from panda3d.core import ConfigVariableDouble, ConfigVariableInt, ConfigVariableBool

ASYNC_REQUEST_DEFAULT_TIMEOUT_IN_SECONDS = 8.0
ASYNC_REQUEST_INFINITE_RETRIES = -1
ASYNC_REQUEST_DEFAULT_NUM_RETRIES = 0

if __debug__:
    _overrideTimeoutTimeForAllAsyncRequests = ConfigVariableDouble("async-request-timeout", -1.0)
    _overrideNumRetriesForAllAsyncRequests = ConfigVariableInt("async-request-num-retries", -1)
    _breakOnTimeout = ConfigVariableBool("async-request-break-on-timeout", False)

class AsyncRequest(DirectObject):
    """
    This class is used to make asynchronous reads and creates to a database.

    You can create a list of self.neededObjects and then ask for each to be
    read or created, or if you only have one object that you need you can
    skip the self.neededObjects because calling askForObject or createObject
    will set the self.neededObjects value for you.

    Once all the objects have been read or created, the self.finish() method
    will be called.  You may override this function to run your code in a
    derived class.

    If you wish to queue up several items that you all need before the finish
    method is called, you can put items in self.neededObjects and then call
    askForObject or createObject afterwards.  That way the _checkCompletion
    will not call finish until after all the requests have been done.

    If you need to chain serveral object reads or creates, just add more
    entries to the self.neededObjects dictionary in the self.finish function
    and return without calling AsyncRequest.finish().  Your finish method
    will be called again when the new self.neededObjects is complete.  You
    may repeat this as necessary.
    """
    _asyncRequests = {}

    notify = DirectNotifyGlobal.directNotify.newCategory('AsyncRequest')

    def __init__(self, air, replyToChannelId = None,
                 timeoutTime = ASYNC_REQUEST_DEFAULT_TIMEOUT_IN_SECONDS,
                 numRetries = ASYNC_REQUEST_DEFAULT_NUM_RETRIES):
        """
        air is the AI Respository.
        replyToChannelId may be an avatarId, an accountId, or a channelId.
        timeoutTime is how many seconds to wait before aborting the request.
        numRetries is the number of times to retry the request before giving up.
        """
        assert AsyncRequest.notify.debugCall()
        if __debug__:
            if _overrideTimeoutTimeForAllAsyncRequests.getValue() >= 0.0:
                timeoutTime = _overrideTimeoutTimeForAllAsyncRequests.getValue()
            if _overrideNumRetriesForAllAsyncRequests.getValue() >= 0:
                numRetries = _overrideNumRetriesForAllAsyncRequests.getValue()
        AsyncRequest._asyncRequests[id(self)] = self
        self.deletingMessage = "AsyncRequest-deleting-%s"%(id(self,))
        self.air = air
        self.replyToChannelId = replyToChannelId
        self.timeoutTask = None
        self.neededObjects = {}
        self._timeoutTime = timeoutTime
        self._initialNumRetries = numRetries

    def delete(self):
        assert AsyncRequest.notify.debugCall()
        del AsyncRequest._asyncRequests[id(self)]
        self.ignoreAll()
        self._resetTimeoutTask(False)
        messenger.send(self.deletingMessage, [])
        del self.neededObjects
        del self.air
        del self.replyToChannelId

    def askForObjectField(
            self, dclassName, fieldName, doId, key = None, context = None):
        """
        Request an already created object, i.e. read from database.
        """
        assert AsyncRequest.notify.debugCall()
        if key is None:
            # default the dictionary key to the fieldName
            key = fieldName
        assert doId
        if context is None:
            context = self.air.allocateContext()
        self.air.contextToClassName[context] = dclassName
        self.acceptOnce(
            "doFieldResponse-%s"%(context,),
            self._checkCompletion, [key])

        self.neededObjects[key] = None

        self.air.queryObjectField(dclassName, fieldName, doId, context)
        self._resetTimeoutTask()

    def askForObjectFields(
            self, dclassName, fieldNames, doId, key = None, context = None):
        """
        Request an already created object, i.e. read from database.
        """
        assert AsyncRequest.notify.debugCall()
        if key is None:
            # default the dictionary key to the fieldName
            key = fieldNames[0]
        assert doId
        if context is None:
            context = self.air.allocateContext()
        self.air.contextToClassName[context] = dclassName
        self.acceptOnce(
            "doFieldResponse-%s"%(context,),
            self._checkCompletion, [key])
        self.air.queryObjectFields(dclassName, fieldNames, doId, context)
        self._resetTimeoutTask()

    def askForObjectFieldsByString(self, dbId, dclassName, objString, fieldNames, key=None, context=None):
        assert AsyncRequest.notify.debugCall()
        assert dbId
        if key is None:
            # default the dictionary key to the fieldNames
            key = fieldNames
        if context is None:
            context=self.air.allocateContext()
        self.air.contextToClassName[context]=dclassName
        self.acceptOnce(
            "doFieldResponse-%s"%(context,),
            self._checkCompletion, [key])
        self.air.queryObjectStringFields(dbId,dclassName,objString,fieldNames,context)
        self._resetTimeoutTask()

    def askForObject(self, doId, context = None):
        """
        Request an already created object, i.e. read from database.
        """
        assert AsyncRequest.notify.debugCall()
        assert doId
        if context is None:
            context = self.air.allocateContext()
        self.acceptOnce(
            "doRequestResponse-%s"%(context,),
            self._checkCompletion, [None])
        self.air.queryObjectAll(doId, context)
        self._resetTimeoutTask()

    def createObject(self, name, className,
            databaseId = None, values = None, context = None):
        """
        Create a new database object.  You can get the doId from within
        your self.finish() function.

        This functions is different from createObjectId in that it does
        generate the object when the response comes back.  The object is
        added to the doId2do and so forth and treated as a full regular
        object (which it is).  This is useful on the AI where we really
        do want the object on the AI.
        """
        assert AsyncRequest.notify.debugCall()
        assert name
        assert className
        self.neededObjects[name] = None
        if context is None:
            context = self.air.allocateContext()
        self.accept(
            self.air.getDatabaseGenerateResponseEvent(context),
            self._doCreateObject, [name, className, values])
        self.air.requestDatabaseGenerate(
            className, context, databaseId = databaseId, values = values)
        self._resetTimeoutTask()

    def createObjectId(self, name, className, values = None, context = None):
        """
        Create a new database object.  You can get the doId from within
        your self.finish() function.

        This functions is different from createObject in that it does not
        generate the object when the response comes back.  It only tells you
        the doId.  This is useful on the UD where we don't really want the
        object on the UD, we just want the object created and the UD wants
        to send messages to it using the ID.
        """
        assert AsyncRequest.notify.debugCall()
        assert name
        assert className
        self.neededObjects[name] = None
        if context is None:
            context = self.air.allocateContext()
        self.accept(
            self.air.getDatabaseGenerateResponseEvent(context),
            self._checkCompletion, [name, None])
        self.air.requestDatabaseGenerate(className, context, values = values)
        self._resetTimeoutTask()

    def finish(self):
        """
        This is the function that gets called when all of the needed objects
        are in (i.e. all the askForObject and createObject requests have
        been satisfied).
        If the other requests timeout, finish will not be called.
        """
        assert self.notify.debugCall()
        self.delete()

    def _doCreateObject(self, name, className, values, doId):
        isInDoId2do = doId in self.air.doId2do
        distObj = self.air.generateGlobalObject(doId, className, values)
        if not isInDoId2do and game.name == 'uberDog':
            # only remove doId if this is the uberdog?, in pirates this was
            # causing traded inventory objects to be generated twice, once
            # here and again later when it was noticed the doId was not in
            # the doId2do list yet.
            self.air.doId2do.pop(doId, None)
        self._checkCompletion(name, None, distObj)

    def _checkCompletion(self, name, context, distObj):
        """
        This checks whether we have all the needed objects and calls
        finish() if we do.
        """
        if name is not None:
            self.neededObjects[name] = distObj
        else:
            self.neededObjects[distObj.doId] = distObj
        for i in self.neededObjects.values():
            if i is None:
                return
        self.finish()

    def _resetTimeoutTask(self, createAnew = True):
        if self.timeoutTask:
            taskMgr.remove(self.timeoutTask)
            self.timeoutTask = None
        if createAnew:
            self.numRetries = self._initialNumRetries
            self.timeoutTask = taskMgr.doMethodLater(
                self._timeoutTime, self.timeout,
                "AsyncRequestTimer-%s"%(id(self,)))

    def timeout(self, task):
        assert AsyncRequest.notify.debugCall(
            "neededObjects: %s"%(self.neededObjects,))
        if self.numRetries > 0:
            assert AsyncRequest.notify.debug(
                'Timed out. Trying %d more time(s) : %s' %
                (self.numRetries + 1, repr(self.neededObjects)))
            self.numRetries -= 1
            return Task.again
        else:
            if __debug__:
                if _breakOnTimeout:
                    if hasattr(self, "avatarId"):
                        print("\n\nself.avatarId =", self.avatarId)
                    print("\nself.neededObjects =", self.neededObjects)
                    print("\ntimed out after %s seconds.\n\n"%(task.delayTime,))
                    import pdb; pdb.set_trace()
            self.delete()
            return Task.done

def cleanupAsyncRequests():
    """
    Only call this when the application is shuting down.
    """
    for asyncRequest in AsyncRequest._asyncRequests:
        asyncRequest.delete()
    assert AsyncRequest._asyncRequests == {}
