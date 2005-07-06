
from otp.ai.AIBaseGlobal import *
from direct.directnotify import DirectNotifyGlobal
from direct.showbase.DirectObject import DirectObject
from ConnectionRepository import *

if __debug__:
    BreakOnTimeout = config.GetBool("break-on-timeout", 0)


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

    def __init__(self, air, replyToChannelId=None, timeout=4.0):
        """
        air is the AI Respository.
        replyToChannelId may be an avatarId, an accountId, or a channelId.
        timeout is how many seconds to wait before aborting the request.
        """
        assert self.notify.debugCall()
        assert isinstance(air, ConnectionRepository) # The api to AsyncRequest has changed.
        #DirectObject.DirectObject.__init__(self)
        self.air=air
        self.replyToChannelId=replyToChannelId
        self.neededObjects={}
        self.timeoutTask=taskMgr.doMethodLater(
            timeout, self.timeout, "AsyncRequestTimer-%s"%(id(self,)))

    def delete(self):
        assert self.notify.debugCall()
        self.timeoutTask.remove()
        del self.timeoutTask
        self.ignoreAll()
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
        if __debug__:
            global BreakOnTimeout
            if BreakOnTimeout:
                print "\n\nself.neededObjects =", self.neededObjects
                print "\ntimed out after %s seconds."%(task.delayTime,)
                import pdb; pdb.set_trace()
        self.delete()

    def _checkCompletion(self, name, context, distObj):
        """
        This checks whether we have all the needed objects and calls
        finish() if we do.
        """
        assert self.notify.debugCall()
        if name is not None:
            self.neededObjects[name]=distObj
        else:
            self.neededObjects[distObj.doId]=distObj
        for i in self.neededObjects.values():
            if i is None:
                return
        self.finish()

    def askForObject(self, doId, context=None):
        """
        Request an already created object, i.e. read from database.
        """
        assert self.notify.debugCall()
        assert doId
        object = self.air.doId2do.get(doId)
        self.neededObjects[doId]=object
        if object is not None:
            self._checkCompletion(None, context, object)
        else:
            if context is None:
                context=self.air.allocateContext()
            self.accept(
                "doRequestResponse-%s"%(context,), 
                self._checkCompletion, [None])
            self.air.queryObjectAll(doId, context)

    def createObject(self, name, className, context=None, values=None):
        """
        Create a new database object.  You can get the doId from within
        your self.finish() function.
        """
        assert self.notify.debugCall()
        assert name
        assert className
        self.neededObjects[name]=None
        if context is None:
            context=self.air.allocateContext()
        self.accept(
            "doRequestResponse-%s"%(context,), self._checkCompletion, [name])
        self.air.requestDatabaseGenerate(className, context, values=values)

    def finish(self):
        """
        This is the function that gets called when all of the needed objects 
        are in (i.e. all the askForObject and createObject requests have 
        been satisfied).
        If the other requests timeout, finish will not be called.
        """
        assert self.notify.debugCall()
        self.delete()
