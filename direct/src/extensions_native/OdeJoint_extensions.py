
from extension_native_helpers import *
from libpanda import *

####################################################################
#Dtool_funcToMethod(func, class)
#del func
#####################################################################

"""
ODE-extensions module: contains methods to extend functionality
of the ODE classes
"""

def attach(self, body1, body2):
    """
    Attach two bodies together.
    If either body is None, the other will be attached to the environment.
    """
    if body1 and body2:
        self.attachBodies(body1, body2)
    elif body1 and not body2:
        self.attachBody(body1, 0)
    elif not body1 and body2:
        self.attachBody(body2, 1)
Dtool_funcToMethod(attach, OdeJoint)
del attach
