from direct.directnotify.DirectNotifyGlobal import directNotify
from direct.showbase.PythonUtil import Queue, invertDictLossless
from direct.showbase.PythonUtil import itype, serialNum, safeRepr
from direct.showbase.Job import Job
import types, weakref, random, __builtin__

class CheckContainers(Job):
    """
    Job to check container sizes and find potential leaks; sub-job of ContainerLeakDetector
    """
    def __init__(self, name, leakDetector, index):
        Job.__init__(self, name)
        self._leakDetector = leakDetector
        self.notify = self._leakDetector.notify
        self._index = index

    def getPriority(self):
        return Job.Priorities.Normal
    
    def run(self):
        self._leakDetector._index2containerName2len[self._index] = {}
        self._leakDetector.notify.debug(repr(self._leakDetector._id2pathStr))
        ids = self._leakDetector._id2pathStr.keys()
        # record the current len of each container
        for id in ids:
            yield None
            name = self._leakDetector._id2pathStr[id]
            try:
                container = eval(name)
            except NameError, ne:
                # this container no longer exists
                self.notify.debug('container %s no longer exists', name)
                del self._leakDetector._id2pathStr[id]
                continue
            cLen = len(container)
            self._leakDetector._index2containerName2len[self._index][name] = cLen
        # compare the current len of each container to past lens
        if self._index > 0:
            idx2name2len = self._leakDetector._index2containerName2len
            for name in idx2name2len[self._index]:
                yield None
                if name in idx2name2len[self._index-1]:
                    diff = idx2name2len[self._index][name] - idx2name2len[self._index-1][name]
                    if diff > 0:
                        if diff > idx2name2len[self._index-1][name]:
                            minutes = (self._leakDetector._index2delay[self._index] -
                                       self._leakDetector._index2delay[self._index-1]) / 60.
                            self.notify.warning('container %s grew > 200% in %s minutes' % (name, minutes))
                    if self._index > 3:
                        diff2 = idx2name2len[self._index-1][name] - idx2name2len[self._index-2][name]
                        diff3 = idx2name2len[self._index-2][name] - idx2name2len[self._index-3][name]
                        if self._index <= 5:
                            msg = ('%s consistently increased in length over the last 3 periods (currently %s items)' %
                                   (name, idx2name2len[self._index][name]))
                            self.notify.warning(msg)
                        else:
                            # if size has consistently increased over the last 5 checks, send out a warning
                            diff4 = idx2name2len[self._index-3][name] - idx2name2len[self._index-4][name]
                            diff5 = idx2name2len[self._index-4][name] - idx2name2len[self._index-5][name]
                            if diff > 0 and diff2 > 0 and diff3 > 0 and diff4 > 0 and diff5 > 0:
                                msg = ('%s consistently increased in length over the last 5 periods (currently %s items), notifying system' %
                                       (name, idx2name2len[self._index][name]))
                                self.notify.warning(msg)
                                messenger.send(self._leakDetector.getLeakEvent(), [msg])
        yield Job.Done

class ContainerLeakDetector(Job):
    """
    Low-priority Python object-graph walker that looks for leaking containers.
    To reduce memory usage, this does a random walk of the Python objects to
    discover containers rather than keep a set of all visited objects.
    Checks container sizes at ever-increasing intervals.
    """
    notify = directNotify.newCategory("ContainerLeakDetector")
    # set of containers that should not be examined
    PrivateIds = set()

    def __init__(self, name, firstCheckDelay = None):
        Job.__init__(self, name)
        self._serialNum = serialNum()
        self._priority = (Job.Priorities.Low + Job.Priorities.Normal) / 2
        self._checkContainersJob = None
        # run first check after one hour
        if firstCheckDelay is None:
            firstCheckDelay = 60. * 60.
        self._nextCheckDelay = firstCheckDelay
        self._index2containerName2len = {}
        self._index2delay = {}
        # set up our data structures
        self._id2pathStr = {}
        self._curObjPathStr = '__builtin__.__dict__'
        jobMgr.add(self)
        ContainerLeakDetector.PrivateIds.update(set([
            id(ContainerLeakDetector.PrivateIds),
            id(self._id2pathStr),
            ]))

    def destroy(self):
        if self._checkContainersJob is not None:
            jobMgr.remove(self._checkContainersJob)
            self._checkContainersJob = None
        del self._id2pathStr
        del self._index2containerName2len
        del self._index2delay

    def getPriority(self):
        return self._priority

    def getCheckTaskName(self):
        return 'checkForLeakingContainers-%s' % self._serialNum

    def getLeakEvent(self):
        # passes description string as argument
        return 'containerLeakDetected-%s' % self._serialNum

    def _getContainerByEval(self, evalStr):
        try:
            container = eval(evalStr)
        except NameError, ne:
            return None
        return container

    def run(self):
        # push on a few things that we want to give priority
        # for the sake of the variable-name printouts
        self._nameContainer(__builtin__.__dict__, '__builtin__.__dict__')
        try:
            base
        except:
            pass
        else:
            self._nameContainer(base.__dict__, 'base.__dict__')
        try:
            simbase
        except:
            pass
        else:
            self._nameContainer(simbase.__dict__, 'simbase.__dict__')

        taskMgr.doMethodLater(self._nextCheckDelay, self._checkForLeaks,
                              self.getCheckTaskName())

        while True:
            # yield up here instead of at the end, since we skip back to the
            # top of the while loop from various points
            yield None
            #import pdb;pdb.set_trace()
            curObj = None
            curObj = self._getContainerByEval(self._curObjPathStr)
            if curObj is None:
                self.notify.debug('lost current container: %s' % self._curObjPathStr)
                while len(self._id2pathStr):
                    _id = random.choice(self._id2pathStr.keys())
                    curObj = self._getContainerByEval(self._id2pathStr[_id])
                    if curObj is not None:
                        break
                    # container is no longer valid
                    del self._id2pathStr[_id]
                self._curObjPathStr = self._id2pathStr[_id]
            #print '%s: %s, %s' % (id(curObj), type(curObj), self._id2pathStr[id(curObj)])
            self.notify.debug('--> %s' % self._curObjPathStr)

            # keep a copy of this obj's eval str, it might not be in _id2pathStr
            curObjPathStr = self._curObjPathStr
            # if we hit a dead end, go back to __builtin__
            self._curObjPathStr = '__builtin__'

            try:
                if curObj.__class__.__name__ == 'method-wrapper':
                    continue
            except:
                pass

            if type(curObj) in (types.StringType, types.UnicodeType):
                continue
            
            if type(curObj) in (types.ModuleType, types.InstanceType):
                child = curObj.__dict__
                if not self._isDeadEnd(child):
                    self._curObjPathStr = curObjPathStr + '.__dict__'
                    if self._isContainer(child):
                        self._nameContainer(child, self._curObjPathStr)
                continue

            if type(curObj) is types.DictType:
                key = None
                attr = None
                keys = curObj.keys()
                # we will continue traversing the object graph via the last container
                # in the list; shuffle the list to randomize the traversal
                random.shuffle(keys)
                for key in keys:
                    try:
                        attr = curObj[key]
                    except KeyError, e:
                        self.notify.warning('could not index into %s with key %s' % (curObjPathStr,
                                                                                     key))
                        continue
                    if not self._isDeadEnd(attr):
                        if curObj is __builtin__:
                            self._curObjPathStr = PathStr(key)
                            if key == '__doc__':
                                import pdb;pdb.set_trace()
                            if self._isContainer(attr):
                                self._nameContainer(attr, PathStr(key))
                        else:
                            # if the parent dictionary is an instance dictionary, remove the __dict__
                            # and use the . operator
                            dLen = len('__dict__')
                            if len(self._curObjPathStr) >= dLen and self._curObjPathStr[-dLen:] == '__dict__':
                                self._curObjPathStr = curObjPathStr[:-dLen] + '.%s' % safeRepr(key)
                            else:
                                self._curObjPathStr = curObjPathStr + '[%s]' % safeRepr(key)
                            if self._isContainer(attr):
                                self._nameContainer(attr, self._curObjPathStr)
                del key
                del attr
                continue

            if type(curObj) is not types.FileType:
                try:
                    itr = iter(curObj)
                except:
                    pass
                else:
                    try:
                        index = 0
                        attrs = []
                        while 1:
                            try:
                                attr = itr.next()
                            except:
                                # some custom classes don't do well when iterated
                                attr = None
                                break
                            attrs.append(attr)
                        # we will continue traversing the object graph via the last container
                        # in the list; shuffle the list to randomize the traversal
                        random.shuffle(attrs)
                        for attr in attrs:
                            if not self._isDeadEnd(attr):
                                self._curObjPathStr = curObjPathStr + '[%s]' % index
                                if self._isContainer(attr):
                                    self._nameContainer(attr, self._curObjPathStr)
                            index += 1
                        del attr
                    except StopIteration, e:
                        pass
                    del itr
                    continue

            try:
                childNames = dir(curObj)
            except:
                pass
            else:
                childName = None
                child = None
                # we will continue traversing the object graph via the last container
                # in the list; shuffle the list to randomize the traversal
                random.shuffle(childNames)
                for childName in childNames:
                    child = getattr(curObj, childName)
                    if not self._isDeadEnd(child):
                        self._curObjPathStr = curObjPathStr + '.%s' % childName
                        if self._isContainer(child):
                            self._nameContainer(child, self._curObjPathStr)
                del childName
                del child
                continue

        yield Job.Done
        
    def _isDeadEnd(self, obj):
        if type(obj) in (types.BooleanType, types.BuiltinFunctionType,
                         types.BuiltinMethodType, types.ComplexType,
                         types.FloatType, types.IntType, types.LongType,
                         types.NoneType, types.NotImplementedType,
                         types.TypeType, types.CodeType, types.FunctionType):
            return True
        # if it's an internal object, ignore it
        if id(obj) in ContainerLeakDetector.PrivateIds:
            return True
        return False

    def _isContainer(self, obj):
        try:
            len(obj)
        except:
            return False
        return True

    def _nameContainer(self, cont, pathStr):
        if self.notify.getDebug():
            self.notify.debug('_nameContainer: %s' % pathStr)
            printStack()
        contId = id(cont)
        # if this container is new, or the pathStr is shorter than what we already have,
        # put it in the table
        if contId not in self._id2pathStr or len(pathStr) < len(self._id2pathStr[contId]):
            self._id2pathStr[contId] = pathStr

    def _checkForLeaks(self, task=None):
        self._index2delay[len(self._index2containerName2len)] = self._nextCheckDelay
        self._checkContainersJob = CheckContainers(
            '%s-checkForLeaks' % self.getJobName(), self, len(self._index2containerName2len))
        jobMgr.add(self._checkContainersJob)
        
        self._nextCheckDelay *= 2
        taskMgr.doMethodLater(self._nextCheckDelay, self._checkForLeaks,
                              self.getCheckTaskName())
