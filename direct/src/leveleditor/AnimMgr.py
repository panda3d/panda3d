"""
Defines AnimMgr
"""
from .AnimMgrBase import *

class AnimMgr(AnimMgrBase):
    """ Animation will create, manage, update animations in the scene """

    def __init__(self, editor):
        AnimMgrBase.__init__(self, editor)
