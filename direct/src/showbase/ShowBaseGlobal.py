"""instantiate global ShowBase object"""

from ShowBase import *

# Create the showbase instance
ShowBase()

# Set direct notify categories now that we have config
directNotify.setDconfigLevels()

def inspect(anObject):
    import Inspector
    return Inspector.inspect(anObject)

__builtins__["inspect"] = inspect
__builtins__["__dev__"] = base.config.GetBool('want-dev', 0)
