"""instantiate global ShowBase object"""

from ShowBase import *

# Create the showbase instance
ShowBase()

# Set direct notify categories now that we have config
directNotify.setDconfigLevels()

def inspect(anObject):
    import Inspector
    return Inspector.inspect(anObject)

import __builtin__
__builtin__.inspect = inspect

