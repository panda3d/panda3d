from direct.directnotify.DirectNotifyGlobal import directNotify
from direct.showbase.PythonUtil import invertDict, makeList, safeRepr
from direct.showbase.PythonUtil import getNumberedTypedString
import types
import gc

class Diff:
    def __init__(self, lost, gained):
        self.lost=lost
        self.gained=gained
    def __repr__(self):
        s  = 'lost %s objects, gained %s objects' % (len(self.lost), len(self.gained))
        s += '\n\nself.lost\n'
        s += self.lost.typeFreqStr()
        s += '\n\nself.gained\n'
        s += self.gained.typeFreqStr()
        s += '\n\nGAINED-OBJECT REFERRERS\n'
        s += self.gained.referrersStr(1)
        return s

class ObjectPool:
    """manipulate a pool of Python objects"""
    notify = directNotify.newCategory('ObjectPool')

    def __init__(self, objects):
        self._objs = list(objects)
        self._type2objs = {}
        self._count2types = {}
        type2count = {}
        for obj in self._objs:
            typ = type(obj)
            # <type 'instance'> isn't all that useful
            if typ is types.InstanceType:
                typ = '%s of %s' % (typ, repr(obj.__class__))
            type2count.setdefault(typ, 0)
            type2count[typ] += 1
            self._type2objs.setdefault(typ, [])
            self._type2objs[typ].append(obj)
        self._count2types = invertDict(type2count)

    def _getInternalObjs(self):
        return (self._objs, self._type2objs, self._count2types)

    def destroy(self):
        del self._objs
        del self._type2objs
        del self._count2types

    def getTypes(self):
        return self._type2objs.keys()

    def getObjsOfType(self, type):
        return self._type2objs.get(type, [])

    def printObjsOfType(self, type):
        for obj in self._type2objs.get(type, []):
            print repr(obj)

    def diff(self, other):
        """print difference between this pool and 'other' pool"""
        thisId2obj = {}
        otherId2obj = {}
        for obj in self._objs:
            thisId2obj[id(obj)] = obj
        for obj in other._objs:
            otherId2obj[id(obj)] = obj
        thisIds = set(thisId2obj.keys())
        otherIds = set(otherId2obj.keys())
        lostIds = thisIds.difference(otherIds)
        print 'lost: %s' % lostIds
        gainedIds = otherIds.difference(thisIds)
        del thisIds
        del otherIds
        lostObjs = []
        for i in lostIds:
            lostObjs.append(thisId2obj[i])
        gainedObjs = []
        for i in gainedIds:
            gainedObjs.append(otherId2obj[i])
        return Diff(self.__class__(lostObjs), self.__class__(gainedObjs))

    def typeFreqStr(self):
        s  =   'Object Pool: Type Frequencies'
        s += '\n============================='
        counts = list(set(self._count2types.keys()))
        counts.sort()
        counts.reverse()
        for count in counts:
            types = makeList(self._count2types[count])
            for typ in types:
                s += '\n%s\t%s' % (count, typ)
        return s

    def referrersStr(self, numEach=3):
        """referrers of the first few of each type of object"""
        s = ''
        counts = list(set(self._count2types.keys()))
        counts.sort()
        counts.reverse()
        for count in counts:
            types = makeList(self._count2types[count])
            for typ in types:
                s += '\n\nTYPE: %s' % typ
                for i in xrange(min(numEach,len(self._type2objs[typ]))):
                    obj = self._type2objs[typ][i]
                    s += '\nOBJ: %s\n' % safeRepr(obj)
                    referrers = gc.get_referrers(obj)
                    if len(referrers):
                        s += getNumberedTypedString(referrers)
                    else:
                        s += '<No Referrers>'
        return s

    def __len__(self):
        return len(self._objs)
