"""instantiate global ShowBase object"""

from ShowBase import *

base = ShowBase()

# Make some global aliases for convenience
render2d = base.render2d
render = base.render
hidden = base.hidden
camera = base.camera
loader = base.loader
run = base.run
tkroot = base.tkroot

# Now create the DIRECT tools
if base.wantDIRECT:
    import DirectSession
    direct = base.direct = DirectSession.DirectSession()
    chanCenter = direct.chanCenter
else:
    direct = base.direct = None
    chanCenter = None
