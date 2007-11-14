"""Undocumented Module"""

__all__ = ['Diff', 'ObjectPool']

from direct.directnotify.DirectNotifyGlobal import directNotify
from direct.showbase.PythonUtil import invertDictLossless, makeList, safeRepr
from direct.showbase.PythonUtil import getNumberedTypedString, getNumberedTypedSortedString
from direct.showbase.PythonUtil import getNumberedTypedSortedStringWithReferrersGen
import types
import gc

class Diff:
    def __init__(self, lost, gained):
        self.lost=lost
        self.gained=gained
    def printOut(self, full=False):
        print 'lost %s objects, gained %s objects' % (len(self.lost), len(self.gained))
        print '\n\nself.lost\n'
        print self.lost.typeFreqStr()
        print '\n\nself.gained\n'
        print self.gained.typeFreqStr()
        if full:
            self.gained.printObjsByType()
            print '\n\nGAINED-OBJECT REFERRERS\n'
            self.gained.printReferrers(1)

class ObjectPool:
    """manipulate a pool of Python objects"""
    notify = directNotify.newCategory('ObjectPool')

    def __init__(self, objects):
        self._objs = list(objects)
        self._type2objs = {}
        self._count2types = {}
        self._len2obj = {}
        type2count = {}
        for obj in self._objs:
            typ = itype(obj)
            type2count.setdefault(typ, 0)
            type2count[typ] += 1
            self._type2objs.setdefault(typ, [])
            self._type2objs[typ].append(obj)
            try:
                self._len2obj[len(obj)] = obj
            except:
                pass
        self._count2types = invertDictLossless(type2count)

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

    def printObjsByType(self, printReferrers=False):
        print   'Object Pool: Objects By Type'
        print '\n============================'
        counts = list(set(self._count2types.keys()))
        counts.sort()
        # print types with the smallest number of instances first, in case
        # there's a large group that waits a long time before printing
        #counts.reverse()
        for count in counts:
            types = makeList(self._count2types[count])
            for typ in types:
                print 'TYPE: %s, %s objects' % (repr(typ), len(self._type2objs[typ]))
                if printReferrers:
                    for line in getNumberedTypedSortedStringWithReferrersGen(self._type2objs[typ]):
                        print line
                else:
                    print getNumberedTypedSortedString(self._type2objs[typ])

    def containerLenStr(self):
        s  =   'Object Pool: Container Lengths'
        s += '\n=============================='
        lengths = list(self._len2obj.keys())
        lengths.sort()
        lengths.reverse()
        for count in counts:
            pass

    def printReferrers(self, numEach=3):
        """referrers of the first few of each type of object"""
        counts = list(set(self._count2types.keys()))
        counts.sort()
        counts.reverse()
        for count in counts:
            types = makeList(self._count2types[count])
            for typ in types:
                print '\n\nTYPE: %s' % repr(typ)
                for i in xrange(min(numEach,len(self._type2objs[typ]))):
                    obj = self._type2objs[typ][i]
                    print '\nOBJ: %s\n' % safeRepr(obj)
                    referrers = gc.get_referrers(obj)
                    print '%s REFERRERS:\n' % len(referrers)
                    if len(referrers):
                        print getNumberedTypedString(referrers, maxLen=80,
                                                    numPrefix='REF')
                    else:
                        print '<No Referrers>'

    def __len__(self):
        return len(self._objs)
