"""instantiate global ShowBase object"""

from ShowBase import *

base = ShowBase()

# Make some global aliases for convenience
render2d = base.render2d
render = base.render
hidden = base.hidden
camera = base.camera
loader = base.loader
ostream = Notify.out()
run = base.run
tkroot = base.tkroot
