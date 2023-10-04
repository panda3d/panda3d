from __future__ import annotations

from direct.directnotify.DirectNotifyGlobal import directNotify
from direct.showbase.PythonUtil import Queue, invertDictLossless
from direct.showbase.PythonUtil import safeRepr
from direct.showbase.Job import Job
from direct.showbase.JobManagerGlobal import jobMgr
from direct.showbase.ContainerLeakDetector import deadEndTypes
import types
import io


class ContainerReport(Job):
    notify = directNotify.newCategory("ContainerReport")
    # set of containers that should not be included in the report
    PrivateIds: set[int] = set()

    def __init__(self, name, log=False, limit=None, threaded=False):
        Job.__init__(self, name)
        self._log = log
        self._limit = limit
        # set up our data structures
        self._visitedIds = set()
        self._id2pathStr = {}
        self._id2container = {}
        self._type2id2len = {}
        self._instanceDictIds = set()
        # for breadth-first searching
        self._queue = Queue()
        jobMgr.add(self)
        if not threaded:
            jobMgr.finish(self)

    def destroy(self):
        del self._queue
        del self._instanceDictIds
        del self._type2id2len
        del self._id2container
        del self._id2pathStr
        del self._visitedIds
        del self._limit
        del self._log

    def finished(self):
        if self._log:
            self.destroy()

    def run(self):
        ContainerReport.PrivateIds.update(set([
            id(ContainerReport.PrivateIds),
            id(self._visitedIds),
            id(self._id2pathStr),
            id(self._id2container),
            id(self._type2id2len),
            id(self._queue),
            id(self._instanceDictIds),
        ]))
        # push on a few things that we want to give priority
        # for the sake of the variable-name printouts
        try:
            base
        except NameError:
            pass
        else:
            self._enqueueContainer(base.__dict__,
                                   'base')
        try:
            simbase
        except NameError:
            pass
        else:
            self._enqueueContainer(simbase.__dict__,
                                   'simbase')
        self._queue.push(__builtins__)
        self._id2pathStr[id(__builtins__)] = ''

        while len(self._queue) > 0:
            # yield up here instead of at the end, since we skip back to the
            # top of the while loop from various points
            yield None
            parentObj = self._queue.pop()
            #print '%s: %s, %s' % (id(parentObj), type(parentObj), self._id2pathStr[id(parentObj)])
            isInstanceDict = False
            if id(parentObj) in self._instanceDictIds:
                isInstanceDict = True

            try:
                if parentObj.__class__.__name__ == 'method-wrapper':
                    continue
            except Exception:
                pass

            if isinstance(parentObj, (str, bytes)):
                continue

            if isinstance(parentObj, dict):
                key = None
                attr = None
                keys = list(parentObj.keys())
                try:
                    keys.sort()
                except TypeError as e:
                    self.notify.warning('non-sortable dict keys: %s: %s' % (self._id2pathStr[id(parentObj)], repr(e)))
                for key in keys:
                    try:
                        attr = parentObj[key]
                    except KeyError as e:
                        self.notify.warning('could not index into %s with key %s' % (self._id2pathStr[id(parentObj)],
                                                                                     key))
                    if id(attr) not in self._visitedIds:
                        self._visitedIds.add(id(attr))
                        if self._examine(attr):
                            assert self._queue.back() is attr
                            if parentObj is __builtins__:
                                self._id2pathStr[id(attr)] = key
                            else:
                                if isInstanceDict:
                                    self._id2pathStr[id(attr)] = self._id2pathStr[id(parentObj)] + '.%s' % key
                                else:
                                    self._id2pathStr[id(attr)] = self._id2pathStr[id(parentObj)] + '[%s]' % safeRepr(key)
                del key
                del attr
                continue

            # types.CellType was added in Python 3.8
            if type(parentObj) is types.CellType:
                child = parentObj.cell_contents
                if self._examine(child):
                    assert (self._queue.back() is child)
                    self._instanceDictIds.add(id(child))
                    self._id2pathStr[id(child)] = str(self._id2pathStr[id(parentObj)]) + '.cell_contents'
                continue

            if hasattr(parentObj, '__dict__'):
                # Instance of a class
                child = parentObj.__dict__
                if self._examine(child):
                    assert (self._queue.back() is child)
                    self._instanceDictIds.add(id(child))
                    self._id2pathStr[id(child)] = str(self._id2pathStr[id(parentObj)])
                continue

            if not isinstance(parentObj, io.TextIOWrapper):
                try:
                    itr = iter(parentObj)
                except Exception:
                    pass
                else:
                    try:
                        index = 0
                        while 1:
                            try:
                                attr = next(itr)
                            except Exception:
                                # some custom classes don't do well when iterated
                                attr = None
                                break
                            if id(attr) not in self._visitedIds:
                                self._visitedIds.add(id(attr))
                                if self._examine(attr):
                                    assert self._queue.back() is attr
                                    self._id2pathStr[id(attr)] = self._id2pathStr[id(parentObj)] + '[%s]' % index
                            index += 1
                        del attr
                    except StopIteration as e:
                        pass
                    del itr
                    continue

            try:
                childNames = dir(parentObj)
            except Exception:
                pass
            else:
                childName = None
                child = None
                for childName in childNames:
                    try:
                        child = getattr(parentObj, childName)
                    except Exception:
                        continue
                    if id(child) not in self._visitedIds:
                        self._visitedIds.add(id(child))
                        if self._examine(child):
                            assert self._queue.back() is child
                            self._id2pathStr[id(child)] = self._id2pathStr[id(parentObj)] + '.%s' % childName
                del childName
                del child
                continue

        if self._log:
            self.printingBegin()
            for i in self._output(limit=self._limit):
                yield None
            self.printingEnd()

        yield Job.Done

    def _enqueueContainer(self, obj, pathStr=None):
        # call this to add a container that should be examined before any (other) direct
        # children of __builtins__
        # this is mostly to fix up the names of variables
        self._queue.push(obj)
        objId = id(obj)
        if pathStr is not None:
            self._id2pathStr[objId] = pathStr
        # if it's a container, put it in the tables
        try:
            length = len(obj)
        except Exception:
            length = None
        if length is not None and length > 0:
            self._id2container[objId] = obj
            self._type2id2len.setdefault(type(obj), {})
            self._type2id2len[type(obj)][objId] = length

    def _examine(self, obj):
        # return False if it's an object that can't contain or lead to other objects
        if type(obj) in deadEndTypes:
            return False
        # if it's an internal object, ignore it
        if id(obj) in ContainerReport.PrivateIds:
            return False
        # this object might lead to more objects. put it on the queue
        self._enqueueContainer(obj)
        return True

    def _outputType(self, type, limit=None):
        if type not in self._type2id2len:
            return
        len2ids = invertDictLossless(self._type2id2len[type])
        print('=====')
        print('===== %s' % type)
        count = 0
        stop = False
        for l in sorted(len2ids, reverse=True):
            #len2ids[l].sort()
            pathStrList = list()
            for id in len2ids[l]:
                obj = self._id2container[id]
                #print '%s: %s' % (l, self._id2pathStr[id])
                pathStrList.append(self._id2pathStr[id])
                count += 1
                if (count & 0x7f) == 0:
                    yield None
            pathStrList.sort()
            for pathstr in pathStrList:
                print('%s: %s' % (l, pathstr))
            if limit is not None and count >= limit:
                return

    def _output(self, **kArgs):
        print("===== ContainerReport: \'%s\' =====" % (self._name,))
        initialTypes = (dict, list, tuple)
        for type in initialTypes:
            for i in self._outputType(type, **kArgs):
                yield None
        otherTypes = set(self._type2id2len).difference(initialTypes)
        for type in sorted(otherTypes, key=lambda obj: obj.__name__):
            for i in self._outputType(type, **kArgs):
                yield None

    def log(self, **kArgs):
        self._output(**kArgs)
