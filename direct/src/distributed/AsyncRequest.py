
from otp.ai.AIBaseGlobal import *
from direct.directnotify import DirectNotifyGlobal
from direct.showbase.DirectObject import DirectObject


class AsyncRequest(DirectObject):
    def __init__(self, manager, air, replyToChannelId=None, timeout=4.0):
        assert self.notify.debugCall()
        #DirectObject.DirectObject.__init__(self)
        self.manager=manager
        self.air=air
        self.replyToChannelId=replyToChannelId
        self.neededObjects={}
        self.timeoutTask=taskMgr.doMethodLater(timeout, self.timeout, "AsyncRequestTimer-%s"%(id(self,)))

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
        del self.manager
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
        object = self.air.doId2do.get(doId)
        if object is not None:
            self._checkCompletion(None, context, object)
        else:
            if context is None:
                context=self.air.allocateContext()
            self.accept("doRequestResponse-%s"%(context,), self._checkCompletion, [None])
            self.air.queryObjectSnapshot(doId, context)

    def createObject(self, name, className, context=None):
        """
        Create a new database object.  You can get the doId from within
        your self.finish() function.
        """
        assert self.notify.debugCall()
        if context is None:
            context=self.air.allocateContext()
        self.accept("doRequestResponse-%s"%(context,), self._checkCompletion, [name])
        self.air.requestDatabaseGenerate(className, context)

    def finish(self):
        """
        This is the function that gets called when all of the needed objects are in.
        I.e. all the askForObject and createObject requests have been satisfied.
        If the other requests timeout, finish will not be called.
        """
        assert self.notify.debugCall()
        self.delete()
