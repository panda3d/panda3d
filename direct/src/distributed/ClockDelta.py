# ClockDelta provides the ability to use clock synchronization for
# distributed objects

from PandaModules import *
import DirectNotifyGlobal
import DirectObject
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
NetworkTimeTopBits = 32 - NetworkTimeBits

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

        self.delta = 0
        self.accept("resetClock", self.__resetClock)

    def __resetClock(self, timeDelta):
        """
        this is called when the global clock gets adjusted
        timeDelta is equal to the amount of time, in seconds,
        that has been added to the global clock
        """
        self.notify.debug("adjusting timebase by %f seconds" % timeDelta)
        # adjust our timebase by the same amount
        self.delta += timeDelta

    def resynchronize(self, localTime, networkTime):
        """resynchronize(self, float localTime, uint32 networkTime)

        Resets the relative delta so that the indicated networkTime
        and localTime map to the same instant.  The return value is
        the amount by which the clock changes, in seconds.
        """
        newDelta = (float(localTime) -
                    (float(networkTime) / NetworkTimePrecision))
        change = newDelta - self.delta
        self.delta = newDelta

        return change


    ### Primary interface functions ###

    def networkToLocalTime(self, networkTime, now = None, bits = 16):
        """networkToLocalTime(self, int networkTime)

        Converts the indicated networkTime to the corresponding
        localTime value.  The time is assumed to be within +/- 5
        minutes of the current local time given in now, or
        getRealTime() if now is not specified.
        """
        if now == None:
            now = self.globalClock.getRealTime()

        # Are we in non-real-time mode (i.e. filming a movie)?  Just return now
        if self.globalClock.getMode() == ClockObject.MNonRealTime:
            return now

        # First, determine what network time we have for 'now'.
        ntime = int(math.floor((now - self.delta) * NetworkTimePrecision + 0.5))

        # The signed difference between these is the number of units
        # of NetworkTimePrecision by which the network time differs
        # from 'now'.
        if bits == 16:
            diff = self.__signExtend(networkTime - ntime)

        else:
            # Assume the bits is either 16 or 32.  If it's 32, no need
            # to sign-extend.  32 bits gives us about 227 days of
            # continuous timestamp.
            
            diff = networkTime - ntime
        
        return now + float(diff) / NetworkTimePrecision

    def localToNetworkTime(self, localTime, bits = 16):
        """localToNetworkTime(self, float localTime)

        Converts the indicated localTime to the corresponding
        networkTime value.
        """
        ntime = int(math.floor((localTime - self.delta) * NetworkTimePrecision + 0.5))
        if bits == 16:
            return self.__signExtend(ntime)

        else:
            # Assume the bits is either 16 or 32.  If it's 32, no need
            # to sign-extend.  32 bits gives us about 227 days of
            # continuous timestamp.
            return ntime


    ### Convenience functions ###

    def getRealNetworkTime(self, bits=16):
        """getRealNetworkTime(self)

        Returns the current getRealTime() expressed as a network time.
        """
        return self.localToNetworkTime(self.globalClock.getRealTime(),
                                       bits=bits)

    def getFrameNetworkTime(self, bits=16):
        """getFrameNetworkTime(self)

        Returns the current getFrameTime() expressed as a network time.
        """
        return self.localToNetworkTime(self.globalClock.getFrameTime(),
                                       bits=bits)

    def localElapsedTime(self, networkTime, bits=16):
        """localElapsedTime(self, int networkTime)

        Returns the amount of time elapsed (in seconds) on the client
        since the server message was sent.  Negative values are
        clamped to zero.
        
        """
        now = self.globalClock.getFrameTime()
        dt = now - self.networkToLocalTime(networkTime, now, bits=bits)

        if (dt >= 0.0):
            return dt
        else:
            self.notify.debug('negative clock delta: %.3f' % dt)
            return 0.0



    ### Private functions ###

    def __signExtend(self, networkTime):
        """__signExtend(self, int networkTime)

        Preserves the lower NetworkTimeBits of the networkTime value,
        and extends the sign bit all the way up.
        """

        return ((networkTime & NetworkTimeMask) << NetworkTimeTopBits) >> NetworkTimeTopBits

globalClockDelta = ClockDelta()
