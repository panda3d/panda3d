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
        self._leakDetector.notify.debug(repr(self._leakDetector._id2ref))
        ids = self._leakDetector.getContainerIds()
        # record the current len of each container
        for id in ids:
            yield None
            try:
                container = self._leakDetector.getContainerById(id)
            except Exception, e:
                # this container no longer exists
                self.notify.debug('container %s no longer exists; caught exception in getContainerById (%s)' % (name, e))
                self._leakDetector.removeContainerById(id)
                continue
            if container is None:
                # this container no longer exists
                self.notify.debug('container %s no longer exists; getContainerById returned None (%s)' % (name, e))
                self._leakDetector.removeContainerById(id)
                continue
            cLen = len(container)
            name = self._leakDetector.getContainerNameById(id)
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
                            self.notify.warning('container %s grew > 200%% in %s minutes' % (name, minutes))
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

class NoDictKey:
    pass

class Indirection:
    # represents the indirection that brings you from a container to an element of the container
    # stored as a string to be used as part of an eval
    # each dictionary dereference is individually eval'd since the dict key might have been garbage-collected
    class GarbageCollectedDictKey(Exception):
        pass

    def __init__(self, evalStr=None, dictKey=NoDictKey):
        # if this is a dictionary lookup, pass dictKey instead of evalStr
        self.evalStr = evalStr
        self.dictKey = NoDictKey
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
                # store a weakref to the key
                self.dictKey = weakref.ref(dictKey)

    def isDictKey(self):
        return self.dictKey is not NoDictKey

    def dereferenceDictKey(self, parentDict):
        key = self.dictKey()
        if key is None:
            raise Indirection.GarbageCollectedDictKey()
        return parentDict[key]

    def getString(self, nextIndirection=None):
        # return our contribution to the name of the object
        if self.evalStr is not None:
            # if we're an instance dict and the next indirection is not a dict key,
            # skip over this one (obj.__dict__[keyName] == obj.keyName)
            if nextIndirection is not None and self.evalStr == '.__dict__':
                return ''
            return self.evalStr

        # we're stored as a dict key
        # this might not eval, but that's OK, we're not using this string to find
        # the object, we dereference the parent dict
        key = self.dictKey()
        if key is None:
            return '<garbage-collected dict key>'
        return safeRepr(key)

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
    # whatever this is set to will be the default ContainerRef
    BaseRef = None

    def __init__(self, other=None, indirection=None):
        self._indirections = []
        # if no other passed in, try ContainerRef.BaseRef
        if other is None:
            other = ContainerRef.BaseRef
        if other is not None:
            for ind in other._indirections:
                self.addIndirection(ind)
        if indirection:
            self.addIndirection(indirection)

    def addIndirection(self, indirection):
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
        curIndirection = None
        nextIndirection = None
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
        curIndirection = None
        nextIndirection = None
        for i in xrange(len(self._indirections)):
            curIndirection = self._indirections[i]
            if i < len(self._indirections)-1:
                nextIndirection = self._indirections[i+1]
            else:
                nextIndirection = None
            str += curIndirection.getString(nextIndirection=nextIndirection)
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
        self._index2containerName2len = {}
        self._index2delay = {}
        # set up our data structures
        self._id2ref = {}

        # set up the base/starting object
        self._nameContainer(__builtin__.__dict__, ContainerRef(indirection=Indirection(evalStr='__builtin__.__dict__')))
        try:
            base
        except:
            pass
        else:
            ContainerRef.BaseRef = ContainerRef(indirection=Indirection(evalStr='base.__dict__'))
            self._nameContainer(base.__dict__, ContainerRef.BaseRef)
        try:
            simbase
        except:
            pass
        else:
            ContainerRef.BaseRef = ContainerRef(indirection=Indirection(evalStr='simbase.__dict__'))
            self._nameContainer(simbase.__dict__, ContainerRef.BaseRef)

        self._curObjRef = ContainerRef()
        jobMgr.add(self)
        ContainerLeakDetector.PrivateIds.update(set([
            id(ContainerLeakDetector.PrivateIds),
            id(self._id2ref),
            ]))

    def destroy(self):
        if self._checkContainersJob is not None:
            jobMgr.remove(self._checkContainersJob)
            self._checkContainersJob = None
        del self._id2ref
        del self._index2containerName2len
        del self._index2delay

    def getPriority(self):
        return self._priority

    def getCheckTaskName(self):
        return 'checkForLeakingContainers-%s' % self._serialNum

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
                              self.getCheckTaskName())

        while True:
            # yield up here instead of at the end, since we skip back to the
            # top of the while loop from various points
            yield None
            #import pdb;pdb.set_trace()
            curObj = None
            if self._curObjRef is None:
                self._curObjRef = random.choice(self._id2ref.values())
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
            self.notify.debug('--> %s' % self._curObjRef)

            # keep a copy of this obj's eval str, it might not be in _id2ref
            curObjRef = self._curObjRef
            # if we hit a dead end, start over at a container we know about
            self._curObjRef = None

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
                    self._curObjRef = ContainerRef(curObjRef, indirection=Indirection(evalStr='.__dict__'))
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
                    if not self._isDeadEnd(attr):
                        if curObj is __builtin__.__dict__:
                            indirection=Indirection(evalStr=key)
                        else:
                            indirection=Indirection(dictKey=key)
                        self._curObjRef = ContainerRef(curObjRef, indirection=indirection)
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
                                self._curObjRef = ContainerRef(curObjRef, indirection=Indirection(evalStr='[%s]' % index))
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
                    if not self._isDeadEnd(child):
                        self._curObjRef = ContainerRef(curObjRef, indirection=Indirection(evalStr='.%s' % childName))
                        if self._isContainer(child):
                            self._nameContainer(child, self._curObjRef)
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
        if id(obj) in self._id2ref:
            return True
        return False

    def _isContainer(self, obj):
        try:
            len(obj)
        except:
            return False
        return True

    def _nameContainer(self, cont, objRef):
        if self.notify.getDebug():
            self.notify.debug('_nameContainer: %s' % objRef)
            #printStack()
        contId = id(cont)
        # if this container is new, or the objRef repr is shorter than what we already have,
        # put it in the table
        if contId not in self._id2ref or len(repr(objRef)) < len(repr(self._id2ref[contId])):
            self._id2ref[contId] = objRef

    def _checkForLeaks(self, task=None):
        self._index2delay[len(self._index2containerName2len)] = self._nextCheckDelay
        self._checkContainersJob = CheckContainers(
            '%s-checkForLeaks' % self.getJobName(), self, len(self._index2containerName2len))
        jobMgr.add(self._checkContainersJob)
        
        self._nextCheckDelay *= 2
        taskMgr.doMethodLater(self._nextCheckDelay, self._checkForLeaks,
                              self.getCheckTaskName())
