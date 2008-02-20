"""IntervalGlobal module"""

# In this unusual case, I'm not going to declare __all__,
# since the purpose of this module is to add up the contributions
# of a number of other modules.

from Interval import *
from ActorInterval import *
from FunctionInterval import *
from LerpInterval import *
from IndirectInterval import *
from MopathInterval import *
from ParticleInterval import *
from SoundInterval import *
from ProjectileInterval import *
from MetaInterval import *
from IntervalManager import *
if __debug__:
    from TestInterval import *
from pandac.PandaModules import WaitInterval

# there is legacy code in Toontown that puts interval-related DelayDeletes directly
# on the interval object, relying on Python to __del__ the DelayDelete when the interval
# is garbage-collected. DelayDelete now requires .destroy() to be called.
# To reduce code duplication, this method may be called to clean up delayDeletes that
# have been placed on an interval.
def cleanupDelayDeletes(interval):
    if hasattr(interval, 'delayDelete'):
        delayDelete = interval.delayDelete
        # get rid of the reference before calling destroy in case destroy causes
        # this function to be called again
        del interval.delayDelete
        delayDelete.destroy()
    if hasattr(interval, 'delayDeletes'):
        delayDeletes = interval.delayDeletes
        # get rid of the reference before calling destroy in case destroy causes
        # this function to be called again
        del interval.delayDeletes
        for i in delayDeletes:
            i.destroy()
