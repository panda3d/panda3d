from ShowBaseGlobal import *
import __builtin__

# If specified in the user's Configrc, create the direct session
if base.wantDIRECT:
    from DirectSession import *
    __builtin__.direct = base.direct = DirectSession()
    direct.enable()
else:
    # Otherwise set the values to None
    __builtin__.direct = base.direct = None

