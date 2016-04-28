from direct.directnotify.DirectNotifyGlobal import directNotify
from direct.showbase.PythonUtil import makeFlywheelGen
from direct.showbase.PythonUtil import itype, serialNum, safeRepr, fastRepr
from direct.showbase.Job import Job
import types, weakref, random, sys

if sys.version_info >= (3, 0):
    import builtins as __builtin__

    intTypes = (int,)
    deadEndTypes = (bool, types.BuiltinFunctionType,
                    types.BuiltinMethodType, complex,
                    float, int,
                    type(None), type(NotImplemented),
                    type, types.CodeType, types.FunctionType,
                    bytes, str, tuple)
else:
    import __builtin__

    intTypes = (int, long)
    deadEndTypes = (types.BooleanType, types.BuiltinFunctionType,
                    types.BuiltinMethodType, types.ComplexType,
                    types.FloatType, types.IntType, types.LongType,
                    types.NoneType, types.NotImplementedType,
                    types.TypeType, types.CodeType, types.FunctionType,
                    types.StringType, types.UnicodeType, types.TupleType)


def _createContainerLeak():
    def leakContainer(task=None):
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
            key = random.choice(list(base.leakContainer.keys()))
            ContainerLeakDetector.notify.debug(
                'removing reference to leakContainer key %s so it will be garbage-collected' % safeRepr(key))
            del base.leakContainer[key]
        taskMgr.doMethodLater(10, leakContainer, 'leakContainer-%s' % serialNum())
        if task:
            return task.done
    leakContainer()

def _createTaskLeak():
    leakTaskName = uniqueName('leakedTask')
    leakDoLaterName = uniqueName('leakedDoLater')
    def nullTask(task=None):
        return task.cont
    def nullDoLater(task=None):
        return task.done
    def leakTask(task=None, leakTaskName=leakTaskName):
        base = getBase()
        taskMgr.add(nullTask, uniqueName(leakTaskName))
        taskMgr.doMethodLater(1 << 31, nullDoLater, uniqueName(leakDoLaterName))
        taskMgr.doMethodLater(10, leakTask, 'doLeakTask-%s' % serialNum())
        if task:
            return task.done
    leakTask()

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
                except TypeError as e:
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

class ObjectRef:
    """
    stores a reference to a container in a way that does not prevent garbage
    collection of the container if possible
    stored as a series of 'indirections' (obj.foo -> '.foo', dict[key] -> '[key]', etc.)
    """
    notify = directNotify.newCategory("ObjectRef")

    class FailedEval(Exception):
        pass

    def __init__(self, indirection, objId, other=None):
        self._indirections = []
        # are we building off of an existing ref?
        if other is not None:
            for ind in other._indirections:
                self._indirections.append(ind)

        # make sure we're not storing a reference to the actual object,
        # that could cause a memory leak
        assert type(objId) in intTypes
        # prevent cycles (i.e. base.loader.base.loader)
        assert not self.goesThrough(objId=objId)

        self._indirections.append(indirection)

        # make sure our indirections don't get destroyed while we're using them
        for ind in self._indirections:
            ind.acquire()
        self.notify.debug(repr(self))

    def destroy(self):
        for indirection in self._indirections:
            indirection.release()
        del self._indirections

    def getNumIndirections(self):
        return len(self._indirections)

    def goesThroughGen(self, obj=None, objId=None):
        if obj is None:
            assert type(objId) in intTypes
        else:
            objId = id(obj)
        o = None
        evalStr = ''
        curObj = None
        # make sure the indirections don't go away on us
        indirections = self._indirections
        for indirection in indirections:
            yield None
            indirection.acquire()
        for indirection in indirections:
            yield None
            if not indirection.isDictKey():
                # build up a string to be eval'd
                evalStr += indirection.getString()
            else:
                curObj = self._getContainerByEval(evalStr, curObj=curObj)
                if curObj is None:
                    raise FailedEval(evalStr)
                # try to look up this key in the curObj dictionary
                curObj = indirection.dereferenceDictKey(curObj)
                evalStr = ''
            yield None
            o = self._getContainerByEval(evalStr, curObj=curObj)
            if id(o) == objId:
                break
        for indirection in indirections:
            yield None
            indirection.release()

        yield id(o) == objId

    def goesThrough(self, obj=None, objId=None):
        # since we cache the ids involved in this reference,
        # this isn't perfect, for example if base.myObject is reassigned
        # to a different object after this Ref was created this would return
        # false, allowing a ref to base.myObject.otherObject.myObject
        for goesThrough in self.goesThroughGen(obj=obj, objId=objId):
            pass
        return goesThrough

    def _getContainerByEval(self, evalStr, curObj=None):
        if curObj is not None:
            # eval('curObj.foo.bar.someDict')
            evalStr = 'curObj%s' % evalStr
        else:
            # this eval is not based off of curObj, use the global__builtin__ namespace
            # put __builtin__ at the start if it's not already there
            bis = '__builtin__'
            if evalStr[:len(bis)] != bis:
                evalStr = '%s.%s' % (bis, evalStr)
        try:
            container = eval(evalStr)
        except NameError as ne:
            return None
        except AttributeError as ae:
            return None
        except KeyError as ke:
            return None
        return container

    def getContainerGen(self, getInstance=False):
        # try to get a handle on the container by eval'ing and looking things
        # up in dictionaries, depending on the type of each indirection
        # if getInstance is True, will return instance instead of instance dict
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
                curObj = self._getContainerByEval(evalStr, curObj=curObj)
                if curObj is None:
                    raise FailedEval(evalStr)
                # try to look up this key in the curObj dictionary
                curObj = indirection.dereferenceDictKey(curObj)
                evalStr = ''
        for indirection in indirections:
            yield None
            indirection.release()

        if getInstance:
            lenDict = len('.__dict__')
            if evalStr[-lenDict:] == '.__dict__':
                evalStr = evalStr[:-lenDict]

        # TODO: check that this is still the object we originally pointed to
        yield self._getContainerByEval(evalStr, curObj=curObj)

    def getEvalStrGen(self, getInstance=False):
        str = ''
        prevIndirection = None
        curIndirection = None
        nextIndirection = None
        # make sure the indirections don't go away on us
        indirections = self._indirections
        for indirection in indirections:
            indirection.acquire()
        for i in range(len(indirections)):
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

        if getInstance:
            lenDict = len('.__dict__')
            if str[-lenDict:] == '.__dict__':
                str = str[:-lenDict]

        for indirection in indirections:
            yield None
            indirection.release()
        yield str

    def getFinalIndirectionStr(self):
        prevIndirection = None
        if len(self._indirections) > 1:
            prevIndirection = self._indirections[-2]
        return self._indirections[-1].getString(prevIndirection=prevIndirection)

    def __repr__(self):
        for result in self.getEvalStrGen():
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
        # these hold objects that we should start traversals from often and not-as-often,
        # respectively
        self._id2baseStartRef = {}
        self._id2discoveredStartRef = {}
        # these are working copies so that our iterations aren't disturbed by changes to the
        # definitive ref sets
        self._baseStartRefWorkingList = ScratchPad(refGen=nullGen(),
                                                   source=self._id2baseStartRef)
        self._discoveredStartRefWorkingList = ScratchPad(refGen=nullGen(),
                                                         source=self._id2discoveredStartRef)
        self.notify = self._leakDetector.notify
        ContainerLeakDetector.addPrivateObj(self.__dict__)

        # set up the base containers, the ones that hold most objects
        ref = ObjectRef(Indirection(evalStr='__builtin__.__dict__'), id(__builtin__.__dict__))
        self._id2baseStartRef[id(__builtin__.__dict__)] = ref
        # container for objects that want to make sure they are found by
        # the object exploration algorithm, including objects that exist
        # just to measure things such as C++ memory usage, scene graph size,
        # framerate, etc. See LeakDetectors.py
        if not hasattr(__builtin__, "leakDetectors"):
            __builtin__.leakDetectors = {}
        ref = ObjectRef(Indirection(evalStr='leakDetectors'), id(leakDetectors))
        self._id2baseStartRef[id(leakDetectors)] = ref
        for i in self._addContainerGen(__builtin__.__dict__, ref):
            pass
        try:
            base
        except:
            pass
        else:
            ref = ObjectRef(Indirection(evalStr='base.__dict__'), id(base.__dict__))
            self._id2baseStartRef[id(base.__dict__)] = ref
            for i in self._addContainerGen(base.__dict__, ref):
                pass
        try:
            simbase
        except:
            pass
        else:
            ref = ObjectRef(Indirection(evalStr='simbase.__dict__'), id(simbase.__dict__))
            self._id2baseStartRef[id(simbase.__dict__)] = ref
            for i in self._addContainerGen(simbase.__dict__, ref):
                pass

    def destroy(self):
        ContainerLeakDetector.removePrivateObj(self.__dict__)
        Job.destroy(self)

    def getPriority(self):
        return Job.Priorities.Low

    @staticmethod
    def getStartObjAffinity(startObj):
        # how good of a starting object is this object for traversing the object graph?
        try:
            return len(startObj)
        except:
            return 1

    def _isDeadEnd(self, obj, objName=None):
        if type(obj) in deadEndTypes:
            return True

        # if it's an internal object, ignore it
        if id(obj) in ContainerLeakDetector.PrivateIds:
            return True
        # prevent crashes in objects that define __cmp__ and don't handle strings
        if type(objName) == str and objName in ('im_self', 'im_class'):
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

    def _hasLength(self, obj):
        return hasattr(obj, '__len__')

    def _addContainerGen(self, cont, objRef):
        contId = id(cont)
        # if this container is new, or the objRef repr is shorter than what we already have,
        # put it in the table
        if contId in self._id2ref:
            for existingRepr in self._id2ref[contId].getEvalStrGen():
                yield None
            for newRepr in objRef.getEvalStrGen():
                yield None
        if contId not in self._id2ref or len(newRepr) < len(existingRepr):
            if contId in self._id2ref:
                self._leakDetector.removeContainerById(contId)
            self._id2ref[contId] = objRef

    def _addDiscoveredStartRef(self, obj, ref):
        # we've discovered an object that can be used to start an object graph traversal
        objId = id(obj)
        if objId in self._id2discoveredStartRef:
            existingRef = self._id2discoveredStartRef[objId]
            if type(existingRef) not in intTypes:
                if (existingRef.getNumIndirections() >=
                    ref.getNumIndirections()):
                    # the ref that we already have is more concise than the new ref
                    return
        if objId in self._id2ref:
            if (self._id2ref[objId].getNumIndirections() >=
                ref.getNumIndirections()):
                # the ref that we already have is more concise than the new ref
                return
        storedItem = ref
        # if we already are storing a reference to this object, don't store a second reference
        if objId in self._id2ref:
            storedItem = objId
        self._id2discoveredStartRef[objId] = storedItem

    def run(self):
        try:
            # this yields a different set of start refs every time we start a new traversal
            # force creation of a new workingListSelector inside the while loop right off the bat
            workingListSelector = nullGen()
            # this holds the current step of the current traversal
            curObjRef = None
            while True:
                # yield up here instead of at the end, since we skip back to the
                # top of the while loop from various points
                yield None
                #import pdb;pdb.set_trace()
                if curObjRef is None:
                    # choose an object to start a traversal from
                    try:
                        startRefWorkingList = next(workingListSelector)
                    except StopIteration:
                        # do relative # of traversals on each set based on how many refs it contains
                        baseLen = len(self._baseStartRefWorkingList.source)
                        discLen = len(self._discoveredStartRefWorkingList.source)
                        minLen = float(max(1, min(baseLen, discLen)))
                        # this will cut down the traversals of the larger set by 2/3
                        minLen *= 3.
                        workingListSelector = flywheel([self._baseStartRefWorkingList, self._discoveredStartRefWorkingList],
                                                       [baseLen/minLen, discLen/minLen])
                        yield None
                        continue

                    # grab the next start ref from this sequence and see if it's still valid
                    while True:
                        yield None
                        try:
                            curObjRef = next(startRefWorkingList.refGen)
                            break
                        except StopIteration:
                            # we've run out of refs, grab a new set
                            if len(startRefWorkingList.source) == 0:
                                # ref set is empty, choose another
                                break
                            # make a generator that yields containers a # of times that is
                            # proportional to their length
                            for fw in makeFlywheelGen(
                                list(startRefWorkingList.source.values()),
                                countFunc=lambda x: self.getStartObjAffinity(x),
                                scale=.05):
                                yield None
                            startRefWorkingList.refGen = fw
                    if curObjRef is None:
                        # this ref set is empty, choose another
                        # the base set should never be empty (__builtin__ etc.)
                        continue
                    # do we need to go look up the object in _id2ref? sometimes we do that
                    # to avoid storing multiple redundant refs to a single item
                    if type(curObjRef) in intTypes:
                        startId = curObjRef
                        curObjRef = None
                        try:
                            for containerRef in self._leakDetector.getContainerByIdGen(startId):
                                yield None
                        except:
                            # ref is invalid
                            self.notify.debug('invalid startRef, stored as id %s' % startId)
                            self._leakDetector.removeContainerById(startId)
                            continue
                        curObjRef = containerRef

                try:
                    for curObj in curObjRef.getContainerGen():
                        yield None
                except:
                    self.notify.debug('lost current container, ref.getContainerGen() failed')
                    # that container is gone, try again
                    curObjRef = None
                    continue

                self.notify.debug('--> %s' % curObjRef)
                #import pdb;pdb.set_trace()

                # store a copy of the current objRef
                parentObjRef = curObjRef
                # if we hit a dead end, start over from another container
                curObjRef = None

                if hasattr(curObj, '__dict__'):
                    child = curObj.__dict__
                    hasLength = self._hasLength(child)
                    notDeadEnd = not self._isDeadEnd(child)
                    if hasLength or notDeadEnd:
                        # prevent cycles in the references (i.e. base.loader.base)
                        for goesThrough in parentObjRef.goesThroughGen(child):
                            # don't yield, container might lose this element
                            pass
                        if not goesThrough:
                            objRef = ObjectRef(Indirection(evalStr='.__dict__'),
                                               id(child), parentObjRef)
                            yield None
                            if hasLength:
                                for i in self._addContainerGen(child, objRef):
                                    yield None
                            if notDeadEnd:
                                self._addDiscoveredStartRef(child, objRef)
                                curObjRef = objRef
                    continue

                if type(curObj) is dict:
                    key = None
                    attr = None
                    keys = list(curObj.keys())
                    # we will continue traversing the object graph via one key of the dict,
                    # choose it at random without taking a big chunk of CPU time
                    numKeysLeft = len(keys) + 1
                    for key in keys:
                        yield None
                        numKeysLeft -= 1
                        try:
                            attr = curObj[key]
                        except KeyError as e:
                            # this is OK because we are yielding during the iteration
                            self.notify.debug('could not index into %s with key %s' % (
                                parentObjRef, safeRepr(key)))
                            continue
                        hasLength = self._hasLength(attr)
                        notDeadEnd = False
                        # if we haven't picked the next ref, check if this one is a candidate
                        if curObjRef is None:
                            notDeadEnd = not self._isDeadEnd(attr, key)
                        if hasLength or notDeadEnd:
                            # prevent cycles in the references (i.e. base.loader.base)
                            for goesThrough in parentObjRef.goesThroughGen(curObj[key]):
                                # don't yield, container might lose this element
                                pass
                            if not goesThrough:
                                if curObj is __builtin__.__dict__:
                                    objRef = ObjectRef(Indirection(evalStr='%s' % key),
                                                       id(curObj[key]))
                                else:
                                    objRef = ObjectRef(Indirection(dictKey=key),
                                                       id(curObj[key]), parentObjRef)
                                yield None
                                if hasLength:
                                    for i in self._addContainerGen(attr, objRef):
                                        yield None
                                if notDeadEnd:
                                    self._addDiscoveredStartRef(attr, objRef)
                                    if curObjRef is None and random.randrange(numKeysLeft) == 0:
                                        curObjRef = objRef
                    del key
                    del attr
                    continue

                    try:
                        childNames = dir(curObj)
                    except:
                        pass
                    else:
                        try:
                            index = -1
                            attrs = []
                            while 1:
                                yield None
                                try:
                                    attr = next(itr)
                                except:
                                    # some custom classes don't do well when iterated
                                    attr = None
                                    break
                                attrs.append(attr)
                            # we will continue traversing the object graph via one attr,
                            # choose it at random without taking a big chunk of CPU time
                            numAttrsLeft = len(attrs) + 1
                            for attr in attrs:
                                yield None
                                index += 1
                                numAttrsLeft -= 1
                                hasLength = self._hasLength(attr)
                                notDeadEnd = False
                                if curObjRef is None:
                                    notDeadEnd = not self._isDeadEnd(attr)
                                if hasLength or notDeadEnd:
                                    # prevent cycles in the references (i.e. base.loader.base)
                                    for goesThrough in parentObjRef.goesThrough(curObj[index]):
                                        # don't yield, container might lose this element
                                        pass
                                    if not goesThrough:
                                        objRef = ObjectRef(Indirection(evalStr='[%s]' % index),
                                                           id(curObj[index]), parentObjRef)
                                        yield None
                                        if hasLength:
                                            for i in self._addContainerGen(attr, objRef):
                                                yield None
                                        if notDeadEnd:
                                            self._addDiscoveredStartRef(attr, objRef)
                                            if curObjRef is None and random.randrange(numAttrsLeft) == 0:
                                                curObjRef = objRef
                            del attr
                        except StopIteration as e:
                            pass
                        del itr
                        continue

        except Exception as e:
            print('FindContainers job caught exception: %s' % e)
            if __dev__:
                raise
        yield Job.Done

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
            for objId in ids:
                yield None
                try:
                    for result in self._leakDetector.getContainerByIdGen(objId):
                        yield None
                    container = result
                except Exception as e:
                    # this container no longer exists
                    if self.notify.getDebug():
                        for contName in self._leakDetector.getContainerNameByIdGen(objId):
                            yield None
                        self.notify.debug(
                            '%s no longer exists; caught exception in getContainerById (%s)' % (
                            contName, e))
                    self._leakDetector.removeContainerById(objId)
                    continue
                if container is None:
                    # this container no longer exists
                    if self.notify.getDebug():
                        for contName in self._leakDetector.getContainerNameByIdGen(objId):
                            yield None
                        self.notify.debug('%s no longer exists; getContainerById returned None' %
                                          contName)
                    self._leakDetector.removeContainerById(objId)
                    continue
                try:
                    cLen = len(container)
                except Exception as e:
                    # this container no longer exists
                    if self.notify.getDebug():
                        for contName in self._leakDetector.getContainerNameByIdGen(objId):
                            yield None
                        self.notify.debug(
                            '%s is no longer a container, it is now %s (%s)' %
                            (contName, safeRepr(container), e))
                    self._leakDetector.removeContainerById(objId)
                    continue
                self._leakDetector._index2containerId2len[self._index][objId] = cLen
            # compare the current len of each container to past lens
            if self._index > 0:
                idx2id2len = self._leakDetector._index2containerId2len
                for objId in idx2id2len[self._index]:
                    yield None
                    if objId in idx2id2len[self._index-1]:
                        diff = idx2id2len[self._index][objId] - idx2id2len[self._index-1][objId]
                        """
                        # this check is too spammy
                        if diff > 20:
                            if diff > idx2id2len[self._index-1][objId]:
                                minutes = (self._leakDetector._index2delay[self._index] -
                                           self._leakDetector._index2delay[self._index-1]) / 60.
                                name = self._leakDetector.getContainerNameById(objId)
                                if idx2id2len[self._index-1][objId] != 0:
                                    percent = 100. * (float(diff) / float(idx2id2len[self._index-1][objId]))
                                    try:
                                        for container in self._leakDetector.getContainerByIdGen(objId):
                                            yield None
                                    except:
                                        # TODO
                                        self.notify.debug('caught exception in getContainerByIdGen (1)')
                                    else:
                                        self.notify.warning(
                                            '%s (%s) grew %.2f%% in %.2f minutes (%s items at last measurement, current contents: %s)' % (
                                            name, itype(container), percent, minutes, idx2id2len[self._index][objId],
                                            fastRepr(container, maxLen=CheckContainers.ReprItems)))
                                    yield None
                                    """
                        if (self._index > 2 and
                            objId in idx2id2len[self._index-2] and
                            objId in idx2id2len[self._index-3]):
                            diff2 = idx2id2len[self._index-1][objId] - idx2id2len[self._index-2][objId]
                            diff3 = idx2id2len[self._index-2][objId] - idx2id2len[self._index-3][objId]
                            if self._index <= 4:
                                if diff > 0 and diff2 > 0 and diff3 > 0:
                                    name = self._leakDetector.getContainerNameById(objId)
                                    try:
                                        for container in self._leakDetector.getContainerByIdGen(objId):
                                            yield None
                                    except:
                                        # TODO
                                        self.notify.debug('caught exception in getContainerByIdGen (2)')
                                    else:
                                        msg = ('%s (%s) consistently increased in size over the last '
                                               '3 periods (%s items at last measurement, current contents: %s)' %
                                               (name, itype(container), idx2id2len[self._index][objId],
                                                fastRepr(container, maxLen=CheckContainers.ReprItems)))
                                        self.notify.warning(msg)
                                    yield None
                            elif (objId in idx2id2len[self._index-4] and
                                  objId in idx2id2len[self._index-5]):
                                # if size has consistently increased over the last 5 checks,
                                # send out a warning
                                diff4 = idx2id2len[self._index-3][objId] - idx2id2len[self._index-4][objId]
                                diff5 = idx2id2len[self._index-4][objId] - idx2id2len[self._index-5][objId]
                                if diff > 0 and diff2 > 0 and diff3 > 0 and diff4 > 0 and diff5 > 0:
                                    name = self._leakDetector.getContainerNameById(objId)
                                    try:
                                        for container in self._leakDetector.getContainerByIdGen(objId):
                                            yield None
                                    except:
                                        # TODO
                                        self.notify.debug('caught exception in getContainerByIdGen (3)')
                                    else:
                                        msg = ('leak detected: %s (%s) consistently increased in size over the last '
                                               '5 periods (%s items at last measurement, current contents: %s)' %
                                               (name, itype(container), idx2id2len[self._index][objId],
                                                fastRepr(container, maxLen=CheckContainers.ReprItems)))
                                        self.notify.warning(msg)
                                        yield None
                                        messenger.send(self._leakDetector.getLeakEvent(), [container, name])
                                        if config.GetBool('pdb-on-leak-detect', 0):
                                            import pdb;pdb.set_trace()
                                            pass
        except Exception as e:
            print('CheckContainers job caught exception: %s' % e)
            if __dev__:
                raise
        yield Job.Done

class FPTObjsOfType(Job):
    def __init__(self, name, leakDetector, otn, doneCallback=None):
        Job.__init__(self, name)
        self._leakDetector = leakDetector
        self.notify = self._leakDetector.notify
        self._otn = otn
        self._doneCallback = doneCallback
        self._ldde = self._leakDetector._getDestroyEvent()
        self.accept(self._ldde, self._handleLDDestroy)
        ContainerLeakDetector.addPrivateObj(self.__dict__)

    def destroy(self):
        self.ignore(self._ldde)
        self._leakDetector = None
        self._doneCallback = None
        ContainerLeakDetector.removePrivateObj(self.__dict__)
        Job.destroy(self)

    def _handleLDDestroy(self):
        self.destroy()

    def getPriority(self):
        return Job.Priorities.High

    def run(self):
        ids = self._leakDetector.getContainerIds()
        try:
            for id in ids:
                getInstance = (self._otn.lower() not in 'dict')
                yield None
                try:
                    for container in self._leakDetector.getContainerByIdGen(
                        id, getInstance=getInstance):
                        yield None
                except:
                    pass
                else:
                    if hasattr(container, '__class__'):
                        cName = container.__class__.__name__
                    else:
                        cName = container.__name__
                    if (self._otn.lower() in cName.lower()):
                        try:
                            for ptc in self._leakDetector.getContainerNameByIdGen(
                                id, getInstance=getInstance):
                                yield None
                        except:
                            pass
                        else:
                            print('GPTC(' + self._otn + '):' + self.getJobName() + ': ' + ptc)
        except Exception as e:
            print('FPTObjsOfType job caught exception: %s' % e)
            if __dev__:
                raise
        yield Job.Done

    def finished(self):
        if self._doneCallback:
            self._doneCallback(self)

class FPTObjsNamed(Job):
    def __init__(self, name, leakDetector, on, doneCallback=None):
        Job.__init__(self, name)
        self._leakDetector = leakDetector
        self.notify = self._leakDetector.notify
        self._on = on
        self._doneCallback = doneCallback
        self._ldde = self._leakDetector._getDestroyEvent()
        self.accept(self._ldde, self._handleLDDestroy)
        ContainerLeakDetector.addPrivateObj(self.__dict__)

    def destroy(self):
        self.ignore(self._ldde)
        self._leakDetector = None
        self._doneCallback = None
        ContainerLeakDetector.removePrivateObj(self.__dict__)
        Job.destroy(self)

    def _handleLDDestroy(self):
        self.destroy()

    def getPriority(self):
        return Job.Priorities.High

    def run(self):
        ids = self._leakDetector.getContainerIds()
        try:
            for id in ids:
                yield None
                try:
                    for container in self._leakDetector.getContainerByIdGen(id):
                        yield None
                except:
                    pass
                else:
                    name = self._leakDetector._id2ref[id].getFinalIndirectionStr()
                    if self._on.lower() in name.lower():
                        try:
                            for ptc in self._leakDetector.getContainerNameByIdGen(id):
                                yield None
                        except:
                            pass
                        else:
                            print('GPTCN(' + self._on + '):' + self.getJobName() + ': ' + ptc)
        except Exception as e:
            print('FPTObjsNamed job caught exception: %s' % e)
            if __dev__:
                raise
        yield Job.Done

    def finished(self):
        if self._doneCallback:
            self._doneCallback(self)

class PruneObjectRefs(Job):
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
                    for container in self._leakDetector.getContainerByIdGen(id):
                        yield None
                except:
                    # reference is invalid, remove it
                    self._leakDetector.removeContainerById(id)
            _id2baseStartRef = self._leakDetector._findContainersJob._id2baseStartRef
            ids = list(_id2baseStartRef.keys())
            for id in ids:
                yield None
                try:
                    for container in _id2baseStartRef[id].getContainerGen():
                        yield None
                except:
                    # reference is invalid, remove it
                    del _id2baseStartRef[id]
            _id2discoveredStartRef = self._leakDetector._findContainersJob._id2discoveredStartRef
            ids = list(_id2discoveredStartRef.keys())
            for id in ids:
                yield None
                try:
                    for container in _id2discoveredStartRef[id].getContainerGen():
                        yield None
                except:
                    # reference is invalid, remove it
                    del _id2discoveredStartRef[id]
        except Exception as e:
            print('PruneObjectRefs job caught exception: %s' % e)
            if __dev__:
                raise
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
            firstCheckDelay = 60. * 15.
        # divide by two, since the first check just takes length measurements and
        # doesn't check for leaks
        self._nextCheckDelay = firstCheckDelay/2.
        self._checkDelayScale = config.GetFloat('leak-detector-check-delay-scale', 1.5)
        self._pruneTaskPeriod = config.GetFloat('leak-detector-prune-period', 60. * 30.)

        # main dict of id(container)->containerRef
        self._id2ref = {}
        # storage for results of check-container job
        self._index2containerId2len = {}
        self._index2delay = {}

        if config.GetBool('leak-container', 0):
            _createContainerLeak()
        if config.GetBool('leak-tasks', 0):
            _createTaskLeak()

        # don't check our own tables for leaks
        ContainerLeakDetector.addPrivateObj(ContainerLeakDetector.PrivateIds)
        ContainerLeakDetector.addPrivateObj(self.__dict__)

        self.setPriority(Job.Priorities.Min)
        jobMgr.add(self)

    def destroy(self):
        messenger.send(self._getDestroyEvent())
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

    def _getDestroyEvent(self):
        # sent when leak detector is about to be destroyed
        return 'cldDestroy-%s' % self._serialNum

    def getLeakEvent(self):
        # sent when a leak is detected
        # passes description string as argument
        return 'containerLeakDetected-%s' % self._serialNum

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
        return list(self._id2ref.keys())

    def getContainerByIdGen(self, id, **kwArgs):
        # return a generator to look up a container
        return self._id2ref[id].getContainerGen(**kwArgs)
    def getContainerById(self, id):
        for result in self._id2ref[id].getContainerGen():
            pass
        return result
    def getContainerNameByIdGen(self, id, **kwArgs):
        return self._id2ref[id].getEvalStrGen(**kwArgs)
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

    def getPathsToContainers(self, name, ot, doneCallback=None):
        j =  FPTObjsOfType(name, self, ot, doneCallback)
        jobMgr.add(j)
        return j

    def getPathsToContainersNamed(self, name, on, doneCallback=None):
        j =  FPTObjsNamed(name, self, on, doneCallback)
        jobMgr.add(j)
        return j

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
        self._nextCheckDelay = self._nextCheckDelay * self._checkDelayScale

    def _checkForLeaks(self, task=None):
        self._index2delay[len(self._index2containerId2len)] = self._nextCheckDelay
        self._checkContainersJob = CheckContainers(
            '%s-checkForLeaks' % self.getJobName(), self, len(self._index2containerId2len))
        self.acceptOnce(self._checkContainersJob.getFinishedEvent(),
                        self._scheduleNextLeakCheck)
        jobMgr.add(self._checkContainersJob)
        return task.done

    def _scheduleNextPruning(self):
        taskMgr.doMethodLater(self._pruneTaskPeriod, self._pruneObjectRefs,
                              self._getPruneTaskName())

    def _pruneObjectRefs(self, task=None):
        self._pruneContainersJob = PruneObjectRefs(
            '%s-pruneObjectRefs' % self.getJobName(), self)
        self.acceptOnce(self._pruneContainersJob.getFinishedEvent(),
                        self._scheduleNextPruning)
        jobMgr.add(self._pruneContainersJob)
        return task.done
