# objects that report different types of leaks to the ContainerLeakDetector

from pandac.PandaModules import *
from direct.showbase.PythonUtil import gcDebugOn
import __builtin__, gc

class LeakDetector:
    def __init__(self):
        # put this object just under __builtins__ where the
        # ContainerLeakDetector will find it quickly
        if not hasattr(__builtin__, "leakDetectors"):
            __builtin__.leakDetectors = {}
        leakDetectors[id(self)] = self
    def destroy(self):
        del leakDetectors[id(self)]

class GarbageLeakDetector(LeakDetector):
    # are we accumulating Python garbage?
    def __len__(self):
        # do a garbage collection
        wasOn = gcDebugOn()
        oldFlags = gc.get_debug()
        if not wasOn:
            gc.set_debug(gc.DEBUG_SAVEALL)
        gc.collect()
        numGarbage = len(gc.garbage)
        del gc.garbage[:]
        if not wasOn:
            gc.set_debug(oldFlags)
        return numGarbage

class SceneGraphLeakDetector(LeakDetector):
    # is a scene graph leaking nodes?
    def __init__(self, render):
        LeakDetector.__init__(self)
        self._render = render
        if config.GetBool('leak-scene-graph', 0):
            self._leakTaskName = 'leakNodes-%s' % serialNum()
            self._leakNode()
    def destroy(self):
        if hasattr(self, '_leakTaskName'):
            taskMgr.remove(self._leakTaskName)
        del self._render
        LeakDetector.destroy(self)
    def __len__(self):
        try:
            # this will be available when the build server finishes
            return self._render.countNumDescendants()
        except:
            return self._render.getNumDescendants()
    def __repr__(self):
        return 'SceneGraphLeakDetector(%s)' % self._render
    def _leakNode(self, task=None):
        self._render.attachNewNode('leakNode-%s' % serialNum())
        taskMgr.doMethodLater(10, self._leakNode, self._leakTaskName)

class CppMemoryUsage(LeakDetector):
    def __len__(self):
        if config.GetBool('track-memory-usage', 0):
            return int(MemoryUsage.getCppSize())
        else:
            return 0
