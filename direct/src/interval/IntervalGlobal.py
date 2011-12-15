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
import pandac.PandaModules
##Some people may have the particle system compiled out
if hasattr( pandac.PandaModules, 'ParticleSystem' ):
    from ParticleInterval import *
from SoundInterval import *
from ProjectileInterval import *
from MetaInterval import *
from IntervalManager import *
if __debug__:
    if hasattr( pandac.PandaModules, 'ParticleSystem' ):
        from TestInterval import *
from pandac.PandaModules import WaitInterval
