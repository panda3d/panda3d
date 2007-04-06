from direct.directnotify.DirectNotifyGlobal import directNotify
from direct.showbase.PythonUtil import Queue, invertDictLossless
from direct.showbase.PythonUtil import itype, serialNum, safeRepr
from direct.showbase.Job import Job
import types, weakref, gc, random, __builtin__

def _createContainerLeak():
    def leakContainer(task):
        if not hasattr(simbase, 'leakContainer'):
            simbase.leakContainer = []
        simbase.leakContainer.append(1)
        return task.cont
    taskMgr.add(leakContainer, 'leakContainer-%s' % serialNum())

class CheckContainers(Job):
    """
    Job to check container sizes and find potential leaks; sub-job of ContainerLeakDetector
    """
    def __init__(self, name, leakDetector, index):
        Job.__init__(self, name)
        self._leakDetector = leakDetector
        self.notify = self._leakDetector.notify
        self._index = index
        ContainerLeakDetector.addPrivateId(self.__dict__)

    def destroy(self):
        ContainerLeakDetector.removePrivateId(self.__dict__)
        Job.destroy(self)

    def getPriority(self):
        return Job.Priorities.Normal
    
    def run(self):
        self._leakDetector._index2containerId2len[self._index] = {}
        #self._leakDetector.notify.debug(repr(self._leakDetector._id2ref))
        ids = self._leakDetector.getContainerIds()
        # record the current len of each container
        for id in ids:
            yield None
            try:
                container = self._leakDetector.getContainerById(id)
            except Exception, e:
                # this container no longer exists
                self.notify.debug('container %s no longer exists; caught exception in getContainerById (%s)' % (
                    self._leakDetector.getContainerNameById(id), e))
                self._leakDetector.removeContainerById(id)
                continue
            if container is None:
                # this container no longer exists
                self.notify.debug('container %s no longer exists; getContainerById returned None' %
                                  self._leakDetector.getContainerNameById(id))
                self._leakDetector.removeContainerById(id)
                continue
            try:
                cLen = len(container)
            except Exception, e:
                # this container no longer exists
                self.notify.debug('%s is no longer a container, it is now %s (%s)' %
                                  (self._leakDetector.getContainerNameById(id), safeRepr(container), e))
                self._leakDetector.removeContainerById(id)
                continue
            self._leakDetector._index2containerId2len[self._index][id] = cLen
        # compare the current len of each container to past lens
        if self._index > 0:
            idx2id2len = self._leakDetector._index2containerId2len
            for id in idx2id2len[self._index]:
                yield None
                if id in idx2id2len[self._index-1]:
                    diff = idx2id2len[self._index][id] - idx2id2len[self._index-1][id]
                    if diff > 0:
                        if diff > idx2id2len[self._index-1][id]:
                            minutes = (self._leakDetector._index2delay[self._index] -
                                       self._leakDetector._index2delay[self._index-1]) / 60.
                            name = self._leakDetector.getContainerNameById(id)
                            self.notify.warning('container %s grew > 200%% in %s minutes' % (name, minutes))
                    if (self._index > 3 and
                        id in idx2id2len[self._index-2] and
                        id in idx2id2len[self._index-3]):
                        diff2 = idx2id2len[self._index-1][id] - idx2id2len[self._index-2][id]
                        diff3 = idx2id2len[self._index-2][id] - idx2id2len[self._index-3][id]
                        if self._index <= 5:
                            if diff > 0 and diff2 > 0 and diff3 > 0:
                                name = self._leakDetector.getContainerNameById(id)
                                msg = ('%s consistently increased in length over the last 3 periods (currently %s items)' %
                                       (name, idx2id2len[self._index][id]))
                                self.notify.warning(msg)
                        elif (id in idx2id2len[self._index-4] and
                              id in idx2id2len[self._index-5]):
                            # if size has consistently increased over the last 5 checks, send out a warning
                            diff4 = idx2id2len[self._index-3][id] - idx2id2len[self._index-4][id]
                            diff5 = idx2id2len[self._index-4][id] - idx2id2len[self._index-5][id]
                            if diff > 0 and diff2 > 0 and diff3 > 0 and diff4 > 0 and diff5 > 0:
                                name = self._leakDetector.getContainerNameById(id)
                                msg = ('%s consistently increased in length over the last 5 periods (currently %s items), notifying system' %
                                       (name, idx2id2len[self._index][id]))
                                self.notify.warning(msg)
                                messenger.send(self._leakDetector.getLeakEvent(), [msg])
        yield Job.Done

class PruneContainerRefs(Job):
    """
    Job to destroy any container refs that have Indirections that are holding references
    to objects that should be garbage-collected
    """
    def __init__(self, name, leakDetector):
        Job.__init__(self, name)
        self._leakDetector = leakDetector
        self.notify = self._leakDetector.notify
        self._index = index
        ContainerLeakDetector.addPrivateId(self.__dict__)

    def destroy(self):
        ContainerLeakDetector.removePrivateId(self.__dict__)
        Job.destroy(self)

    def getPriority(self):
        return Job.Priorities.Normal
    
    def run(self):
        ids = self._leakDetector._id2ref.keys()
        for id in ids:
            yield None
            ref = self._leakDetector._id2ref[id]
            ref.destroyIfGarbageDictKey()
        yield Job.Done

class NoDictKey:
    pass

class Indirection:
    # represents the indirection that brings you from a container to an element of the container
    # stored as a string to be used as part of an eval
    # each dictionary dereference is individually eval'd since the dict key might have been garbage-collected

    def __init__(self, evalStr=None, dictKey=NoDictKey):
        # if this is a dictionary lookup, pass dictKey instead of evalStr
        self.evalStr = evalStr
        self.dictKey = NoDictKey
        # is the dictKey a weak reference?
        self._isWeakRef = False
        self._refCount = 0
        if dictKey is not NoDictKey:
            # if we can repr/eval the key, store it as an evalStr
            keyRepr = repr(dictKey)
            useEval = False
            try:
                keyEval = eval(keyRepr)
                useEval = True
            except:
                pass
            if useEval:
                # check to make sure the eval succeeded
                if hash(keyEval) != hash(dictKey):
                    useEval = False
            if useEval:
                # eval/repr succeeded, store as an evalStr
                self.evalStr = '[%s]' % keyRepr
            else:
                try:
                    # store a weakref to the key
                    self.dictKey = weakref.ref(dictKey)
                    self._isWeakRef = True
                except TypeError, e:
                    self.dictKey = dictKey
                    self._isWeakRef = False

    def destroy(self):
        del self.dictKey

    def acquire(self):
        self._refCount += 1
    def release(self):
        self._refCount -= 1
        if self._refCount == 0:
            self.destroy()

    def isDictKey(self):
        # is this an indirection through a dictionary?
        return self.dictKey is not NoDictKey
    def isGarbageDictKey(self):
        # are we holding a non-weak reference to an object that should be
        # garbage-collected?
        if self.isDictKey() and not self._isWeakRef:
            referrers = gc.get_referrers(self.dictKey)
            print referrers
            import pdb;pdb.set_trace()

    def _getNonWeakDictKey(self):
        if not self._isWeakRef:
            return self.dictKey
        else:
            key = self.dictKey()
            if key is None:
                return '<garbage-collected dict key>'
            return key

    def dereferenceDictKey(self, parentDict):
        # look ourselves up in parentDict
        key = self._getNonWeakDictKey()
        # objects in __builtin__ will have parentDict==None
        if parentDict is None:
            return key
        return parentDict[key]

    def getString(self, prevIndirection=None, nextIndirection=None):
        # return our contribution to the full name of an object
        instanceDictStr = '.__dict__'
        if self.evalStr is not None:
            # if we're an instance dict, skip over this one (obj.__dict__[keyName] == obj.keyName)
            if nextIndirection is not None and self.evalStr[-len(instanceDictStr):] == instanceDictStr:
                return self.evalStr[:-len(instanceDictStr)]
            # if the previous indirection was an instance dict, change our syntax from ['key'] to .key
            if prevIndirection is not None and prevIndirection.evalStr is not None:
                if prevIndirection.evalStr[-len(instanceDictStr):] == instanceDictStr:
                    return '.%s' % self.evalStr[2:-2]
            return self.evalStr

        # we're stored as a dict key
        # this might not eval, but that's OK, we're not using this string to find
        # the object, we dereference the parent dict
        keyRepr = safeRepr(self._getNonWeakDictKey())
        # if the previous indirection was an instance dict, change our syntax from ['key'] to .key
        if prevIndirection is not None and prevIndirection.evalStr is not None:
            if prevIndirection.evalStr[-len(instanceDictStr):] == instanceDictStr:
                return '.%s' % keyRepr
        return '[%s]' % keyRepr

    def __repr__(self):
        return self.getString()

class ContainerRef:
    """
    stores a reference to a container in a way that does not prevent garbage
    collection of the container
    stored as a series of 'indirections' (obj.foo -> '.foo', dict[key] -> '[key]', etc.)
    """
    class FailedEval(Exception):
        pass

    def __init__(self, indirection, other=None):
        self._indirections = []
        # if no other passed in, try ContainerRef.BaseRef
        if other is not None:
            for ind in other._indirections:
                self.addIndirection(ind)
        self.addIndirection(indirection)

    def destroy(self):
        for indirection in self._indirections:
            indirection.release()
        del self._indirections

    def destroyIfGarbageDictKey(self):
        # if any of our indirections are holding onto objects that
        # should be garbage-collected, destroy
        for indirection in self._indirections:
            if indirection.isGarbageDictKey():
                self.destroy()
                return

    def addIndirection(self, indirection):
        indirection.acquire()
        self._indirections.append(indirection)

    def _getContainerByEval(self, evalStr):
        try:
            container = eval(evalStr)
        except NameError, ne:
            return None
        return container

    def _evalWithObj(self, evalStr, curObj=None):
        # eval an evalStr, optionally based off of an existing object
        if curObj is not None:
            # eval('curObj.foo.bar.someDict')
            evalStr = 'curObj%s' % evalStr
        return self._getContainerByEval(evalStr)

    def getContainer(self):
        # try to get a handle on the container by eval'ing and looking things
        # up in dictionaries, depending on the type of each indirection
        #import pdb;pdb.set_trace()
        evalStr = ''
        curObj = None
        for indirection in self._indirections:
            if not indirection.isDictKey():
                # build up a string to be eval'd
                evalStr += indirection.getString()
            else:
                curObj = self._evalWithObj(evalStr, curObj)
                if curObj is None:
                    raise FailedEval(evalStr)
                # try to look up this key in the curObj dictionary
                curObj = indirection.dereferenceDictKey(curObj)
                evalStr = ''

        return self._evalWithObj(evalStr, curObj)
        
    def __repr__(self):
        str = ''
        prevIndirection = None
        curIndirection = None
        nextIndirection = None
        for i in xrange(len(self._indirections)):
            if i > 0:
                prevIndirection = self._indirections[i-1]
            else:
                prevIndirection = None
            curIndirection = self._indirections[i]
            if i < len(self._indirections)-1:
                nextIndirection = self._indirections[i+1]
            else:
                nextIndirection = None
            str += curIndirection.getString(prevIndirection=prevIndirection,
                                            nextIndirection=nextIndirection)
        return str

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
            firstCheckDelay = 60. * 15.
        self._nextCheckDelay = firstCheckDelay
        self._pruneTaskPeriod = config.GetFloat('leak-detector-prune-period', 60. * 30.)
        self._index2containerId2len = {}
        self._index2delay = {}
        # set up our data structures
        self._id2ref = {}

        # set up the base/starting object
        self._baseObjRef = ContainerRef(Indirection(evalStr='__builtin__.__dict__'))
        self._nameContainer(__builtin__.__dict__, self._baseObjRef)
        try:
            base
        except:
            pass
        else:
            self._baseObjRef = ContainerRef(Indirection(evalStr='base.__dict__'))
            self._nameContainer(base.__dict__, self._baseObjRef)
        try:
            simbase
        except:
            pass
        else:
            self._baseObjRef = ContainerRef(Indirection(evalStr='simbase.__dict__'))
            self._nameContainer(simbase.__dict__, self._baseObjRef)

        if config.GetBool('leak-container', 0):
            _createContainerLeak()

        self._curObjRef = self._baseObjRef

        jobMgr.add(self)
        ContainerLeakDetector.PrivateIds.update(set([
            id(ContainerLeakDetector.PrivateIds),
            id(self.__dict__),
            ]))

    def destroy(self):
        if self._checkContainersJob is not None:
            jobMgr.remove(self._checkContainersJob)
            self._checkContainersJob = None
        del self._id2ref
        del self._index2containerId2len
        del self._index2delay

    def getPriority(self):
        return self._priority

    @classmethod
    def addPrivateId(cls, obj):
        cls.PrivateIds.add(id(obj))
    @classmethod
    def removePrivateId(cls, obj):
        cls.PrivateIds.remove(id(obj))

    def _getCheckTaskName(self):
        return 'checkForLeakingContainers-%s' % self._serialNum
    def _getPruneTaskName(self):
        return 'pruneLeakingContainerRefs-%s' % self._serialNum

    def getLeakEvent(self):
        # passes description string as argument
        return 'containerLeakDetected-%s' % self._serialNum

    def getContainerIds(self):
        return self._id2ref.keys()

    def getContainerById(self, id):
        return self._id2ref[id].getContainer()
    def getContainerNameById(self, id):
        return repr(self._id2ref[id])
    def removeContainerById(self, id):
        del self._id2ref[id]

    def run(self):
        taskMgr.doMethodLater(self._nextCheckDelay, self._checkForLeaks,
                              self._getCheckTaskName())
        taskMgr.doMethodLater(self._pruneTaskPeriod, self._pruneContainerRefs,
                              self._getPruneTaskName())

        while True:
            # yield up here instead of at the end, since we skip back to the
            # top of the while loop from various points
            yield None
            #import pdb;pdb.set_trace()
            curObj = None
            if self._curObjRef is None:
                self._curObjRef = self._baseObjRef
            try:
                curObj = self._curObjRef.getContainer()
            except:
                self.notify.debug('lost current container: %s' % self._curObjRef)
                while len(self._id2ref):
                    _id = random.choice(self._id2ref.keys())
                    curObj = self.getContainerById(_id)
                    if curObj is not None:
                        break
                    # container is no longer valid
                    del self._id2ref[_id]
                self._curObjRef = self._id2ref[_id]
            #print '%s: %s, %s' % (id(curObj), type(curObj), self._id2ref[id(curObj)])
            #self.notify.debug('--> %s' % self._curObjRef)

            # keep a copy of this obj's eval str, it might not be in _id2ref
            curObjRef = self._curObjRef
            # if we hit a dead end, start over at a container we know about
            self._curObjRef = None

            if type(curObj) in (types.ModuleType, types.InstanceType):
                child = curObj.__dict__
                if not self._isDeadEnd(child):
                    self._curObjRef = ContainerRef(Indirection(evalStr='.__dict__'), curObjRef)
                    if self._isContainer(child):
                        self._nameContainer(child, self._curObjRef)
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
                        self.notify.warning('could not index into %s with key %s' % (curObjRef, key))
                        continue
                    if not self._isDeadEnd(attr, key):
                        if curObj is __builtin__.__dict__:
                            indirection=Indirection(evalStr=key)
                            self._curObjRef = ContainerRef(indirection)
                        else:
                            indirection=Indirection(dictKey=key)
                            self._curObjRef = ContainerRef(indirection, curObjRef)
                        if self._isContainer(attr):
                            self._nameContainer(attr, self._curObjRef)
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
                                self._curObjRef = ContainerRef(Indirection(evalStr='[%s]' % index), curObjRef)
                                if self._isContainer(attr):
                                    self._nameContainer(attr, self._curObjRef)
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
                    if not self._isDeadEnd(child, childName):
                        self._curObjRef = ContainerRef(Indirection(evalStr='.%s' % childName), curObjRef)
                        if self._isContainer(child):
                            self._nameContainer(child, self._curObjRef)
                del childName
                del child
                continue

        yield Job.Done
        
    def _isDeadEnd(self, obj, objName=None):
        if type(obj) in (types.BooleanType, types.BuiltinFunctionType,
                         types.BuiltinMethodType, types.ComplexType,
                         types.FloatType, types.IntType, types.LongType,
                         types.NoneType, types.NotImplementedType,
                         types.TypeType, types.CodeType, types.FunctionType,
                         types.StringType, types.UnicodeType):
            return True
        if id(obj) in self._id2ref:
            return True
        # if it's an internal object, ignore it
        if id(obj) in ContainerLeakDetector.PrivateIds:
            return True
        if objName in ('im_self', 'im_class'):
            return True
        try:
            className = obj.__class__.__name__
        except:
            pass
        else:
            # prevent infinite recursion in built-in containers related to methods
            if className == 'method-wrapper':
                return True
        return False
            

    def _isContainer(self, obj):
        try:
            len(obj)
        except:
            return False
        return True

    def _nameContainer(self, cont, objRef):
        """
        if self.notify.getDebug():
            self.notify.debug('_nameContainer: %s' % objRef)
            #printStack()
            """
        contId = id(cont)
        # if this container is new, or the objRef repr is shorter than what we already have,
        # put it in the table
        if contId not in self._id2ref or len(repr(objRef)) < len(repr(self._id2ref[contId])):
            self._id2ref[contId] = objRef

    def _checkForLeaks(self, task=None):
        self._index2delay[len(self._index2containerId2len)] = self._nextCheckDelay
        self._checkContainersJob = CheckContainers(
            '%s-checkForLeaks' % self.getJobName(), self, len(self._index2containerId2len))
        jobMgr.add(self._checkContainersJob)
        
        self._nextCheckDelay *= 2
        taskMgr.doMethodLater(self._nextCheckDelay, self._checkForLeaks,
                              self._getCheckTaskName())
        return task.done

    def _pruneContainerRefs(self, task=None):
        taskMgr.doMethodLater(self._pruneTaskPeriod, self._pruneContainerRefs,
                              self._getPruneTaskName())
        return task.done

