"""instantiate global ShowBase object"""

from ShowBase import *
import __builtin__

__builtin__.base = ShowBase()

# Make some global aliases for convenience
__builtin__.ostream = Notify.out()
__builtin__.directNotify = directNotify

# Set direct notify categories now that we have config
directNotify.setDconfigLevels()

def inspect(anObject):
    import Inspector
    return Inspector.inspect(anObject)

__builtin__.inspect = inspect

