"""
This module imports all of the other interval modules, to provide a
single convenient module from which all interval types can be imported.
"""

# In this unusual case, I'm not going to declare __all__,
# since the purpose of this module is to add up the contributions
# of a number of other modules.

from .Interval import *
from .ActorInterval import *
from .FunctionInterval import *
from .LerpInterval import *
from .IndirectInterval import *
from .MopathInterval import *
try:
    import panda3d.physics
    ##Some people may have the particle system compiled out
    if hasattr( panda3d.physics, 'ParticleSystem' ):
        from .ParticleInterval import *
        if __debug__:
            from .TestInterval import *
except ImportError:
    pass
from .SoundInterval import *
from .ProjectileInterval import *
from .MetaInterval import *
from .IntervalManager import *
from panda3d.direct import WaitInterval
