"""instantiate global ShowBase object"""

import ShowBase

base = ShowBase.ShowBase()

# Make some global aliases for convenience
render2d = base.render2d
render = base.render
hidden = base.hidden
camera = base.camera
loader = base.loader
messenger = base.messenger
taskMgr = base.taskMgr
run = base.run
tkroot = base.tkroot
