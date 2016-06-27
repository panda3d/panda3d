"""
Defines ObjectMgr
"""
from .ObjectMgrBase import *

class ObjectMgr(ObjectMgrBase):
    """ ObjectMgr will create, manage, update objects in the scene """

    def __init__(self, editor):
        ObjectMgrBase.__init__(self, editor)
