
from otp.ai.AIBaseGlobal import *
from direct.directnotify import DirectNotifyGlobal
from direct.showbase.DirectObject import DirectObject
from ConnectionRepository import *

DefaultTimeout = 8.0
if __debug__:
    DefaultTimeout = config.GetFloat("async-request-default-timeout", 8.0)
    BreakOnTimeout = config.GetBool("async-request-break-on-timeout", 0)

_asyncRequests={}

def _addActiveAsyncRequest(asyncRequest):
    global _asyncRequests
    _asyncRequests[id(asyncRequest)]=asyncRequest

def _removeActiveAsyncRequest(asyncRequest):
    global _asyncRequests
    del _asyncRequests[id(asyncRequest)]

def cleanupAsyncRequests():
    """
    Only call this when the application is shuting down.
    """
    global _asyncRequests
    for asyncRequest in _asyncRequests:
        asyncRequest.delete()
    assert _asyncRequests == {}
    _asyncRequests={}

class AsyncRequest(DirectObject):
    """
    This class is used to make asynchronos reads and creates to a database.

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
    if __debug__:
        notify = DirectNotifyGlobal.directNotify.newCategory('AsyncRequest')

    def __init__(self, air, replyToChannelId=None, timeout=DefaultTimeout):
        """
        air is the AI Respository.
        replyToChannelId may be an avatarId, an accountId, or a channelId.
        timeout is how many seconds to wait before aborting the request.
        """
        assert self.notify.debugCall()
        if __debug__:
            self.__deleted=False
        _addActiveAsyncRequest(self)
        self.deletingMessage="AsyncRequest-deleting-%s"%(id(self,))
        #DirectObject.DirectObject.__init__(self)
        self.air=air
        self.replyToChannelId=replyToChannelId
        self.neededObjects={}
        self.timeoutTask=taskMgr.doMethodLater(
            timeout, self.timeout, "AsyncRequestTimer-%s"%(id(self,)))

    def delete(self):
        assert self.notify.debugCall()
        assert not self.__deleted
        if __debug__:
            self.__deleted=True
        _removeActiveAsyncRequest(self)
        self.ignoreAll()
        self.timeoutTask.remove()
        del self.timeoutTask
        messenger.send(self.deletingMessage, [])
        if 0:
            for i in self.neededObjects.values():
                if i is not None:
                    #self.air.unRegisterForChannel(o.account.doId)
                    #self.air.removeDOFromTables(o.account)
                    #if 0:
                    #    o.account.delete()
                    #    self.air.deleteObject(o.account.getDoId())
                    self.air.removeDOFromTables(i)
                    i.delete()
        del self.neededObjects
        del self.air
        del self.replyToChannelId
        #DirectObject.DirectObject.delete(self)

    def timeout(self, task):
        """
        If this is called we have not gotten the needed objects in the timeout
        period.  Derived classes should inform the user or whomever and then
        call this base method to cleanup.
        """
        assert self.notify.debugCall("neededObjects: %s"%(self.neededObjects,))
        assert not self.__deleted
        if __debug__:
            global BreakOnTimeout
            if BreakOnTimeout:
                if hasattr(self, "avatarId"):
                    print "\n\nself.avatarId =", self.avatarId
                print "\nself.neededObjects =", self.neededObjects
                print "\ntimed out after %s seconds.\n\n"%(task.delayTime,)
                import pdb; pdb.set_trace()
        self.delete()

    def _checkCompletion(self, name, context, distObj):
        """
        This checks whether we have all the needed objects and calls
        finish() if we do.
        """
        assert self.notify.debugCall()
        assert not self.__deleted
        if name is not None:
            self.neededObjects[name]=distObj
        else:
            self.neededObjects[distObj.doId]=distObj
        for i in self.neededObjects.values():
            if i is None:
                return
        self.finish()

    def askForObjectField(
            self, dclassName, fieldName, doId, key=None, context=None):
        """
        Request an already created object, i.e. read from database.
        """
        assert self.notify.debugCall()
        assert not self.__deleted
        if key is None:
            # default the dictionary key to the fieldName
            key = fieldName
        assert doId
        ## object = self.air.doId2do.get(doId)
        ## self.neededObjects[key]=object
        if 0 and object is not None:
            self._checkCompletion(key, None, object)
        else:
            if context is None:
                context=self.air.allocateContext()
            self.air.contextToClassName[context]=dclassName
            self.acceptOnce(
                "doFieldResponse-%s"%(context,), 
                self._checkCompletion, [key])
            self.air.queryObjectField(dclassName, fieldName, doId, context)

    def askForObject(self, doId, context=None):
        """
        Request an already created object, i.e. read from database.
        """
        assert self.notify.debugCall()
        assert not self.__deleted
        assert doId
        #object = self.air.doId2do.get(doId)
        #self.neededObjects[doId]=object
        #if object is not None:
        #    self._checkCompletion(None, context, object)
        #else:
        if 1:
            if context is None:
                context=self.air.allocateContext()
            self.acceptOnce(
                "doRequestResponse-%s"%(context,), 
                self._checkCompletion, [None])
            self.air.queryObjectAll(doId, context)

    #def addInterestInObject(self, doId, context=None):
    #    """
    #    Request an already created object, i.e. read from database
    #    and claim a long term interest in the object (get updates, etc.).
    #    """
    #    assert self.notify.debugCall()
    #    assert doId
    #    object = self.air.doId2do.get(doId)
    #    self.neededObjects[doId]=object
    #    if object is not None:
    #        self._checkCompletion(None, context, object)
    #    else:
    #        if context is None:
    #            context=self.air.allocateContext()
    #        self.accept(
    #            "doRequestResponse-%s"%(context,), 
    #            self._checkCompletion, [None])
    #        self.air.queryObject(doId, context)

    def createObject(self, name, className, context=None, values=None):
        """
        Create a new database object.  You can get the doId from within
        your self.finish() function.
        
        This functions is different from createObjectId in that it does
        generate the object when the response comes back.  The object is
        added to the doId2do and so forth and treated as a full regular
        object (which it is).  This is useful on the AI where we really 
        do want the object on the AI.
        """
        assert self.notify.debugCall()
        assert not self.__deleted
        assert name
        assert className
        self.neededObjects[name]=None
        if context is None:
            context=self.air.allocateContext()
        self.accept(
            self.air.getDatabaseGenerateResponseEvent(context),
            self._doCreateObject, [name, className, values])
        ## newDBRequestGen = config.GetBool(#HACK:
        ##     'new-database-request-generate', 1)
        ## if newDBRequestGen:
        ##     self.accept(
        ##         self.air.getDatabaseGenerateResponseEvent(context),
        ##         self._doCreateObject, [name, className, values])
        ## else:
        ##     self.accept(
        ##         "doRequestResponse-%s"%(context,), self._checkCompletion, [name])
        self.air.requestDatabaseGenerate(className, context, values=values)

    def createObjectId(self, name, className, context=None, values=None):
        """
        Create a new database object.  You can get the doId from within
        your self.finish() function.
        
        This functions is different from createObject in that it does not
        generate the object when the response comes back.  It only tells you
        the doId.  This is useful on the UD where we don't really want the
        object on the UD, we just want the object created and the UD wants
        to send messages to it using the ID.
        """
        assert self.notify.debugCall()
        assert not self.__deleted
        assert name
        assert className
        self.neededObjects[name]=None
        if context is None:
            context=self.air.allocateContext()
        self.accept(
            self.air.getDatabaseGenerateResponseEvent(context),
            self._checkCompletion, [name, None])
        self.air.requestDatabaseGenerate(className, context, values=values)
    
    def _doCreateObject(self, name, className, values, doId):
        assert self.notify.debugCall()
        assert not self.__deleted
        isInDoId2do = doId in self.air.doId2do
        distObj = self.air.generateGlobalObject(doId, className, values)
        if not isInDoId2do:
            self.air.doId2do.pop(doId, None)
        self._checkCompletion(name, None, distObj)

    def finish(self):
        """
        This is the function that gets called when all of the needed objects 
        are in (i.e. all the askForObject and createObject requests have 
        been satisfied).
        If the other requests timeout, finish will not be called.
        """
        assert self.notify.debugCall()
        assert not self.__deleted
        self.delete()
