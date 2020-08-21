from panda3d.core import *
from panda3d.direct import *
from .MsgTypes import *
from direct.task import Task
from direct.directnotify import DirectNotifyGlobal
from . import CRCache
from direct.distributed.CRDataCache import CRDataCache
from direct.distributed.ConnectionRepository import ConnectionRepository
from direct.showbase import PythonUtil
from . import ParentMgr
from . import RelatedObjectMgr
import time
from .ClockDelta import *


class ClientRepositoryBase(ConnectionRepository):
    """
    This maintains a client-side connection with a Panda server.

    This base class exists to collect the common code between
    ClientRepository, which is the CMU-provided, open-source version
    of the client repository code, and OTPClientRepository, which is
    the VR Studio's implementation of the same.
    """
    notify = DirectNotifyGlobal.directNotify.newCategory("ClientRepositoryBase")

    def __init__(self, dcFileNames = None, dcSuffix = '',
                 connectMethod = None, threadedNet = None):
        if connectMethod is None:
            connectMethod = self.CM_HTTP
        ConnectionRepository.__init__(self, connectMethod, base.config, hasOwnerView = True, threadedNet = threadedNet)
        self.dcSuffix = dcSuffix
        if hasattr(self, 'setVerbose'):
            if self.config.GetBool('verbose-clientrepository'):
                self.setVerbose(1)

        self.context=100000
        self.setClientDatagram(1)

        self.deferredGenerates = []
        self.deferredDoIds = {}
        self.lastGenerate = 0
        self.setDeferInterval(base.config.GetDouble('deferred-generate-interval', 0.2))
        self.noDefer = False  # Set this True to temporarily disable deferring.

        self.recorder = base.recorder

        self.readDCFile(dcFileNames)
        self.cache=CRCache.CRCache()
        self.doDataCache = CRDataCache()
        self.cacheOwner=CRCache.CRCache()
        self.serverDelta = 0

        self.bootedIndex = None
        self.bootedText = None

        # create a parentMgr to handle distributed reparents
        # this used to be 'token2nodePath'
        self.parentMgr = ParentMgr.ParentMgr()

        # The RelatedObjectMgr helps distributed objects find each
        # other.
        self.relatedObjectMgr = RelatedObjectMgr.RelatedObjectMgr(self)

        # This will be filled in when a TimeManager is created.
        self.timeManager = None

        # Keep track of how recently we last sent a heartbeat message.
        # We want to keep these coming at heartbeatInterval seconds.
        self.heartbeatInterval = base.config.GetDouble('heartbeat-interval', 10)
        self.heartbeatStarted = 0
        self.lastHeartbeat = 0

        self._delayDeletedDOs = {}

        self.specialNameNumber = 0

    def setDeferInterval(self, deferInterval):
        """Specifies the minimum amount of time, in seconds, that must
        elapse before generating any two DistributedObjects whose
        class type is marked "deferrable".  Set this to 0 to indicate
        no deferring will occur."""

        self.deferInterval = deferInterval
        self.setHandleCUpdates(self.deferInterval == 0)

        if self.deferredGenerates:
            taskMgr.remove('deferredGenerate')
            taskMgr.doMethodLater(self.deferInterval, self.doDeferredGenerate, 'deferredGenerate')

    ## def queryObjectAll(self, doID, context=0):
        ## """
        ## Get a one-time snapshot look at the object.
        ## """
        ## assert self.notify.debugStateCall(self)
        ## # Create a message
        ## datagram = PyDatagram()
        ## datagram.addServerHeader(
            ## doID, localAvatar.getDoId(), 2020)
        ## # A context that can be used to index the response if needed
        ## datagram.addUint32(context)
        ## self.send(datagram)
        ## # Make sure the message gets there.
        ## self.flush()

    def specialName(self, label):
        name = ("SpecialName %s %s" % (self.specialNameNumber, label))
        self.specialNameNumber += 1
        return name

    def getTables(self, ownerView):
        if ownerView:
            return self.doId2ownerView, self.cacheOwner
        else:
            return self.doId2do, self.cache

    def _getMsgName(self, msgId):
        # we might get a list of message names, use the first one
        return makeList(MsgId2Names.get(msgId, 'UNKNOWN MESSAGE: %s' % msgId))[0]

    def allocateContext(self):
        self.context+=1
        return self.context

    def setServerDelta(self, delta):
        """
        Indicates the approximate difference in seconds between the
        client's clock and the server's clock, in universal time (not
        including timezone shifts).  This is mainly useful for
        reporting synchronization information to the logs; don't
        depend on it for any precise timing requirements.

        Also see Notify.setServerDelta(), which also accounts for a
        timezone shift.
        """
        self.serverDelta = delta

    def getServerDelta(self):
        return self.serverDelta

    def getServerTimeOfDay(self):
        """
        Returns the current time of day (seconds elapsed since the
        1972 epoch) according to the server's clock.  This is in GMT,
        and hence is irrespective of timezones.

        The value is computed based on the client's clock and the
        known delta from the server's clock, which is not terribly
        precisely measured and may drift slightly after startup, but
        it should be accurate plus or minus a couple of seconds.
        """
        return time.time() + self.serverDelta

    def doGenerate(self, parentId, zoneId, classId, doId, di):
        # Look up the dclass
        assert parentId == self.GameGlobalsId or parentId in self.doId2do
        dclass = self.dclassesByNumber[classId]
        assert(self.notify.debug("performing generate for %s %s" % (dclass.getName(), doId)))
        dclass.startGenerate()
        # Create a new distributed object, and put it in the dictionary
        distObj = self.generateWithRequiredOtherFields(dclass, doId, di, parentId, zoneId)
        dclass.stopGenerate()

    def flushGenerates(self):
        """ Forces all pending generates to be performed immediately. """
        while self.deferredGenerates:
            msgType, extra = self.deferredGenerates[0]
            del self.deferredGenerates[0]
            self.replayDeferredGenerate(msgType, extra)

        taskMgr.remove('deferredGenerate')

    def replayDeferredGenerate(self, msgType, extra):
        """ Override this to do something appropriate with deferred
        "generate" messages when they are replayed().
        """

        if msgType == CLIENT_ENTER_OBJECT_REQUIRED_OTHER:
            # It's a generate message.
            doId = extra
            if doId in self.deferredDoIds:
                args, deferrable, dg, updates = self.deferredDoIds[doId]
                del self.deferredDoIds[doId]
                self.doGenerate(*args)

                if deferrable:
                    self.lastGenerate = globalClock.getFrameTime()

                for dg, di in updates:
                    # non-DC updates that need to be played back in-order are
                    # stored as (msgType, (dg, di))
                    if type(di) is tuple:
                        msgType = dg
                        dg, di = di
                        self.replayDeferredGenerate(msgType, (dg, di))
                    else:
                        # ovUpdated is set to True since its OV
                        # is assumbed to have occured when the
                        # deferred update was originally received
                        self.__doUpdate(doId, di, True)
        else:
            self.notify.warning("Ignoring deferred message %s" % (msgType))

    def doDeferredGenerate(self, task):
        """ This is the task that generates an object on the deferred
        queue. """

        now = globalClock.getFrameTime()
        while self.deferredGenerates:
            if now - self.lastGenerate < self.deferInterval:
                # Come back later.
                return Task.again

            # Generate the next deferred object.
            msgType, extra = self.deferredGenerates[0]
            del self.deferredGenerates[0]
            self.replayDeferredGenerate(msgType, extra)

        # All objects are generaetd.
        return Task.done

    def generateWithRequiredFields(self, dclass, doId, di, parentId, zoneId):
        if doId in self.doId2do:
            # ...it is in our dictionary.
            # Just update it.
            distObj = self.doId2do[doId]
            assert distObj.dclass == dclass
            distObj.generate()
            distObj.setLocation(parentId, zoneId)
            distObj.updateRequiredFields(dclass, di)
            # updateRequiredFields calls announceGenerate
        elif self.cache.contains(doId):
            # ...it is in the cache.
            # Pull it out of the cache:
            distObj = self.cache.retrieve(doId)
            assert distObj.dclass == dclass
            # put it in the dictionary:
            self.doId2do[doId] = distObj
            # and update it.
            distObj.generate()
            # make sure we don't have a stale location
            distObj.parentId = None
            distObj.zoneId = None
            distObj.setLocation(parentId, zoneId)
            distObj.updateRequiredFields(dclass, di)
            # updateRequiredFields calls announceGenerate
        else:
            # ...it is not in the dictionary or the cache.
            # Construct a new one
            classDef = dclass.getClassDef()
            if classDef == None:
                self.notify.error("Could not create an undefined %s object." % (dclass.getName()))
            distObj = classDef(self)
            distObj.dclass = dclass
            # Assign it an Id
            distObj.doId = doId
            # Put the new do in the dictionary
            self.doId2do[doId] = distObj
            # Update the required fields
            distObj.generateInit()  # Only called when constructed
            distObj._retrieveCachedData()
            distObj.generate()
            distObj.setLocation(parentId, zoneId)
            distObj.updateRequiredFields(dclass, di)
            # updateRequiredFields calls announceGenerate
            self.notify.debug("New DO:%s, dclass:%s" % (doId, dclass.getName()))
        return distObj

    def generateWithRequiredOtherFields(self, dclass, doId, di,
                                        parentId = None, zoneId = None):
        if doId in self.doId2do:
            # ...it is in our dictionary.
            # Just update it.
            distObj = self.doId2do[doId]
            assert distObj.dclass == dclass
            distObj.generate()
            distObj.setLocation(parentId, zoneId)
            distObj.updateRequiredOtherFields(dclass, di)
            # updateRequiredOtherFields calls announceGenerate
        elif self.cache.contains(doId):
            # ...it is in the cache.
            # Pull it out of the cache:
            distObj = self.cache.retrieve(doId)
            assert distObj.dclass == dclass
            # put it in the dictionary:
            self.doId2do[doId] = distObj
            # and update it.
            distObj.generate()
            # make sure we don't have a stale location
            distObj.parentId = None
            distObj.zoneId = None
            distObj.setLocation(parentId, zoneId)
            distObj.updateRequiredOtherFields(dclass, di)
            # updateRequiredOtherFields calls announceGenerate
        else:
            # ...it is not in the dictionary or the cache.
            # Construct a new one
            classDef = dclass.getClassDef()
            if classDef == None:
                self.notify.error("Could not create an undefined %s object." % (dclass.getName()))
            distObj = classDef(self)
            distObj.dclass = dclass
            # Assign it an Id
            distObj.doId = doId
            # Put the new do in the dictionary
            self.doId2do[doId] = distObj
            # Update the required fields
            distObj.generateInit()  # Only called when constructed
            distObj._retrieveCachedData()
            distObj.generate()
            distObj.setLocation(parentId, zoneId)
            distObj.updateRequiredOtherFields(dclass, di)
            # updateRequiredOtherFields calls announceGenerate
        return distObj

    def generateWithRequiredOtherFieldsOwner(self, dclass, doId, di):
        if doId in self.doId2ownerView:
            # ...it is in our dictionary.
            # Just update it.
            self.notify.error('duplicate owner generate for %s (%s)' % (
                doId, dclass.getName()))
            distObj = self.doId2ownerView[doId]
            assert distObj.dclass == dclass
            distObj.generate()
            distObj.updateRequiredOtherFields(dclass, di)
            # updateRequiredOtherFields calls announceGenerate
        elif self.cacheOwner.contains(doId):
            # ...it is in the cache.
            # Pull it out of the cache:
            distObj = self.cacheOwner.retrieve(doId)
            assert distObj.dclass == dclass
            # put it in the dictionary:
            self.doId2ownerView[doId] = distObj
            # and update it.
            distObj.generate()
            distObj.updateRequiredOtherFields(dclass, di)
            # updateRequiredOtherFields calls announceGenerate
        else:
            # ...it is not in the dictionary or the cache.
            # Construct a new one
            classDef = dclass.getOwnerClassDef()
            if classDef == None:
                self.notify.error("Could not create an undefined %s object. Have you created an owner view?" % (dclass.getName()))
            distObj = classDef(self)
            distObj.dclass = dclass
            # Assign it an Id
            distObj.doId = doId
            # Put the new do in the dictionary
            self.doId2ownerView[doId] = distObj
            # Update the required fields
            distObj.generateInit()  # Only called when constructed
            distObj.generate()
            distObj.updateRequiredOtherFields(dclass, di)
            # updateRequiredOtherFields calls announceGenerate
        return distObj


    def disableDoId(self, doId, ownerView=False):
        table, cache = self.getTables(ownerView)
        # Make sure the object exists
        if doId in table:
            # Look up the object
            distObj = table[doId]
            # remove the object from the dictionary
            del table[doId]

            # Only cache the object if it is a "cacheable" type
            # object; this way we don't clutter up the caches with
            # trivial objects that don't benefit from caching.
            # also don't try to cache an object that is delayDeleted
            cached = False
            if distObj.getCacheable() and distObj.getDelayDeleteCount() <= 0:
                cached = cache.cache(distObj)
            if not cached:
                distObj.deleteOrDelay()
                if distObj.getDelayDeleteCount() <= 0:
                    # make sure we're not leaking
                    distObj.detectLeaks()

        elif doId in self.deferredDoIds:
            # The object had been deferred.  Great; we don't even have
            # to generate it now.
            del self.deferredDoIds[doId]
            i = self.deferredGenerates.index((CLIENT_ENTER_OBJECT_REQUIRED_OTHER, doId))
            del self.deferredGenerates[i]
            if len(self.deferredGenerates) == 0:
                taskMgr.remove('deferredGenerate')

        else:
            self._logFailedDisable(doId, ownerView)

    def _logFailedDisable(self, doId, ownerView):
        self.notify.warning(
            "Disable failed. DistObj "
            + str(doId) +
            " is not in dictionary, ownerView=%s" % ownerView)

    def handleDelete(self, di):
        # overridden by ClientRepository
        assert 0

    def handleUpdateField(self, di):
        """
        This method is called when a CLIENT_OBJECT_UPDATE_FIELD
        message is received; it decodes the update, unpacks the
        arguments, and calls the corresponding method on the indicated
        DistributedObject.

        In fact, this method is exactly duplicated by the C++ method
        cConnectionRepository::handle_update_field(), which was
        written to optimize the message loop by handling all of the
        CLIENT_OBJECT_UPDATE_FIELD messages in C++.  That means that
        nowadays, this Python method will probably never be called,
        since UPDATE_FIELD messages will not even be passed to the
        Python message handlers.  But this method remains for
        documentation purposes, and also as a "just in case" handler
        in case we ever do come across a situation in the future in
        which python might handle the UPDATE_FIELD message.
        """
        # Get the DO Id
        doId = di.getUint32()

        ovUpdated = self.__doUpdateOwner(doId, di)

        if doId in self.deferredDoIds:
            # This object hasn't really been generated yet.  Sit on
            # the update.
            args, deferrable, dg0, updates = self.deferredDoIds[doId]

            # Keep a copy of the datagram, and move the di to the copy
            dg = Datagram(di.getDatagram())
            di = DatagramIterator(dg, di.getCurrentIndex())

            updates.append((dg, di))
        else:
            # This object has been fully generated.  It's OK to update.
            self.__doUpdate(doId, di, ovUpdated)


    def __doUpdate(self, doId, di, ovUpdated):
        # Find the DO
        do = self.doId2do.get(doId)
        if do is not None:
            # Let the dclass finish the job
            do.dclass.receiveUpdate(do, di)
        elif not ovUpdated:
            # this next bit is looking for avatar handles so that if you get an update
            # for an avatar that isn't in your doId2do table but there is a
            # avatar handle for that object then it's messages will be forwarded to that
            # object. We are currently using that for whisper echoing
            # if you need a more general perpose system consider registering proxy objects on
            # a dict and adding the avatar handles to that dict when they are created
            # then change/remove the old method. I didn't do that because I couldn't think
            # of a use for it. -JML
            try :
                handle = self.identifyAvatar(doId)
                if handle:
                    dclass = self.dclassesByName[handle.dclassName]
                    dclass.receiveUpdate(handle, di)

                else:
                    self.notify.warning(
                        "Asked to update non-existent DistObj " + str(doId))
            except:
                self.notify.warning(
                        "Asked to update non-existent DistObj " + str(doId) + "and failed to find it")

    def __doUpdateOwner(self, doId, di):
        ovObj = self.doId2ownerView.get(doId)
        if ovObj:
            odg = Datagram(di.getDatagram())
            odi = DatagramIterator(odg, di.getCurrentIndex())
            ovObj.dclass.receiveUpdate(ovObj, odi)
            return True
        return False

    def handleGoGetLost(self, di):
        # The server told us it's about to drop the connection on us.
        # Get ready!
        if (di.getRemainingSize() > 0):
            self.bootedIndex = di.getUint16()
            self.bootedText = di.getString()

            self.notify.warning(
                "Server is booting us out (%d): %s" % (self.bootedIndex, self.bootedText))
        else:
            self.bootedIndex = None
            self.bootedText = None
            self.notify.warning(
                "Server is booting us out with no explanation.")

        # disconnect now, don't wait for send/recv to fail
        self.stopReaderPollTask()
        self.lostConnection()

    def handleServerHeartbeat(self, di):
        # Got a heartbeat message from the server.
        if base.config.GetBool('server-heartbeat-info', 1):
            self.notify.info("Server heartbeat.")

    def handleSystemMessage(self, di):
        # Got a system message from the server.
        message = di.getString()
        self.notify.info('Message from server: %s' % (message))
        return message

    def handleSystemMessageAknowledge(self, di):
        # Got a system message from the server.
        message = di.getString()
        self.notify.info('Message with aknowledge from server: %s' % (message))
        messenger.send("system message aknowledge", [message])
        return message

    def getObjectsOfClass(self, objClass):
        """ returns dict of doId:object, containing all objects
        that inherit from 'class'. returned dict is safely mutable. """
        doDict = {}
        for doId, do in self.doId2do.items():
            if isinstance(do, objClass):
                doDict[doId] = do
        return doDict

    def getObjectsOfExactClass(self, objClass):
        """ returns dict of doId:object, containing all objects that
        are exactly of type 'class' (neglecting inheritance). returned
        dict is safely mutable. """
        doDict = {}
        for doId, do in self.doId2do.items():
            if do.__class__ == objClass:
                doDict[doId] = do
        return doDict


    def considerHeartbeat(self):
        """Send a heartbeat message if we haven't sent one recently."""
        if not self.heartbeatStarted:
            self.notify.debug("Heartbeats not started; not sending.")
            return

        elapsed = globalClock.getRealTime() - self.lastHeartbeat
        if elapsed < 0 or elapsed > self.heartbeatInterval:
            # It's time to send the heartbeat again (or maybe someone
            # reset the clock back).
            self.notify.info("Sending heartbeat mid-frame.")
            self.startHeartbeat()

    def stopHeartbeat(self):
        taskMgr.remove("heartBeat")
        self.heartbeatStarted = 0

    def startHeartbeat(self):
        self.stopHeartbeat()
        self.heartbeatStarted = 1
        self.sendHeartbeat()
        self.waitForNextHeartBeat()

    def sendHeartbeatTask(self, task):
        self.sendHeartbeat()
        return Task.again

    def waitForNextHeartBeat(self):
        taskMgr.doMethodLater(self.heartbeatInterval, self.sendHeartbeatTask,
                              "heartBeat", taskChain = 'net')

    def replaceMethod(self, oldMethod, newFunction):
        return 0

    def getWorld(self, doId):
        # Get the world node for this object
        obj = self.doId2do[doId]

        worldNP = obj.getParent()
        while 1:
            nextNP = worldNP.getParent()
            if nextNP == render:
                break
            elif worldNP.isEmpty():
                return None
        return worldNP

    def isLive(self):
        if base.config.GetBool('force-live', 0):
            return True
        return not (__dev__ or launcher.isTestServer())

    def isLocalId(self, id):
        # By default, no ID's are local.  See also
        # ClientRepository.isLocalId().
        return 0

    # methods for tracking delaydeletes
    def _addDelayDeletedDO(self, do):
        # use the id of the object, it's possible to have multiple DelayDeleted instances
        # with identical doIds if an object gets deleted then re-generated
        key = id(do)
        assert key not in self._delayDeletedDOs
        self._delayDeletedDOs[key] = do

    def _removeDelayDeletedDO(self, do):
        key = id(do)
        del self._delayDeletedDOs[key]

    def printDelayDeletes(self):
        print('DelayDeletes:')
        print('=============')
        for obj in self._delayDeletedDOs.values():
            print('%s\t%s (%s)\tdelayDeletes=%s' % (
                obj.doId, safeRepr(obj), itype(obj), obj.getDelayDeleteNames()))
