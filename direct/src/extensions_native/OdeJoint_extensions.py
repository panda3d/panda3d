from extension_native_helpers import *
Dtool_PreloadDLL("libpanda")
from libpanda import *

####################################################################
#Dtool_funcToMethod(func, class)
#del func
#####################################################################

"""
OdeJoint-extensions module: contains methods to extend functionality
of the OdeJoint class
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

def convert(self):
    """
    Do a sort of pseudo-downcast on this joint in 
    order to expose its specialized functions.
    """
    if self.getJointType() == OdeJoint.JTBall:
        return self.convertToBall()
    elif self.getJointType() == OdeJoint.JTHinge:
        return self.convertToHinge()
    elif self.getJointType() == OdeJoint.JTSlider:
        return self.convertToSlider()
    elif self.getJointType() == OdeJoint.JTContact:
        return self.convertToContact()
    elif self.getJointType() == OdeJoint.JTUniversal:
        return self.convertToUniversal()
    elif self.getJointType() == OdeJoint.JTHinge2:
        return self.convertToHinge2()
    elif self.getJointType() == OdeJoint.JTFixed:
        return self.convertToFixed()
    elif self.getJointType() == OdeJoint.JTNull:
        return self.convertToNull()
    elif self.getJointType() == OdeJoint.JTAMotor:
        return self.convertToAMotor()
    elif self.getJointType() == OdeJoint.JTLMotor:
        return self.convertToLMotor()
    elif self.getJointType() == OdeJoint.JTPlane2d:
        return self.convertToPlane2d()
Dtool_funcToMethod(convert, OdeJoint)
del convert

