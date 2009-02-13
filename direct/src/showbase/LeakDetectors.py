# objects that report different types of leaks to the ContainerLeakDetector

from pandac.PandaModules import *
from direct.showbase.DirectObject import DirectObject
from direct.showbase.Job import Job
import __builtin__, gc

class LeakDetector:
    def __init__(self):
        # put this object just under __builtins__ where the
        # ContainerLeakDetector will find it quickly
        if not hasattr(__builtin__, "leakDetectors"):
            __builtin__.leakDetectors = {}
        self._leakDetectorsKey = self.getLeakDetectorKey()
        if __dev__:
            assert self._leakDetectorsKey not in leakDetectors
        leakDetectors[self._leakDetectorsKey] = self
    def destroy(self):
        del leakDetectors[self._leakDetectorsKey]

    def getLeakDetectorKey(self):
        # this string will be shown to the end user and should ideally contain enough information to
        # point to what is leaking
        return '%s-%s' % (self.__class__.__name__, id(self))

class ObjectTypeLeakDetector(LeakDetector):
    def __init__(self, otld, objType, generation):
        self._otld = otld
        self._objType = objType
        self._generation = generation
        LeakDetector.__init__(self)

    def destroy(self):
        self._otld = None
        LeakDetector.destroy(self)

    def getLeakDetectorKey(self):
        return '%s-%s' % (self._objType, self.__class__.__name__)

    def __len__(self):
        num = self._otld._getNumObjsOfType(self._objType, self._generation)
        self._generation = self._otld._getGeneration()
        return num

class ObjectTypesLeakDetector(LeakDetector):
    # are we accumulating any particular Python object type?
    def __init__(self):
        LeakDetector.__init__(self)
        self._type2ld = {}
        self._type2count = {}
        self._generation = 0
        self._thisLdGen = 0

    def destroy(self):
        for ld in self._type2ld.itervalues():
            ld.destroy()
        LeakDetector.destroy(self)

    def _recalc(self):
        objs = gc.get_objects()
        self._type2count = {}
        for obj in objs:
            objType = safeTypeName(obj)
            if objType not in self._type2ld:
                self._type2ld[objType] = ObjectTypeLeakDetector(self, objType, self._generation)
            self._type2count.setdefault(objType, 0)
            self._type2count[objType] += 1
        self._generation += 1

    def _getGeneration(self):
        return self._generation

    def _getNumObjsOfType(self, objType, otherGen):
        if self._generation == otherGen:
            self._recalc()
        return self._type2count.get(objType, 0)

    def __len__(self):
        if self._generation == self._thisLdGen:
            self._recalc()
        self._thisLdGen = self._generation
        return len(self._type2count)

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
        haveMemoryUsage = True
        try:
            MemoryUsage
        except:
            haveMemoryUsage = False
        if haveMemoryUsage:
            return int(MemoryUsage.getCurrentCppSize())
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
        return '%s-%s' % (self._taskNamePattern, self.__class__.__name__)

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
        self._taskName2collector = {}
        # update our table of task leak detectors
        for task in taskMgr.getTasks():
            self._processTaskName(task.name)
        for task in taskMgr.getDoLaters():
            self._processTaskName(task.name)
        # are we leaking task types?
        return len(self._taskName2collector)

class MessageLeakDetectorBase:
    def _getMessageNamePattern(self, msgName):
        # get a generic string pattern from a message name by removing numeric characters
        for i in xrange(10):
            msgName = msgName.replace('%s' % i, '')
        return msgName

class _MessageTypeLeakDetector(LeakDetector, MessageLeakDetectorBase):
    # tracks the number of objects that are listening to each message
    def __init__(self, msgNamePattern):
        self._msgNamePattern = msgNamePattern
        self._msgNames = set()
        LeakDetector.__init__(self)

    def addMsgName(self, msgName):
        # for efficiency, we keep the actual message names around
        # for queries on the messenger
        self._msgNames.add(msgName)

    def __len__(self):
        toRemove = set()
        num = 0
        for msgName in self._msgNames:
            n = messenger._getNumListeners(msgName)
            if n == 0:
                toRemove.add(msgName)
            else:
                num += n
        # remove message names that are no longer in the messenger
        self._msgNames.difference_update(toRemove)
        return num

    def getLeakDetectorKey(self):
        return '%s-%s' % (self._msgNamePattern, self.__class__.__name__)

class _MessageTypeLeakDetectorCreator(Job):
    def __init__(self, creator):
        Job.__init__(self, uniqueName(typeName(self)))
        self._creator = creator

    def destroy(self):
        self._creator = None
        Job.destroy(self)

    def finished(self):
        Job.finished(self)

    def run(self):
        for msgName in messenger._getEvents():
            yield None
            namePattern = self._creator._getMessageNamePattern(msgName)
            if namePattern not in self._creator._msgName2detector:
                self._creator._msgName2detector[namePattern] = _MessageTypeLeakDetector(namePattern)
            self._creator._msgName2detector[namePattern].addMsgName(msgName)
        yield Job.Done

class MessageTypesLeakDetector(LeakDetector, MessageLeakDetectorBase):
    def __init__(self):
        LeakDetector.__init__(self)
        self._msgName2detector = {}
        self._createJob = None
        if config.GetBool('leak-message-types', 0):
            self._leakers = []
            self._leakTaskName = uniqueName('leak-message-types')
            taskMgr.add(self._leak, self._leakTaskName)

    def _leak(self, task):
        self._leakers.append(DirectObject())
        self._leakers[-1].accept('leak-msg', self._leak)
        return task.cont

    def destroy(self):
        if hasattr(self, '_leakTaskName'):
            taskMgr.remove(self._leakTaskName)
            for leaker in self._leakers:
                leaker.ignoreAll()
            self._leakers = None
        if self._createJob:
            self._createJob.destroy()
        self._createJob = None
        for msgName, detector in self._msgName2detector.iteritems():
            detector.destroy()
        del self._msgName2detector
        LeakDetector.destroy(self)

    def __len__(self):
        if self._createJob:
            if self._createJob.isFinished():
                self._createJob.destroy()
                self._createJob = None
        self._createJob = _MessageTypeLeakDetectorCreator(self)
        jobMgr.add(self._createJob)
        # are we leaking message types?
        return len(self._msgName2detector)
    
class _MessageListenerTypeLeakDetector(LeakDetector):
    # tracks the number of each object type that is listening for events
    def __init__(self, typeName):
        self._typeName = typeName
        LeakDetector.__init__(self)

    def __len__(self):
        numObjs = 0
        for obj in messenger._getObjects():
            if typeName(obj) == self._typeName:
                numObjs += 1
        return numObjs

    def getLeakDetectorKey(self):
        return '%s-%s' % (self._typeName, self.__class__.__name__)

class _MessageListenerTypeLeakDetectorCreator(Job):
    def __init__(self, creator):
        Job.__init__(self, uniqueName(typeName(self)))
        self._creator = creator

    def destroy(self):
        self._creator = None
        Job.destroy(self)

    def finished(self):
        Job.finished(self)

    def run(self):
        for obj in messenger._getObjects():
            yield None
            tName = typeName(obj)
            if tName not in self._creator._typeName2detector:
                self._creator._typeName2detector[tName] = (
                    _MessageListenerTypeLeakDetector(tName))
        yield Job.Done

class MessageListenerTypesLeakDetector(LeakDetector):
    def __init__(self):
        LeakDetector.__init__(self)
        self._typeName2detector = {}
        self._createJob = None
        if config.GetBool('leak-message-listeners', 0):
            self._leakers = []
            self._leakTaskName = uniqueName('leak-message-listeners')
            taskMgr.add(self._leak, self._leakTaskName)

    def _leak(self, task):
        self._leakers.append(DirectObject())
        self._leakers[-1].accept(uniqueName('leak-msg-listeners'), self._leak)
        return task.cont

    def destroy(self):
        if hasattr(self, '_leakTaskName'):
            taskMgr.remove(self._leakTaskName)
            for leaker in self._leakers:
                leaker.ignoreAll()
            self._leakers = None
        if self._createJob:
            self._createJob.destroy()
        self._createJob = None
        for typeName, detector in self._typeName2detector.iteritems():
            detector.destroy()
        del self._typeName2detector
        LeakDetector.destroy(self)

    def __len__(self):
        if self._createJob:
            if self._createJob.isFinished():
                self._createJob.destroy()
                self._createJob = None
        self._createJob = _MessageListenerTypeLeakDetectorCreator(self)
        jobMgr.add(self._createJob)
        # are we leaking listener types?
        return len(self._typeName2detector)
