# objects that report different types of leaks to the ContainerLeakDetector

from pandac.PandaModules import *
import __builtin__, gc

class LeakDetector:
    def __init__(self):
        # put this object just under __builtins__ where the
        # ContainerLeakDetector will find it quickly
        if not hasattr(__builtin__, "leakDetectors"):
            __builtin__.leakDetectors = {}
        self._leakDetectorsKey = self.getLeakDetectorKey()
        leakDetectors[self._leakDetectorsKey] = self
    def destroy(self):
        del leakDetectors[self._leakDetectorsKey]

    def getLeakDetectorKey(self):
        # this string will be shown to the end user and should ideally contain enough information to
        # point to what is leaking
        return '%s-%s' % (self.__class__.__name__, id(self))

class GarbageLeakDetector(LeakDetector):
    # are we accumulating Python garbage?
    def __len__(self):
        # do a garbage collection
        oldFlags = gc.get_debug()
        gc.set_debug(0)
        gc.collect()
        numGarbage = len(gc.garbage)
        del gc.garbage[:]
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

class TaskLeakDetectorBase:
    def _getTaskNamePattern(self, taskName):
        # get a generic string pattern from a task name by removing numeric characters
        for i in xrange(10):
            taskName = taskName.replace('%s' % i, '')
        return taskName
    
class _TaskNamePatternLeakDetector(LeakDetector, TaskLeakDetectorBase):
    # tracks the number of each individual task type
    # e.g. are we leaking 'examine-<doId>' tasks
    def __init__(self, taskNamePattern):
        self._taskNamePattern = taskNamePattern
        LeakDetector.__init__(self)

    def __len__(self):
        # count the number of tasks that match our task name pattern
        numTasks = 0
        for task in taskMgr.getTasks():
            if self._getTaskNamePattern(task.name) == self._taskNamePattern:
                numTasks += 1
        for task in taskMgr.getDoLaters():
            if self._getTaskNamePattern(task.name) == self._taskNamePattern:
                numTasks += 1
        return numTasks

    def getLeakDetectorKey(self):
        return '%s-%s' % (self._taskNamePattern, LeakDetector.getLeakDetectorKey(self))

class TaskLeakDetector(LeakDetector, TaskLeakDetectorBase):
    # tracks the number task 'types' and creates leak detectors for each task type
    def __init__(self):
        LeakDetector.__init__(self)
        self._taskName2collector = {}

    def destroy(self):
        for taskName, collector in self._taskName2collector.iteritems():
            collector.destroy()
        del self._taskName2collector
        LeakDetector.destroy(self)

    def _processTaskName(self, taskName):
        # if this is a new task name pattern, create a leak detector for that pattern
        namePattern = self._getTaskNamePattern(taskName)
        if namePattern not in self._taskName2collector:
            self._taskName2collector[namePattern] = _TaskNamePatternLeakDetector(namePattern)

    def __len__(self):
        # update our table of task leak detectors
        for task in taskMgr.getTasks():
            self._processTaskName(task.name)
        for task in taskMgr.getDoLaters():
            self._processTaskName(task.name)
        # are we leaking task types?
        return len(self._taskName2collector)
