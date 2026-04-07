
"This module is deprecated.  Import from panda3d.core and other panda3d.* modules instead."

if __debug__:
    print("Warning: pandac.PandaModules is deprecated, import from panda3d.core instead")

try:
    from panda3d.core import *
except ImportError as err:
    if "No module named core" not in str(err):
        raise
try:
    from panda3d.physics import *
except ImportError as err:
    if "No module named physics" not in str(err):
        raise
try:
    from panda3d.fx import *
except ImportError as err:
    if "No module named fx" not in str(err):
        raise
try:
    from panda3d.direct import *
except ImportError as err:
    if "No module named direct" not in str(err):
        raise
try:
    from panda3d.vision import *
except ImportError as err:
    if "No module named vision" not in str(err):
        raise
try:
    from panda3d.skel import *
except ImportError as err:
    if "No module named skel" not in str(err):
        raise
try:
    from panda3d.egg import *
except ImportError as err:
    if "No module named egg" not in str(err):
        raise
try:
    from panda3d.ode import *
except ImportError as err:
    if "No module named ode" not in str(err):
        raise

from direct.showbase import DConfig

def get_config_showbase():
    return DConfig

def get_config_express():
    return DConfig

getConfigShowbase = get_config_showbase
getConfigExpress = get_config_express
