"""instantiate global ShowBase object"""

from ShowBase import *
import __builtin__

__builtin__.base = ShowBase()

# Make some global aliases for convenience
__builtin__.render2d = base.render2d
__builtin__.aspect2d = base.aspect2d
__builtin__.render = base.render
__builtin__.hidden = base.hidden
__builtin__.camera = base.camera
__builtin__.loader = base.loader
__builtin__.ostream = Notify.out()
__builtin__.run = base.run
__builtin__.taskMgr = base.taskMgr
__builtin__.eventMgr = base.eventMgr
__builtin__.messenger = base.messenger
__builtin__.config = base.config
__builtin__.directNotify = directNotify

# Initialize DIRECT
if base.wantDIRECT:
    from DirectSession import *
    __builtin__.direct = base.direct = DirectSession()
    direct.enable()
else:
    # Otherwise set the values to None
    __builtin__.direct = base.direct = None

# Set direct notify categories now that we have config
directNotify.setDconfigLevels()

def inspect(anObject):
    import Inspector
    Inspector.inspect(anObject)

__builtin__.inspect = inspect

