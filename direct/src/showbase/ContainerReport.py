from direct.showbase.PythonUtil import Queue, fastRepr, invertDictLossless
from direct.showbase.PythonUtil import itype, safeRepr
import types

"""
import types;from direct.showbase.ContainerReport import ContainerReport;ContainerReport().log()
"""

class ContainerReport:
    PrivateIds = set()
    def __init__(self, name, log=False, limit=None):
        self._name = name
        self._visitedIds = set()
        self._id2pathStr = {}
        self._id2container = {}
        self._type2id2len = {}
        self._instanceDictIds = set()
        # for breadth-first searching
        self._queue = Queue()
        ContainerReport.PrivateIds.update(set([
            id(ContainerReport.PrivateIds),
            id(self._visitedIds),
            id(self._id2pathStr),
            id(self._id2container),
            id(self._type2id2len),
            id(self._queue),
            id(self._instanceDictIds),
            ]))
        self._queue.push(__builtins__)
        self._id2pathStr[id(__builtins__)] = ''
        self._traverse()
        if log:
            self.log(limit=limit)
    def _examine(self, obj):
        # return False if it's an object that can't contain or lead to other objects
        if type(obj) in (types.BooleanType, types.BuiltinFunctionType,
                         types.BuiltinMethodType, types.ComplexType,
                         types.FloatType, types.IntType, types.LongType,
                         types.NoneType, types.NotImplementedType,
                         types.TypeType, types.CodeType, types.FunctionType):
            return False
        # if it's an internal object, ignore it
        if id(obj) in ContainerReport.PrivateIds:
            return False
        # this object might lead to more objects. put it on the queue
        self._queue.push(obj)
        # if it's a container, put it in the tables
        try:
            length = len(obj)
        except:
            length = None
        if length is not None and length > 0:
            objId = id(obj)
            self._id2container[objId] = obj
            self._type2id2len.setdefault(type(obj), {})
            self._type2id2len[type(obj)][objId] = length
        return True
    def _traverse(self):
        while len(self._queue) > 0:
            parentObj = self._queue.pop()
            #print '%s: %s, %s' % (id(parentObj), type(parentObj), fastRepr(parentObj))
            isInstanceDict = False
            if id(parentObj) in self._instanceDictIds:
                isInstanceDict = True

            try:
                if parentObj.__class__.__name__ == 'method-wrapper':
                    continue
            except:
                pass

            if type(parentObj) in (types.StringType, types.UnicodeType):
                continue
            
            if type(parentObj) in (types.ModuleType, types.InstanceType):
                child = parentObj.__dict__
                if self._examine(child):
                    assert _equal(self._queue.back(), child)
                    self._instanceDictIds.add(id(child))
                    self._id2pathStr[id(child)] = str(self._id2pathStr[id(parentObj)])
                continue

            if type(parentObj) is types.DictType:
                key = None
                attr = None
                for key, attr in parentObj.items():
                    if id(attr) not in self._visitedIds:
                        self._visitedIds.add(id(attr))
                        if self._examine(attr):
                            assert _equal(self._queue.back(), attr)
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

            if type(parentObj) is not types.FileType:
                try:
                    itr = iter(parentObj)
                except:
                    pass
                else:
                    try:
                        index = 0
                        while 1:
                            try:
                                attr = itr.next()
                            except:
                                # some custom classes don't do well when iterated
                                attr = None
                                break
                            if id(attr) not in self._visitedIds:
                                self._visitedIds.add(id(attr))
                                if self._examine(attr):
                                    assert _equal(self._queue.back(), attr)
                                    self._id2pathStr[id(attr)] = self._id2pathStr[id(parentObj)] + '[%s]' % index
                            index += 1
                        del attr
                    except StopIteration, e:
                        pass
                    del itr
                    continue

            try:
                childNames = dir(parentObj)
            except:
                pass
            else:
                childName = None
                child = None
                for childName in childNames:
                    child = getattr(parentObj, childName)
                    if id(child) not in self._visitedIds:
                        self._visitedIds.add(id(child))
                        if self._examine(child):
                            assert _equal(self._queue.back(), child)
                            self._id2pathStr[id(child)] = self._id2pathStr[id(parentObj)] + '.%s' % childName
                del childName
                del child
                continue

    def _outputType(self, type, limit=None):
        if type not in self._type2id2len:
            return
        len2ids = invertDictLossless(self._type2id2len[type])
        lengths = len2ids.keys()
        lengths.sort()
        lengths.reverse()
        print '====='
        print '===== %s' % type
        count = 0
        stop = False
        for l in lengths:
            len2ids[l].sort()
            for id in len2ids[l]:
                obj = self._id2container[id]
                print '%s: %s' % (l, self._id2pathStr[id])
                count += 1
                if limit is not None and count >= limit:
                    return

    def _output(self, **kArgs):
        print "===== ContainerReport: \'%s\' =====" % (self._name,)
        initialTypes = (types.DictType, types.ListType, types.TupleType)
        for type in initialTypes:
            self._outputType(type, **kArgs)
        otherTypes = list(set(self._type2id2len.keys()).difference(set(initialTypes)))
        otherTypes.sort()
        for type in otherTypes:
            self._outputType(type, **kArgs)

    def log(self, **kArgs):
        self._output(**kArgs)
