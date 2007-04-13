from pandac.PandaModules import PStatCollector
from direct.directnotify.DirectNotifyGlobal import directNotify
from direct.showbase.PythonUtil import Queue, invertDictLossless
from direct.showbase.PythonUtil import itype, serialNum, safeRepr, fastRepr
from direct.showbase.Job import Job
import types, weakref, random, __builtin__

def _createContainerLeak():
    def leakContainer(task):
        base = getBase()
        if not hasattr(base, 'leakContainer'):
            base.leakContainer = {}
        # use tuples as keys since they can't be weakref'd, and use an instance
        # since it can't be repr/eval'd
        # that will force the leak detector to hold a normal 'non-weak' reference
        class LeakKey:
            pass
        base.leakContainer[(LeakKey(),)] = {}
        # test the non-weakref object reference handling
        if random.random() < .01:
            key = random.choice(base.leakContainer.keys())
            ContainerLeakDetector.notify.debug(
                'removing reference to leakContainer key %s so it will be garbage-collected' % safeRepr(key))
            del base.leakContainer[key]
        return task.cont
    task = taskMgr.add(leakContainer, 'leakContainer-%s' % serialNum())

class NoDictKey:
    pass

class Indirection:
    """
    Represents the indirection that brings you from a container to an element of the container.
    Stored as a string to be used as part of an eval, or as a key to be looked up in a dict.
    Each dictionary dereference is individually eval'd since the dict key might have been
    garbage-collected
    TODO: store string components that are duplicates of strings in the actual system so that
    Python will keep one copy and reduce memory usage
    """

    def __init__(self, evalStr=None, dictKey=NoDictKey):
        # if this is a dictionary lookup, pass dictKey instead of evalStr
        self.evalStr = evalStr
        self.dictKey = NoDictKey
        # is the dictKey a weak reference?
        self._isWeakRef = False
        self._refCount = 0
        if dictKey is not NoDictKey:
            # if we can repr/eval the key, store it as an evalStr
            keyRepr = safeRepr(dictKey)
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
                    ContainerLeakDetector.notify.debug('could not weakref dict key %s' % keyRepr)
                    self.dictKey = dictKey
                    self._isWeakRef = False

    def destroy(self):
        # re-entrant
        self.dictKey = NoDictKey

    def acquire(self):
        self._refCount += 1
    def release(self):
        self._refCount -= 1
        if self._refCount == 0:
            self.destroy()

    def isDictKey(self):
        # is this an indirection through a dictionary?
        return self.dictKey is not NoDictKey

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
    collection of the container if possible
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
        # re-entrant
        for indirection in self._indirections:
            indirection.release()
        self._indirections = []

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
        # make sure the indirections don't go away on us
        indirections = self._indirections
        for indirection in indirections:
            indirection.acquire()
        for indirection in indirections:
            yield None
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
        for indirection in indirections:
            yield None
            indirection.release()

        yield self._evalWithObj(evalStr, curObj)
        
    def getNameGen(self):
        str = ''
        prevIndirection = None
        curIndirection = None
        nextIndirection = None
        # make sure the indirections don't go away on us
        indirections = self._indirections
        for indirection in indirections:
            indirection.acquire()
        for i in xrange(len(indirections)):
            yield None
            if i > 0:
                prevIndirection = indirections[i-1]
            else:
                prevIndirection = None
            curIndirection = indirections[i]
            if i < len(indirections)-1:
                nextIndirection = indirections[i+1]
            else:
                nextIndirection = None
            str += curIndirection.getString(prevIndirection=prevIndirection,
                                            nextIndirection=nextIndirection)
        for indirection in indirections:
            yield None
            indirection.release()
        yield str

    def __repr__(self):
        for result in self.getNameGen():
            pass
        return result

class FindContainers(Job):
    """
    Explore the Python graph, looking for objects that support __len__()
    """
    def __init__(self, name, leakDetector):
        Job.__init__(self, name)
        self._leakDetector = leakDetector
        self._id2ref = self._leakDetector._id2ref
        self.notify = self._leakDetector.notify
        ContainerLeakDetector.addPrivateObj(self.__dict__)

        # set up the base/starting object
        self._baseObjRef = ContainerRef(Indirection(evalStr='__builtin__.__dict__'))
        for i in self._nameContainerGen(__builtin__.__dict__, self._baseObjRef):
            pass
        try:
            base
        except:
            pass
        else:
            self._baseObjRef = ContainerRef(Indirection(evalStr='base.__dict__'))
            for i in self._nameContainerGen(base.__dict__, self._baseObjRef):
                pass
        try:
            simbase
        except:
            pass
        else:
            self._baseObjRef = ContainerRef(Indirection(evalStr='simbase.__dict__'))
            for i in self._nameContainerGen(simbase.__dict__, self._baseObjRef):
                pass

        self._curObjRef = self._baseObjRef

    def destroy(self):
        ContainerLeakDetector.removePrivateObj(self.__dict__)
        Job.destroy(self)

    def getPriority(self):
        return Job.Priorities.Low
    
    def _isDeadEnd(self, obj, objName=None):
        if type(obj) in (types.BooleanType, types.BuiltinFunctionType,
                         types.BuiltinMethodType, types.ComplexType,
                         types.FloatType, types.IntType, types.LongType,
                         types.NoneType, types.NotImplementedType,
                         types.TypeType, types.CodeType, types.FunctionType,
                         types.StringType, types.UnicodeType,
                         types.TupleType):
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

    def _nameContainerGen(self, cont, objRef):
        """
        if self.notify.getDebug():
            self.notify.debug('_nameContainer: %s' % objRef)
            #printStack()
            """
        contId = id(cont)
        # if this container is new, or the objRef repr is shorter than what we already have,
        # put it in the table
        if contId in self._id2ref:
            for existingRepr in self._id2ref[contId].getNameGen():
                yield None
            for newRepr in objRef.getNameGen():
                yield None
        if contId not in self._id2ref or len(newRepr) < len(existingRepr):
            if contId in self._id2ref:
                self._leakDetector.removeContainerById(contId)
            self._id2ref[contId] = objRef

    def run(self):
        try:
            while True:
                # yield up here instead of at the end, since we skip back to the
                # top of the while loop from various points
                yield None
                #import pdb;pdb.set_trace()
                curObj = None
                if self._curObjRef is None:
                    self._curObjRef = self._baseObjRef
                try:
                    for result in self._curObjRef.getContainer():
                        yield None
                    curObj = result
                except:
                    self.notify.debug('lost current container: %s' % self._curObjRef)
                    # that container is gone, try again
                    self._curObjRef = None
                    continue
                self.notify.debug('--> %s' % self._curObjRef)

                # keep a copy of this obj's eval str, it might not be in _id2ref
                curObjRef = self._curObjRef
                # if we hit a dead end, start over at a container we know about
                self._curObjRef = None

                if type(curObj) in (types.ModuleType, types.InstanceType):
                    child = curObj.__dict__
                    isContainer = self._isContainer(child)
                    notDeadEnd = not self._isDeadEnd(child)
                    if isContainer or notDeadEnd:
                        objRef = ContainerRef(Indirection(evalStr='.__dict__'), curObjRef)
                        yield None
                        if isContainer:
                            for i in self._nameContainerGen(child, objRef):
                                yield None
                        if notDeadEnd:
                            self._curObjRef = objRef
                    continue

                if type(curObj) is types.DictType:
                    key = None
                    attr = None
                    keys = curObj.keys()
                    # we will continue traversing the object graph via one key of the dict,
                    # choose it at random without taking a big chunk of CPU time
                    numKeysLeft = len(keys)
                    nextObjRef = None
                    for key in keys:
                        yield None
                        try:
                            attr = curObj[key]
                        except KeyError, e:
                            # this is OK because we are yielding during the iteration
                            self.notify.debug('could not index into %s with key %s' % (curObjRef, safeRepr(key)))
                            continue
                        isContainer = self._isContainer(attr)
                        notDeadEnd = False
                        if nextObjRef is None:
                            notDeadEnd = not self._isDeadEnd(attr, key)
                        if isContainer or notDeadEnd:
                            if curObj is __builtin__.__dict__:
                                objRef = ContainerRef(Indirection(evalStr=key))
                            else:
                                objRef = ContainerRef(Indirection(dictKey=key), curObjRef)
                            yield None
                            if isContainer:
                                for i in self._nameContainerGen(attr, objRef):
                                    yield None
                            if notDeadEnd and nextObjRef is None:
                                if random.randrange(numKeysLeft) == 0:
                                    nextObjRef = objRef
                            numKeysLeft -= 1
                    if nextObjRef is not None:
                        self._curObjRef = nextObjRef
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
                                yield None
                                try:
                                    attr = itr.next()
                                except:
                                    # some custom classes don't do well when iterated
                                    attr = None
                                    break
                                attrs.append(attr)
                            # we will continue traversing the object graph via one attr,
                            # choose it at random without taking a big chunk of CPU time
                            numAttrsLeft = len(attrs)
                            nextObjRef = None
                            for attr in attrs:
                                yield None
                                isContainer = self._isContainer(attr)
                                notDeadEnd = False
                                if nextObjRef is None:
                                    notDeadEnd = not self._isDeadEnd(attr)
                                if isContainer or notDeadEnd:
                                    objRef = ContainerRef(Indirection(evalStr='[%s]' % index), curObjRef)
                                    yield None
                                    if isContainer:
                                        for i in self._nameContainerGen(attr, objRef):
                                            yield None
                                    if notDeadEnd and nextObjRef is None:
                                        if random.randrange(numAttrsLeft) == 0:
                                            nextObjRef = objRef
                                numAttrsLeft -= 1
                                index += 1
                            if nextObjRef is not None:
                                self._curObjRef = nextObjRef
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
                    # we will continue traversing the object graph via one child,
                    # choose it at random without taking a big chunk of CPU time
                    numChildrenLeft = len(childNames)
                    nextObjRef = None
                    for childName in childNames:
                        yield None
                        child = getattr(curObj, childName)
                        isContainer = self._isContainer(child)
                        notDeadEnd = False
                        if nextObjRef is None:
                            notDeadEnd = not self._isDeadEnd(child, childName)
                        if isContainer or notDeadEnd:
                            objRef = ContainerRef(Indirection(evalStr='.%s' % childName), curObjRef)
                            yield None
                            if isContainer:
                                for i in self._nameContainerGen(child, objRef):
                                    yield None
                            if notDeadEnd and nextObjRef is None:
                                if random.randrange(numChildrenLeft) == 0:
                                    nextObjRef = objRef
                        numChildrenLeft -= 1
                    if nextObjRef is not None:
                        self._curObjRef = nextObjRef
                    del childName
                    del child
                    continue
        except Exception, e:
            print 'FindContainers job caught exception: %s' % e
            if __dev__:
                #raise e
                pass
            yield Job.done

class CheckContainers(Job):
    """
    Job to check container sizes and find potential leaks; sub-job of ContainerLeakDetector
    """
    ReprItems = 5
    
    def __init__(self, name, leakDetector, index):
        Job.__init__(self, name)
        self._leakDetector = leakDetector
        self.notify = self._leakDetector.notify
        self._index = index
        ContainerLeakDetector.addPrivateObj(self.__dict__)

    def destroy(self):
        ContainerLeakDetector.removePrivateObj(self.__dict__)
        Job.destroy(self)

    def getPriority(self):
        return Job.Priorities.Normal
    
    def run(self):
        try:
            self._leakDetector._index2containerId2len[self._index] = {}
            ids = self._leakDetector.getContainerIds()
            # record the current len of each container
            for id in ids:
                yield None
                try:
                    for result in self._leakDetector.getContainerByIdGen(id):
                        yield None
                    container = result
                except Exception, e:
                    # this container no longer exists
                    if self.notify.getDebug():
                        for contName in self._leakDetector.getContainerNameByIdGen(id):
                            yield None
                        self.notify.debug(
                            '%s no longer exists; caught exception in getContainerById (%s)' % (
                            contName, e))
                    self._leakDetector.removeContainerById(id)
                    continue
                if container is None:
                    # this container no longer exists
                    if self.notify.getDebug():
                        for contName in self._leakDetector.getContainerNameByIdGen(id):
                            yield None
                        self.notify.debug('%s no longer exists; getContainerById returned None' %
                                          contName)
                    self._leakDetector.removeContainerById(id)
                    continue
                try:
                    cLen = len(container)
                except Exception, e:
                    # this container no longer exists
                    if self.notify.getDebug():
                        for contName in self._leakDetector.getContainerNameByIdGen(id):
                            yield None
                        self.notify.debug(
                            '%s is no longer a container, it is now %s (%s)' %
                            (contName, safeRepr(container), e))
                    self._leakDetector.removeContainerById(id)
                    continue
                self._leakDetector._index2containerId2len[self._index][id] = cLen
            # compare the current len of each container to past lens
            if self._index > 0:
                oashdfjkahsdf
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
                                if idx2id2len[self._index-1][id] != 0:
                                    percent = 100. * (float(diff) / float(idx2id2len[self._index-1][id]))
                                    try:
                                        for container in self._leakDetector.getContainerByIdGen(id):
                                            yield None
                                    except:
                                        # TODO
                                        self.notify.debug('caught exception in getContainerByIdGen (1)')
                                    else:
                                        self.notify.warning(
                                            '%s (%s) grew %.2f%% in %.2f minutes (%s items at last measurement, current contents: %s)' % (
                                            name, itype(container), percent, minutes, idx2id2len[self._index][id],
                                            fastRepr(container, maxLen=CheckContainers.ReprItems)))
                                    yield None
                        if (self._index > 2 and
                            id in idx2id2len[self._index-2] and
                            id in idx2id2len[self._index-3]):
                            diff2 = idx2id2len[self._index-1][id] - idx2id2len[self._index-2][id]
                            diff3 = idx2id2len[self._index-2][id] - idx2id2len[self._index-3][id]
                            if self._index <= 4:
                                if diff > 0 and diff2 > 0 and diff3 > 0:
                                    name = self._leakDetector.getContainerNameById(id)
                                    try:
                                        for container in self._leakDetector.getContainerByIdGen(id):
                                            yield None
                                    except:
                                        # TODO
                                        self.notify.debug('caught exception in getContainerByIdGen (2)')
                                    else:
                                        msg = ('%s (%s) consistently increased in size over the last '
                                               '3 periods (%s items at last measurement, current contents: %s)' %
                                               (name, itype(container), idx2id2len[self._index][id],
                                                fastRepr(container, maxLen=CheckContainers.ReprItems)))
                                        self.notify.warning(msg)
                                    yield None
                            elif (id in idx2id2len[self._index-4] and
                                  id in idx2id2len[self._index-5]):
                                # if size has consistently increased over the last 5 checks,
                                # send out a warning
                                diff4 = idx2id2len[self._index-3][id] - idx2id2len[self._index-4][id]
                                diff5 = idx2id2len[self._index-4][id] - idx2id2len[self._index-5][id]
                                if diff > 0 and diff2 > 0 and diff3 > 0 and diff4 > 0 and diff5 > 0:
                                    name = self._leakDetector.getContainerNameById(id)
                                    try:
                                        for container in self._leakDetector.getContainerByIdGen(id):
                                            yield None
                                    except:
                                        # TODO
                                        self.notify.debug('caught exception in getContainerByIdGen (3)')
                                    else:
                                        msg = ('%s (%s) consistently increased in size over the last '
                                               '5 periods (%s items at last measurement, current contents: %s)' %
                                               (name, itype(container), idx2id2len[self._index][id],
                                                fastRepr(container, maxLen=CheckContainers.ReprItems)))
                                        self.notify.warning(msg)
                                        self.notify.info('sending notification...')
                                        yield None
                                        messenger.send(self._leakDetector.getLeakEvent(), [container, name])
        except Exception, e:
            print 'CheckContainers job caught exception: %s' % e
            if __dev__:
                #raise e
                pass
        yield Job.Done

class PruneContainerRefs(Job):
    """
    Job to destroy any container refs that are no longer valid.
    Checks validity by asking for each container
    """
    def __init__(self, name, leakDetector):
        Job.__init__(self, name)
        self._leakDetector = leakDetector
        self.notify = self._leakDetector.notify
        ContainerLeakDetector.addPrivateObj(self.__dict__)

    def destroy(self):
        ContainerLeakDetector.removePrivateObj(self.__dict__)
        Job.destroy(self)

    def getPriority(self):
        return Job.Priorities.Normal
    
    def run(self):
        try:
            ids = self._leakDetector.getContainerIds()
            for id in ids:
                yield None
                try:
                    for result in self._leakDetector.getContainerByIdGen(id):
                        yield None
                    container = result
                except:
                    # reference is invalid, remove it
                    self._leakDetector.removeContainerById(id)
        except Exception, e:
            print 'PruneContainerRefs job caught exception: %s' % e
            if __dev__:
                #raise e
                pass
        yield Job.Done

class ContainerLeakDetector(Job):
    """
    Low-priority Python object-graph walker that looks for leaking containers.
    To reduce memory usage, this does a random walk of the Python objects to
    discover containers rather than keep a set of all visited objects; it may
    visit the same object many times but eventually it will discover every object.
    Checks container sizes at ever-increasing intervals.
    """
    notify = directNotify.newCategory("ContainerLeakDetector")
    # set of containers that should not be examined
    PrivateIds = set()

    def __init__(self, name, firstCheckDelay = None):
        Job.__init__(self, name)
        self._serialNum = serialNum()

        self._findContainersJob = None
        self._checkContainersJob = None
        self._pruneContainersJob = None

        if firstCheckDelay is None:
            firstCheckDelay = 60. * (15./2)
        self._nextCheckDelay = firstCheckDelay
        self._pruneTaskPeriod = config.GetFloat('leak-detector-prune-period', 60. * 30.)

        # main dict of id(container)->containerRef
        self._id2ref = {}
        # storage for results of check-container job
        self._index2containerId2len = {}
        self._index2delay = {}

        if config.GetBool('leak-container', 0):
            _createContainerLeak()

        # don't check our own tables for leaks
        ContainerLeakDetector.addPrivateObj(ContainerLeakDetector.PrivateIds)
        ContainerLeakDetector.addPrivateObj(self.__dict__)

        jobMgr.add(self)

    def destroy(self):
        self.ignoreAll()
        if self._pruneContainersJob is not None:
            jobMgr.remove(self._pruneContainersJob)
            self._pruneContainersJob = None
        if self._checkContainersJob is not None:
            jobMgr.remove(self._checkContainersJob)
            self._checkContainersJob = None
        jobMgr.remove(self._findContainersJob)
        self._findContainersJob = None
        del self._id2ref
        del self._index2containerId2len
        del self._index2delay

    def getLeakEvent(self):
        # passes description string as argument
        return 'containerLeakDetected-%s' % self._serialNum

    def getPriority(self):
        return Job.Priorities.Min

    @classmethod
    def addPrivateObj(cls, obj):
        cls.PrivateIds.add(id(obj))
    @classmethod
    def removePrivateObj(cls, obj):
        cls.PrivateIds.remove(id(obj))

    def _getCheckTaskName(self):
        return 'checkForLeakingContainers-%s' % self._serialNum
    def _getPruneTaskName(self):
        return 'pruneLeakingContainerRefs-%s' % self._serialNum

    def getContainerIds(self):
        return self._id2ref.keys()

    def getContainerByIdGen(self, id):
        # return a generator to look up a container
        return self._id2ref[id].getContainer()
    def getContainerById(self, id):
        for result in self._id2ref[id].getContainer():
            pass
        return result
    def getContainerNameByIdGen(self, id):
        return self._id2ref[id].getNameGen()
    def getContainerNameById(self, id):
        if id in self._id2ref:
            return repr(self._id2ref[id])
        return '<unknown container>'
    def removeContainerById(self, id):
        if id in self._id2ref:
            self._id2ref[id].destroy()
            del self._id2ref[id]

    def run(self):
        # start looking for containers
        self._findContainersJob = FindContainers(
            '%s-findContainers' % self.getJobName(), self)
        jobMgr.add(self._findContainersJob)

        self._scheduleNextLeakCheck()
        self._scheduleNextPruning()

        while True:
            yield Job.Sleep
        
    def _scheduleNextLeakCheck(self):
        taskMgr.doMethodLater(self._nextCheckDelay, self._checkForLeaks,
                              self._getCheckTaskName())
        # delay between checks
        # fib:    1   1   2   3   5   8   13   21   34   55   89
        # * 2.:   1   2   4   8  16  32   64  128  256  512 1024
        # * 1.5:  1 1.5 2.3 3.4 5.1 7.6 11.4 17.1 25.6 38.4 57.7
        #
        # delay from job start
        # fib:    1   2    4    7   12   20    33    54    88    143    232
        # * 2.:   1   3    7   15   31   63   127   255   511   1023   2047
        # * 1.5:  1 2.5 4.75  8.1 13.2 20.8  32.2  49.3  74.9  113.3    171
        self._nextCheckDelay = self._nextCheckDelay * 1.5

    def _checkForLeaks(self, task=None):
        self._index2delay[len(self._index2containerId2len)] = self._nextCheckDelay
        self._checkContainersJob = CheckContainers(
            '%s-checkForLeaks' % self.getJobName(), self, len(self._index2containerId2len))
        self.acceptOnce(self._checkContainersJob.getFinishedEvent(),
                        self._scheduleNextLeakCheck)
        jobMgr.add(self._checkContainersJob)
        return task.done

    def _scheduleNextPruning(self):
        taskMgr.doMethodLater(self._pruneTaskPeriod, self._pruneContainerRefs,
                              self._getPruneTaskName())

    def _pruneContainerRefs(self, task=None):
        self._pruneContainersJob = PruneContainerRefs(
            '%s-pruneContainerRefs' % self.getJobName(), self)
        self.acceptOnce(self._pruneContainersJob.getFinishedEvent(),
                        self._scheduleNextPruning)
        jobMgr.add(self._pruneContainersJob)
        return task.done
