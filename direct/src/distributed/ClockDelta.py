# ClockDelta provides the ability to use clock synchronization for
# distributed objects

from pandac.PandaModules import *
from direct.directnotify import DirectNotifyGlobal
from direct.showbase import DirectObject
import math

# The following two parameters, NetworkTimeBits and
# NetworkTimePrecision, define the number of bits required to store a
# network time, and the number of ticks per second it represents,
# respectively.  The tradeoff is the longest period of elapsed time we
# can measure, vs. the precision with which we can measure it.

# 16 and 100 give us precision to 1/100th of a second, with a range of
# +/- 5 minutes in a 16-bit integer.  These are eminently tweakable,
# but the parameter types in toon.dc must match the number of bits
# specified here (i.e. int16 if NetworkTimeBits is 16; int32 if
# NetworkTimeBits is 32).
NetworkTimeBits = 16
NetworkTimePrecision = 100.0


# These values are derived from the above.
NetworkTimeMask = (1 << NetworkTimeBits) - 1
NetworkTimeSignedMask = NetworkTimeMask >> 1 # the max absolute value bits.
NetworkTimeTopBits = 32 - NetworkTimeBits
MaxTimeDelta = NetworkTimeSignedMask / NetworkTimePrecision

# This is the maximum number of seconds by which we expect our clock
# (or the server's clock) to drift over an hour.
ClockDriftPerHour = 1.0   # Is this generous enough?

# And the above, scaled into a per-second value.
ClockDriftPerSecond = ClockDriftPerHour / 3600.0

# How many seconds to insist on waiting before accepting a second
# resync request from another client.
P2PResyncDelay = 10.0

class ClockDelta(DirectObject.DirectObject):
    """
    The ClockDelta object converts between universal ("network") time,
    which is used for all network traffic, and local time (e.g. as
    returned by getFrameTime() or getRealTime()), which is used for
    everything else.
    """

    notify = DirectNotifyGlobal.directNotify.newCategory('ClockDelta')

    def __init__(self):
        self.globalClock = ClockObject.getGlobalClock()

        # self.delta is the relative delta from our clock to the
        # server's clock.
        self.delta = 0

        # self.uncertainty represents the number of seconds plus or
        # minus in which we are confident our delta matches the
        # server's actual time.  The initial value, None, represents
        # infinity--we have no idea.
        self.uncertainty = None

        # self.lastResync is the time at which self.uncertainty
        # was measured.  It is important to remember because our
        # uncertainty increases over time (due to relative clock
        # drift).
        self.lastResync = 0.0

        self.accept("resetClock", self.__resetClock)

    def getDelta(self):
        return self.delta

    def getUncertainty(self):
        # Returns our current uncertainty with our clock measurement,
        # as a number of seconds plus or minus.  Returns None,
        # representing infinite uncertainty, if we have never received
        # a time measurement.

        if self.uncertainty == None:
            return None

        now = self.globalClock.getRealTime()
        elapsed = now - self.lastResync
        return self.uncertainty + elapsed * ClockDriftPerSecond

    def getLastResync(self):
        # Returns the local time at which we last resynchronized the
        # clock delta.
        return self.lastResync

    def __resetClock(self, timeDelta):
        """
        this is called when the global clock gets adjusted
        timeDelta is equal to the amount of time, in seconds,
        that has been added to the global clock
        """
        assert self.notify.debug(
            "adjusting timebase by %f seconds" % timeDelta)
        # adjust our timebase by the same amount
        self.delta += timeDelta

    def clear(self):
        """
        Throws away any previous synchronization information.
        """
        self.delta = 0
        self.uncertainty = None
        self.lastResync = 0.0

    def resynchronize(self, localTime, networkTime, newUncertainty,
                      trustNew = 1):
        """resynchronize(self, float localTime, int32 networkTime,
                         float newUncertainty)

        Accepts a new networkTime value, which is understood to
        represent the same moment as localTime, plus or minus
        uncertainty seconds.  Improves our current notion of the time
        delta accordingly.
        """
        newDelta = (float(localTime) -
            (float(networkTime) / NetworkTimePrecision))
        self.newDelta(
            localTime, newDelta, newUncertainty, trustNew = trustNew)

    def peerToPeerResync(self, avId, timestamp, serverTime, uncertainty):
        """
        Accepts an AI time and uncertainty value from another client,
        along with a local timestamp value of the message from this
        client which prompted the other client to send us its delta
        information.

        The return value is true if the other client's measurement was
        reasonably close to our own, or false if the other client's
        time estimate was wildly divergent from our own; the return
        value is negative if the test was not even considered (because
        it happened too soon after another recent request).
        """

        now = self.globalClock.getRealTime()
        if now - self.lastResync < P2PResyncDelay:
            # We can't process this request; it came in on the heels
            # of some other request, and our local timestamp may have
            # been resynced since then: ergo, the timestamp in this
            # request is meaningless.
            assert self.notify.debug(
                "Ignoring request for resync from %s within %.3f s." %
                (avId, now - self.lastResync))
            return -1

        # The timestamp value will be a timestamp that we sent out
        # previously, echoed back to us.  Therefore we can confidently
        # convert it back into our local time, even though we suspect
        # our clock delta might be off.
        local = self.networkToLocalTime(timestamp, now)
        elapsed = now - local
        delta = (local + now) / 2.0 - serverTime

        gotSync = 0
        if elapsed <= 0 or elapsed > P2PResyncDelay:
            # The elapsed time must be positive (the local timestamp
            # must be in the past), and shouldn't be more than
            # P2PResyncDelay.  If it does not meet these requirements,
            # it must be very old indeed, or someone is playing tricks
            # on us.
            self.notify.info(
                "Ignoring old request for resync from %s." % (avId))
        else:
            # Now the other client has told us his delta and uncertainty
            # information, which was generated somewhere in the range
            # [-elapsed, 0] seconds ago.  That means our complete window
            # is wider by that amount.
            self.notify.info(
                "Got sync +/- %.3f s, elapsed %.3f s, from %s." %
                (uncertainty, elapsed, avId))
            delta -= elapsed / 2.0
            uncertainty += elapsed / 2.0

            gotSync = self.newDelta(local, delta, uncertainty, trustNew = 0)

        return gotSync

    def newDelta(self, localTime, newDelta, newUncertainty,
                 trustNew = 1):
        """
        Accepts a new delta and uncertainty pair, understood to
        represent time as of localTime.  Improves our current notion
        of the time delta accordingly.  The return value is true if
        the new measurement was used, false if it was discarded.
        """
        oldUncertainty = self.getUncertainty()
        if oldUncertainty != None:
            self.notify.info(
                'previous delta at %.3f s, +/- %.3f s.' % 
                (self.delta, oldUncertainty))
            self.notify.info(
                'new delta at %.3f s, +/- %.3f s.' %
                (newDelta, newUncertainty))
            # Our previous measurement was self.delta +/- oldUncertainty;
            # our new measurement is newDelta +/- newUncertainty.  Take
            # the intersection of both.

            oldLow = self.delta - oldUncertainty
            oldHigh = self.delta + oldUncertainty
            newLow = newDelta - newUncertainty
            newHigh = newDelta + newUncertainty

            low = max(oldLow, newLow)
            high = min(oldHigh, newHigh)

            # If there is no intersection, whoops!  Either the old
            # measurement or the new measurement is completely wrong.
            if low > high:
                if not trustNew:
                    self.notify.info('discarding new delta.')
                    return 0

                self.notify.info('discarding previous delta.')
            else:
                newDelta = (low + high) / 2.0
                newUncertainty = (high - low) / 2.0
                self.notify.info(
                    'intersection at %.3f s, +/- %.3f s.' %
                    (newDelta, newUncertainty))

        self.delta = newDelta
        self.uncertainty = newUncertainty
        self.lastResync = localTime

        return 1

    ### Primary interface functions ###

    def networkToLocalTime(self, networkTime, now = None, bits = 16,
                           ticksPerSec=NetworkTimePrecision):
        """networkToLocalTime(self, int networkTime)

        Converts the indicated networkTime to the corresponding
        localTime value.  The time is assumed to be within +/- 5
        minutes of the current local time given in now, or
        getRealTime() if now is not specified.
        """
        if now == None:
            now = self.globalClock.getRealTime()

        # Are we in non-real-time mode (i.e. filming a movie)?  If you
        # set movie-network-time 1, then we'll circumvent this logic
        # and always return now.
        if self.globalClock.getMode() == ClockObject.MNonRealTime and \
           base.config.GetBool('movie-network-time', False):
            return now

        # First, determine what network time we have for 'now'.
        ntime = int(math.floor(((now - self.delta) * ticksPerSec) + 0.5))

        # The signed difference between these is the number of ticks
        # by which the network time differs from 'now'.
        if bits == 16:
            diff = self.__signExtend(networkTime - ntime)
        else:
            # Assume the bits is either 16 or 32.  If it's 32, no need
            # to sign-extend.  32 bits gives us about 227 days of
            # continuous timestamp.

            diff = networkTime - ntime

        return now + float(diff) / ticksPerSec

    def localToNetworkTime(self, localTime, bits = 16,
                           ticksPerSec=NetworkTimePrecision):
        """localToNetworkTime(self, float localTime)

        Converts the indicated localTime to the corresponding
        networkTime value.
        """
        ntime = int(math.floor(((localTime - self.delta) * ticksPerSec) + 0.5))
        if bits == 16:
            return self.__signExtend(ntime)
        else:
            # Assume the bits is either 16 or 32.  If it's 32, no need
            # to sign-extend.  32 bits gives us about 227 days of
            # continuous timestamp.
            return ntime


    ### Convenience functions ###

    def getRealNetworkTime(self, bits=16,
                           ticksPerSec=NetworkTimePrecision):
        """
        Returns the current getRealTime() expressed as a network time.
        """
        return self.localToNetworkTime(self.globalClock.getRealTime(),
                                       bits=bits,
                                       ticksPerSec=ticksPerSec)

    def getFrameNetworkTime(self, bits=16,
                            ticksPerSec=NetworkTimePrecision):
        """
        Returns the current getFrameTime() expressed as a network time.
        """
        return self.localToNetworkTime(self.globalClock.getFrameTime(),
                                       bits=bits,
                                       ticksPerSec=ticksPerSec)

    def localElapsedTime(self, networkTime, bits=16,
                         ticksPerSec=NetworkTimePrecision):
        """localElapsedTime(self, int networkTime)

        Returns the amount of time elapsed (in seconds) on the client
        since the server message was sent.  Negative values are
        clamped to zero.
        """
        now = self.globalClock.getFrameTime()
        dt = now - self.networkToLocalTime(networkTime, now, bits=bits,
                                           ticksPerSec=ticksPerSec)

        return max(dt, 0.0)

    ### Private functions ###

    def __signExtend(self, networkTime):
        """__signExtend(self, int networkTime)

        Preserves the lower NetworkTimeBits of the networkTime value,
        and extends the sign bit all the way up.
        """
        r = ((networkTime+32768) & NetworkTimeMask) - 32768
        assert -32768 <= r <= 32767
        return r


globalClockDelta = ClockDelta()
