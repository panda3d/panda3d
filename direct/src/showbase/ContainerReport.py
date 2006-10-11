from direct.showbase.PythonUtil import Stack, fastRepr, invertDictLossless
from direct.showbase.PythonUtil import itype, safeRepr
import types

class ContainerReport:
    def __init__(self, rootObj, rootObjName=None):
        self._rootObj = rootObj
        if rootObjName is None:
            rootObjName = repr(rootObj)
        self._visitedIds = set()
        self._id2pathStr = {}
        self._id2container = {}
        self._id2len = {}
        self._stack = Stack()
        self._stack.push(rootObj)
        self._id2pathStr[id(rootObj)] = rootObjName
        self._traverse()
    def _examine(self, obj):
        # return False if it's an object that can't contain or lead to other objects
        if type(obj) in (types.BooleanType, types.BuiltinFunctionType,
                         types.BuiltinMethodType, types.ComplexType,
                         types.FloatType, types.IntType, types.LongType,
                         types.NoneType, types.NotImplementedType,
                         types.TypeType, types.CodeType):
            return False
        try:
            length = len(obj)
        except:
            length = None
        if length is not None and length > 0:
            objId = id(obj)
            self._id2container[objId] = obj
            self._id2len[objId] = length
        return True
    def _traverse(self):
        while len(self._stack) > 0:
            obj = self._stack.pop()
            #print '_traverse: %s' % fastRepr(obj, 30)
            try:
                dirList = dir(obj)
            except:
                pass
            else:
                for name in dirList:
                    if name[-2:] == '__':
                        continue
                    attr = getattr(obj, name)
                    if id(attr) not in self._visitedIds:
                        self._visitedIds.add(id(attr))
                        if self._examine(attr):
                            self._id2pathStr[id(attr)] = self._id2pathStr[id(obj)] + '.%s' % name
                            self._stack.push(attr)
            if type(obj) not in (types.StringType, types.UnicodeType):
                if type(obj) is types.DictType:
                    keys = obj.keys()
                    for key in keys:
                        attr = obj[key]
                        if id(attr) not in self._visitedIds:
                            self._visitedIds.add(id(attr))
                            if self._examine(attr):
                                self._id2pathStr[id(attr)] = self._id2pathStr[id(obj)] + '[%s]' % safeRepr(key)
                                self._stack.push(attr)
                elif type(obj) is not types.FileType:
                    try:
                        itr = iter(obj)
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
                                    break
                                if id(attr) not in self._visitedIds:
                                    self._visitedIds.add(id(attr))
                                    if self._examine(attr):
                                        self._id2pathStr[id(attr)] = self._id2pathStr[id(obj)] + '[%s]' % index
                                        self._stack.push(attr)
                                index += 1
                        except StopIteration, e:
                            pass
        
    def __repr__(self):
        len2ids = invertDictLossless(self._id2len)
        lengths = len2ids.keys()
        lengths.sort()
        lengths.reverse()
        for l in lengths:
            for id in len2ids[l]:
                obj = self._id2container[id]
                print '%s: %s: %s' % (l, repr(itype(obj)), self._id2pathStr[id])
