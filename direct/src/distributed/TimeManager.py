from direct.showbase.DirectObject import *
from panda3d.core import *
from direct.task import Task
from direct.distributed import DistributedObject
from direct.directnotify import DirectNotifyGlobal
from direct.distributed.ClockDelta import globalClockDelta

class TimeManager(DistributedObject.DistributedObject):
    """
    This DistributedObject lives on the AI and on the client side, and
    serves to synchronize the time between them so they both agree, to
    within a few hundred milliseconds at least, what time it is.

    It uses a pull model where the client can request a
    synchronization check from time to time.  It also employs a
    round-trip measurement to minimize the effect of latency.
    """

    notify = DirectNotifyGlobal.directNotify.newCategory("TimeManager")

    # The number of seconds to wait between automatic
    # synchronizations.  Set to 0 to disable auto sync after
    # startup.
    updateFreq = ConfigVariableDouble('time-manager-freq', 1800).getValue()

    # The minimum number of seconds to wait between two unrelated
    # synchronization attempts.  Increasing this number cuts down
    # on frivolous synchronizations.
    minWait = ConfigVariableDouble('time-manager-min-wait', 10).getValue()

    # The maximum number of seconds of uncertainty to tolerate in
    # the clock delta without trying again.
    maxUncertainty = ConfigVariableDouble('time-manager-max-uncertainty', 1).getValue()

    # The maximum number of attempts to try to get a low-latency
    # time measurement before giving up and accepting whatever we
    # get.
    maxAttempts = ConfigVariableInt('time-manager-max-attempts', 5).getValue()

    # A simulated clock skew for debugging, in seconds.
    extraSkew = ConfigVariableInt('time-manager-extra-skew', 0).getValue()

    if extraSkew != 0:
        notify.info("Simulating clock skew of %0.3f s" % extraSkew)

    reportFrameRateInterval = ConfigVariableDouble('report-frame-rate-interval', 300.0).getValue()

    def __init__(self, cr):
        DistributedObject.DistributedObject.__init__(self, cr)

        self.thisContext = -1
        self.nextContext = 0
        self.attemptCount = 0
        self.start = 0
        self.lastAttempt = -self.minWait*2

    ### DistributedObject methods ###

    def generate(self):
        """
        This method is called when the DistributedObject is reintroduced
        to the world, either for the first time or from the cache.
        """
        DistributedObject.DistributedObject.generate(self)

        self.accept('clock_error', self.handleClockError)

        if self.updateFreq > 0:
            self.startTask()

    def announceGenerate(self):
        DistributedObject.DistributedObject.announceGenerate(self)
        self.cr.timeManager = self
        self.synchronize("TimeManager.announceGenerate")

    def disable(self):
        """
        This method is called when the DistributedObject is removed from
        active duty and stored in a cache.
        """
        self.ignore('clock_error')
        self.stopTask()
        taskMgr.remove('frameRateMonitor')
        if self.cr.timeManager is self:
            self.cr.timeManager = None
        DistributedObject.DistributedObject.disable(self)

    def delete(self):
        """
        This method is called when the DistributedObject is permanently
        removed from the world and deleted from the cache.
        """
        DistributedObject.DistributedObject.delete(self)

    ### Task management methods ###

    def startTask(self):
        self.stopTask()
        taskMgr.doMethodLater(self.updateFreq, self.doUpdate, "timeMgrTask")

    def stopTask(self):
        taskMgr.remove("timeMgrTask")

    def doUpdate(self, task):
        self.synchronize("timer")
        # Spawn the next one
        taskMgr.doMethodLater(self.updateFreq, self.doUpdate, "timeMgrTask")
        return Task.done

    ### Automatic clock error handling ###

    def handleClockError(self):
        self.synchronize("clock error")

    ### Synchronization methods ###

    def synchronize(self, description):
        """synchronize(self, string description)

        Call this function from time to time to synchronize watches
        with the server.  This initiates a round-trip transaction;
        when the transaction completes, the time will be synced.

        The description is the string that will be written to the log
        file regarding the reason for this synchronization attempt.

        The return value is true if the attempt is made, or false if
        it is too soon since the last attempt.
        """
        now = globalClock.getRealTime()

        if now - self.lastAttempt < self.minWait:
            self.notify.debug("Not resyncing (too soon): %s" % (description))
            return 0

        self.talkResult = 0
        self.thisContext = self.nextContext
        self.attemptCount = 0
        self.nextContext = (self.nextContext + 1) & 255
        self.notify.info("Clock sync: %s" % (description))
        self.start = now
        self.lastAttempt = now
        self.sendUpdate("requestServerTime", [self.thisContext])

        return 1


    def serverTime(self, context, timestamp):
        """serverTime(self, int8 context, int32 timestamp)

        This message is sent from the AI to the client in response to
        a previous requestServerTime.  It contains the time as
        observed by the AI.

        The client should use this, in conjunction with the time
        measurement taken before calling requestServerTime (above), to
        determine the clock delta between the AI and the client
        machines.
        """
        end = globalClock.getRealTime()

        if context != self.thisContext:
            self.notify.info("Ignoring TimeManager response for old context %d" % (context))
            return

        elapsed = end - self.start
        self.attemptCount += 1
        self.notify.info("Clock sync roundtrip took %0.3f ms" % (elapsed * 1000.0))

        average = (self.start + end) / 2.0 - self.extraSkew
        uncertainty = (end - self.start) / 2.0 + abs(self.extraSkew)

        globalClockDelta.resynchronize(average, timestamp, uncertainty)

        self.notify.info("Local clock uncertainty +/- %.3f s" % (globalClockDelta.getUncertainty()))

        if globalClockDelta.getUncertainty() > self.maxUncertainty:
            if self.attemptCount < self.maxAttempts:
                self.notify.info("Uncertainty is too high, trying again.")
                self.start = globalClock.getRealTime()
                self.sendUpdate("requestServerTime", [self.thisContext])
                return
            self.notify.info("Giving up on uncertainty requirement.")

        messenger.send("gotTimeSync", taskChain = 'default')
        messenger.send(self.cr.uniqueName("gotTimeSync"), taskChain = 'default')

