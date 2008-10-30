from direct.directnotify.DirectNotifyGlobal import directNotify
from direct.fsm.StatePush import FunctionCall

class FrameProfiler:
    notify = directNotify.newCategory('FrameProfiler')

    def __init__(self):
        self._enableFC = FunctionCall(self._setEnabled, taskMgr.getProfileFramesSV())

    def destroy(self):
        self._enableFC.set(False)
        self._enableFC.destroy()
        
    def _setEnabled(self, enabled):
        if enabled:
            print 'FrameProfiler enabled'
        else:
            print 'FrameProfiler disabled'
