"""DistributedSmoothNode module: contains the DistributedSmoothNode class"""

from panda3d.core import *
from panda3d.direct import *
from .ClockDelta import *
from . import DistributedNode
from . import DistributedSmoothNodeBase
from direct.task.Task import cont
from direct.showbase import DConfig as config

# This number defines our tolerance for out-of-sync telemetry packets.
# If a packet appears to have originated from more than MaxFuture
# seconds in the future, assume we're out of sync with the other
# avatar and suggest a resync for both.
MaxFuture = config.GetFloat("smooth-max-future", 0.2)

# How frequently can we suggest a resynchronize with another client?
MinSuggestResync = config.GetFloat("smooth-min-suggest-resync", 15)

# These flags indicate whether global smoothing and/or prediction is
# allowed or disallowed.
EnableSmoothing = config.GetBool("smooth-enable-smoothing", 1)
EnablePrediction = config.GetBool("smooth-enable-prediction", 1)

# These values represent the amount of time, in seconds, to delay the
# apparent position of other avatars, when non-predictive and
# predictive smoothing is in effect, respectively.  This is in
# addition to the automatic delay of the observed average latency from
# each avatar, which is intended to compensate for relative clock
# skew.
Lag = config.GetDouble("smooth-lag", 0.2)
PredictionLag = config.GetDouble("smooth-prediction-lag", 0.0)


GlobalSmoothing = 0
GlobalPrediction = 0
def globalActivateSmoothing(smoothing, prediction):
    """ Globally activates or deactivates smoothing and prediction on
    all DistributedSmoothNodes currently in existence, or yet to be
    generated. """

    global GlobalSmoothing, GlobalPrediction
    GlobalSmoothing = smoothing
    GlobalPrediction = prediction

    for obj in base.cr.getAllOfType(DistributedSmoothNode):
        obj.activateSmoothing(smoothing, prediction)

# For historical reasons, we temporarily define
# DistributedSmoothNode.activateSmoothing() to be the global function.
# We'll remove this soon, so it won't get confused with the instance
# method, below.
activateSmoothing = globalActivateSmoothing


class DistributedSmoothNode(DistributedNode.DistributedNode,
                            DistributedSmoothNodeBase.DistributedSmoothNodeBase):
    """
    This specializes DistributedNode to add functionality to smooth
    motion over time, via the SmoothMover C++ object defined in
    DIRECT.
    """

    def __init__(self, cr):
        try:
            self.DistributedSmoothNode_initialized
        except:
            self.DistributedSmoothNode_initialized = 1
            DistributedNode.DistributedNode.__init__(self, cr)
            DistributedSmoothNodeBase.DistributedSmoothNodeBase.__init__(self)
            self.smoothStarted = 0

            # Set this True to assert that the local process has
            # complete authority over the position of this object when
            # smoothing is not in effect.  When this is True, position
            # reports received over the wire will not be applied to
            # this node's position, unless those position reports are
            # received between startSmooth() and endSmooth().
            self.localControl = False

            # flag set when we receive a stop message
            self.stopped = False

    def generate(self):
        self.smoother = SmoothMover()
        self.smoothStarted = 0
        self.lastSuggestResync = 0
        self._smoothWrtReparents = False

        DistributedNode.DistributedNode.generate(self)
        DistributedSmoothNodeBase.DistributedSmoothNodeBase.generate(self)
        self.cnode.setRepository(self.cr, 0, 0)

        self.activateSmoothing(GlobalSmoothing, GlobalPrediction)

        # clear stopped flag for re-generate
        self.stopped = False

    def disable(self):
        DistributedSmoothNodeBase.DistributedSmoothNodeBase.disable(self)
        DistributedNode.DistributedNode.disable(self)
        del self.smoother

    def delete(self):
        DistributedSmoothNodeBase.DistributedSmoothNodeBase.delete(self)
        DistributedNode.DistributedNode.delete(self)

    ### Methods to handle computing and updating of the smoothed
    ### position.

    def smoothPosition(self):
        """
        This function updates the position of the node to its computed
        smoothed position.  This may be overridden by a derived class
        to specialize the behavior.
        """
        self.smoother.computeAndApplySmoothPosHpr(self, self)

    def doSmoothTask(self, task):
        self.smoothPosition()
        return cont

    def wantsSmoothing(self):
        # Override this function to return 0 if this particular kind
        # of smooth node doesn't really want to be smoothed.
        return 1

    def startSmooth(self):
        """
        This function starts the task that ensures the node is
        positioned correctly every frame.  However, while the task is
        running, you won't be able to lerp the node or directly
        position it.
        """
        if not self.wantsSmoothing() or self.isDisabled() or self.isLocal():
            return
        if not self.smoothStarted:
            taskName = self.taskName("smooth")
            taskMgr.remove(taskName)
            self.reloadPosition()
            taskMgr.add(self.doSmoothTask, taskName)
            self.smoothStarted = 1

    def stopSmooth(self):
        """
        This function stops the task spawned by startSmooth(), and
        allows show code to move the node around directly.
        """
        if self.smoothStarted:
            taskName = self.taskName("smooth")
            taskMgr.remove(taskName)
            self.forceToTruePosition()
            self.smoothStarted = 0

    def setSmoothWrtReparents(self, flag):
        self._smoothWrtReparents = flag
    def getSmoothWrtReparents(self):
        return self._smoothWrtReparents

    def forceToTruePosition(self):
        """
        This forces the node to reposition itself to its latest known
        position.  This may result in a pop as the node skips the last
        of its lerp points.
        """
        #printStack()
        if (not self.isLocal()) and \
           self.smoother.getLatestPosition():
            self.smoother.applySmoothPosHpr(self, self)
        self.smoother.clearPositions(1)

    def reloadPosition(self):
        """
        This function re-reads the position from the node itself and
        clears any old position reports for the node.  This should be
        used whenever show code bangs on the node position and expects
        it to stick.
        """
        self.smoother.clearPositions(0)
        self.smoother.setPosHpr(self.getPos(), self.getHpr())
        self.smoother.setPhonyTimestamp()
        self.smoother.markPosition()

    def _checkResume(self,timestamp):
        """
        Determine if we were previously stopped and now need to
        resume movement by making sure any old stored positions
        reflect the node's current position
        """
        if (self.stopped):
            currTime = globalClock.getFrameTime()
            now = currTime - self.smoother.getExpectedBroadcastPeriod()
            last = self.smoother.getMostRecentTimestamp()
            if (now > last):
                # only set a new timestamp postion if we still have
                # a position being smoothed to (so we don't interrupt
                # any current smoothing and only do this if the object
                # is actually locally stopped)
                if (timestamp == None):
                    # no timestamp, use current time
                    local = 0.0
                else:
                    local = globalClockDelta.networkToLocalTime(
                        timestamp, currTime)

                self.smoother.setPhonyTimestamp(local,True)
                self.smoother.markPosition()

        self.stopped = False

    # distributed set pos and hpr functions
    # 'send' versions are inherited from DistributedSmoothNodeBase
    def setSmStop(self, timestamp=None):
        self.setComponentTLive(timestamp)
        self.stopped = True
    def setSmH(self, h, timestamp=None):
        self._checkResume(timestamp)
        self.setComponentH(h)
        self.setComponentTLive(timestamp)
    def setSmZ(self, z, timestamp=None):
        self._checkResume(timestamp)
        self.setComponentZ(z)
        self.setComponentTLive(timestamp)
    def setSmXY(self, x, y, timestamp=None):
        self._checkResume(timestamp)
        self.setComponentX(x)
        self.setComponentY(y)
        self.setComponentTLive(timestamp)
    def setSmXZ(self, x, z, timestamp=None):
        self._checkResume(timestamp)
        self.setComponentX(x)
        self.setComponentZ(z)
        self.setComponentTLive(timestamp)
    def setSmPos(self, x, y, z, timestamp=None):
        self._checkResume(timestamp)
        self.setComponentX(x)
        self.setComponentY(y)
        self.setComponentZ(z)
        self.setComponentTLive(timestamp)
    def setSmHpr(self, h, p, r, timestamp=None):
        self._checkResume(timestamp)
        self.setComponentH(h)
        self.setComponentP(p)
        self.setComponentR(r)
        self.setComponentTLive(timestamp)
    def setSmXYH(self, x, y, h, timestamp):
        self._checkResume(timestamp)
        self.setComponentX(x)
        self.setComponentY(y)
        self.setComponentH(h)
        self.setComponentTLive(timestamp)
    def setSmXYZH(self, x, y, z, h, timestamp=None):
        self._checkResume(timestamp)
        self.setComponentX(x)
        self.setComponentY(y)
        self.setComponentZ(z)
        self.setComponentH(h)
        self.setComponentTLive(timestamp)
    def setSmPosHpr(self, x, y, z, h, p, r, timestamp=None):
        self._checkResume(timestamp)
        self.setComponentX(x)
        self.setComponentY(y)
        self.setComponentZ(z)
        self.setComponentH(h)
        self.setComponentP(p)
        self.setComponentR(r)
        self.setComponentTLive(timestamp)

    def setSmPosHprL(self, l, x, y, z, h, p, r, timestamp=None):
        self._checkResume(timestamp)
        self.setComponentL(l)
        self.setComponentX(x)
        self.setComponentY(y)
        self.setComponentZ(z)
        self.setComponentH(h)
        self.setComponentP(p)
        self.setComponentR(r)
        self.setComponentTLive(timestamp)

    ### component set pos and hpr functions ###

    ### These are the component functions that are invoked
    ### remotely by the above composite functions.

    @report(types = ['args'], dConfigParam = 'smoothnode')
    def setComponentX(self, x):
        self.smoother.setX(x)
    @report(types = ['args'], dConfigParam = 'smoothnode')
    def setComponentY(self, y):
        self.smoother.setY(y)
    @report(types = ['args'], dConfigParam = 'smoothnode')
    def setComponentZ(self, z):
        self.smoother.setZ(z)
    @report(types = ['args'], dConfigParam = 'smoothnode')
    def setComponentH(self, h):
        self.smoother.setH(h)
    @report(types = ['args'], dConfigParam = 'smoothnode')
    def setComponentP(self, p):
        self.smoother.setP(p)
    @report(types = ['args'], dConfigParam = 'smoothnode')
    def setComponentR(self, r):
        self.smoother.setR(r)
    @report(types = ['args'], dConfigParam = 'smoothnode')
    def setComponentL(self, l):
        if (l != self.zoneId):
            # only perform set location if location is different
            self.setLocation(self.parentId,l)
    @report(types = ['args'], dConfigParam = 'smoothnode')
    def setComponentT(self, timestamp):
        # This is a little bit hacky.  If *this* function is called,
        # it must have been called directly by the server, for
        # instance to update the values previously set for some avatar
        # that was already into the zone as we entered.  (A live
        # update would have gone through the function called
        # setComponentTLive, below.)

        # Since we know this update came through the server, it may
        # reflect very old data.  Thus, we can't accurately decode the
        # network timestamp (since the network time encoding can only
        # represent a time up to about 5 minutes in the past), but we
        # don't really need to know the timestamp anyway.  We'll just
        # arbitrarily place it at right now.
        self.smoother.setPhonyTimestamp()
        self.smoother.clearPositions(1)
        self.smoother.markPosition()

        # mark position only takes most recent position sent over the wire
        # and applies it to the smoother's sample points, but we still
        # need to make sure and apply that position to the actual node
        # path
        self.forceToTruePosition()

    @report(types = ['args'], dConfigParam = 'smoothnode')
    def setComponentTLive(self, timestamp):
        # This is the variant of setComponentT() that will be called
        # whenever we receive a live update directly from the other
        # client.  This is because the component functions, above,
        # call this function explicitly instead of setComponentT().

        #print 'setComponentTLive: %s' % timestamp

        if timestamp is None:
            # if no timestamp, re-use the most recent timestamp to keep things
            # from getting out of order
            if self.smoother.hasMostRecentTimestamp():
                self.smoother.setTimestamp(self.smoother.getMostRecentTimestamp())
            else:
                # no most-recent timestamp, use current time
                self.smoother.setPhonyTimestamp()
            self.smoother.markPosition()
        else:
            now = globalClock.getFrameTime()
            local = globalClockDelta.networkToLocalTime(timestamp, now)
            realTime = globalClock.getRealTime()
            chug = realTime - now

            # Sanity check the timestamp from the other avatar.  It should
            # be just slightly in the past, but it might be off by as much
            # as this frame's amount of time forward or back.
            howFarFuture = local - now
            if howFarFuture - chug >= MaxFuture:
                # Too far off; advise the other client of our clock information.
                if globalClockDelta.getUncertainty() != None and \
                   realTime - self.lastSuggestResync >= MinSuggestResync and \
                   hasattr(self.cr, 'localAvatarDoId'):
                    self.lastSuggestResync = realTime
                    timestampB = globalClockDelta.localToNetworkTime(realTime)
                    serverTime = realTime - globalClockDelta.getDelta()
                    assert self.notify.info(
                        "Suggesting resync for %s, with discrepency %s; local time is %s and server time is %s." % (
                        self.doId, howFarFuture - chug,
                        realTime, serverTime))
                    self.d_suggestResync(
                        self.cr.localAvatarDoId, timestamp,
                        timestampB, serverTime,
                        globalClockDelta.getUncertainty())

            self.smoother.setTimestamp(local)
            self.smoother.markPosition()

        if not self.localControl and not self.smoothStarted and \
           self.smoother.getLatestPosition():
            self.smoother.applySmoothPosHpr(self, self)

    # These are all required by the CMU server, which requires get* to
    # match set* in more cases than the Disney server does.
    def getComponentL(self):
        return self.zoneId
    def getComponentX(self):
        return self.getX()
    def getComponentY(self):
        return self.getY()
    def getComponentZ(self):
        return self.getZ()
    def getComponentH(self):
        return self.getH()
    def getComponentP(self):
        return self.getP()
    def getComponentR(self):
        return self.getR()
    def getComponentT(self):
        return 0

    @report(types = ['args'], dConfigParam = 'smoothnode')
    def clearSmoothing(self, bogus = None):
        # Call this to invalidate all the old position reports
        # (e.g. just before popping to a new position).
        #printStack()
        self.smoother.clearPositions(1)


    @report(types = ['args'], dConfigParam = 'smoothnode')
    def wrtReparentTo(self, parent):
        # We override this NodePath method to force it to
        # automatically reset the smoothing position when we call it.
        if self.smoothStarted:
            if self._smoothWrtReparents:
                #print self.getParent(), parent, self.getParent().getPos(parent)
                self.smoother.handleWrtReparent(self.getParent(), parent)
                NodePath.wrtReparentTo(self, parent)
            else:
                self.forceToTruePosition()
                NodePath.wrtReparentTo(self, parent)
                self.reloadPosition()
        else:
            NodePath.wrtReparentTo(self, parent)

    @report(types = ['args'], dConfigParam = 'smoothnode')
    def d_setParent(self, parentToken):
        # We override this DistributedNode method to force a full position
        # update immediately after the distributed setParent is sent.
        # See ParentMgr.py for an explanation.
        DistributedNode.DistributedNode.d_setParent(self, parentToken)

        self.forceToTruePosition()
        self.sendCurrentPosition()

    ### Monitor clock sync ###

    def d_suggestResync(self, avId, timestampA, timestampB,
                        serverTime, uncertainty):
        serverTimeSec = math.floor(serverTime)
        serverTimeUSec = (serverTime - serverTimeSec) * 10000.0
        self.sendUpdate("suggestResync", [avId, timestampA, timestampB,
                                          serverTimeSec, serverTimeUSec,
                                          uncertainty])

    def suggestResync(self, avId, timestampA, timestampB,
                      serverTimeSec, serverTimeUSec, uncertainty):
        """
        This message is sent from one client to another when the other
        client receives a timestamp from this client that is so far
        out of date as to suggest that one or both clients needs to
        resynchronize their clock information.
        """
        serverTime = float(serverTimeSec) + float(serverTimeUSec) / 10000.0
        result = self.peerToPeerResync(
            avId, timestampA, serverTime, uncertainty)
        if result >= 0 and \
           globalClockDelta.getUncertainty() != None:
            other = self.cr.doId2do.get(avId)
            if (not other):
                assert self.notify.info(
                    "Warning: couldn't find the avatar %d" % (avId))
            elif hasattr(other, "d_returnResync") and \
                 hasattr(self.cr, 'localAvatarDoId'):
                realTime = globalClock.getRealTime()
                serverTime = realTime - globalClockDelta.getDelta()
                assert self.notify.info(
                    "Returning resync for %s; local time is %s and server time is %s." % (
                    self.doId, realTime, serverTime))
                other.d_returnResync(
                    self.cr.localAvatarDoId, timestampB,
                    serverTime,
                    globalClockDelta.getUncertainty())


    def d_returnResync(self, avId, timestampB, serverTime, uncertainty):
        serverTimeSec = math.floor(serverTime)
        serverTimeUSec = (serverTime - serverTimeSec) * 10000.0
        self.sendUpdate("returnResync", [
            avId, timestampB, serverTimeSec, serverTimeUSec, uncertainty])

    def returnResync(self, avId, timestampB, serverTimeSec, serverTimeUSec,
            uncertainty):
        """
        A reply sent by a client whom we recently sent suggestResync
        to, this reports the client's new delta information so we can
        adjust our clock as well.
        """
        serverTime = float(serverTimeSec) + float(serverTimeUSec) / 10000.0
        self.peerToPeerResync(avId, timestampB, serverTime, uncertainty)

    def peerToPeerResync(self, avId, timestamp, serverTime, uncertainty):
        gotSync = globalClockDelta.peerToPeerResync(
            avId, timestamp, serverTime, uncertainty)

        # If we didn't get anything useful from the other client,
        # maybe our clock is just completely hosed.  Go ask the AI.
        if not gotSync:
            if self.cr.timeManager != None:
                self.cr.timeManager.synchronize("suggested by %d" % (avId))

        return gotSync

    def activateSmoothing(self, smoothing, prediction):
        """
        Enables or disables the smoothing of other avatars' motion.
        This used to be a global flag, but now it is specific to each
        avatar instance.  However, see globalActivateSmoothing() in
        this module.

        If smoothing is off, no kind of smoothing will be performed,
        regardless of the setting of prediction.

        This is not necessarily predictive smoothing; if predictive
        smoothing is off, avatars will be lagged by a certain factor
        to achieve smooth motion.  Otherwise, if predictive smoothing
        is on, avatars will be drawn as nearly as possible in their
        current position, by extrapolating from old position reports.

        This assumes you have a client repository that knows its
        localAvatarDoId -- stored in self.cr.localAvatarDoId
        """
        if smoothing and EnableSmoothing:
            if prediction and EnablePrediction:
                # Prediction and smoothing.
                self.smoother.setSmoothMode(SmoothMover.SMOn)
                self.smoother.setPredictionMode(SmoothMover.PMOn)
                self.smoother.setDelay(PredictionLag)
            else:
                # Smoothing, but no prediction.
                self.smoother.setSmoothMode(SmoothMover.SMOn)
                self.smoother.setPredictionMode(SmoothMover.PMOff)
                self.smoother.setDelay(Lag)
        else:
            # No smoothing, no prediction.
            self.smoother.setSmoothMode(SmoothMover.SMOff)
            self.smoother.setPredictionMode(SmoothMover.PMOff)
            self.smoother.setDelay(0.0)
