import inspect
import sys
import gc
from direct.showbase.PythonUtil import _getSafeReprNotify
from direct.showbase.Job import Job

class ReferrerSearch(Job):
    def __init__(self, obj):
        Job.__init__(self, 'ReferrerSearch')
        self.obj = obj
        self.found = set()
    
    def __call__(self):
        safeReprNotify = _getSafeReprNotify()
        info = safeReprNotify.getInfo()
        safeReprNotify.setInfo(0)

        self.found = set()
        try:
            self.step(0, [self.obj])
        finally:
            self.obj = None
            pass
        
        safeReprNotify.setInfo(info)
        pass

    def run(self):
        safeReprNotify = _getSafeReprNotify()
        info = safeReprNotify.getInfo()
        safeReprNotify.setInfo(0)

        print 'RefPath: Beginning ReferrerSearch for', fastRepr(self.obj)

        self.found = set()
        for x in self.stepGenerator(0, [self.obj]):
            yield None
            pass
        
        self.obj = None
        pass
        
        safeReprNotify.setInfo(info)

        yield Job.Done
        pass

    def finished(self):
        print 'RefPath: Completed ReferrerSearch for', fastRepr(self.obj)
        self.obj = None
        
        
    def truncateAtNewLine(self, s):
        if s.find('\n') == -1:
            return s
        else:
            return s[:s.find('\n')]
        
    def myrepr(self, referrer, refersTo):
        pre = ''
        if (isinstance(referrer, dict)):
            for k,v in referrer.iteritems():
                if v is refersTo:
                    pre = self.truncateAtNewLine(fastRepr(k)) + ']-> '
                    break
        elif (isinstance(referrer, (list, tuple))):
            for x in xrange(len(referrer)):
                if referrer[x] is refersTo:
                    pre = '%s]-> ' % (x)
                    break

        if (isinstance(refersTo, dict)):
            post = 'dict['
        elif (isinstance(refersTo, list)):
            post = 'list['
        elif (isinstance(refersTo, tuple)):
            post = 'tuple['
        elif (isinstance(refersTo, set)):
            post = 'set->'
        else:
            post = self.truncateAtNewLine(fastRepr(refersTo)) + "-> "

        return '%s%s' % (pre, post)
        
    def step(self, depth, path):
        at = path[-1]

        if inspect.isframe(at) or \
           (isinstance(at, dict) and \
            at.keys() == locals().keys()) or \
           at is self.__dict__ or \
           id(at) in self.found:
               # don't continue down this path
               return 

        # Now we define our 'roots'

        # __builtins__
        if at is __builtins__:
            sys.stdout.write("RefPath: __builtins__-> ")
            path = list(reversed(path))
            path.insert(0,0)
            for x in xrange(len(path)-1):
                sys.stdout.write(self.myrepr(path[x], path[x+1]))
                pass
            print
            return

        # any module scope
        if inspect.ismodule(at):
            sys.stdout.write("RefPath: Module(%s)-> " % (at.__name__))
            path = list(reversed(path))
            for x in xrange(len(path)-1):
                sys.stdout.write(self.myrepr(path[x], path[x+1]))
                pass
            print
            return

        # simbase
        if at is simbase:
            sys.stdout.write("RefPath: simbase-> ")
            path = list(reversed(path))
            path.insert(0,0)
            for x in xrange(len(path)-1):
                sys.stdout.write(self.myrepr(path[x], path[x+1]))
                pass
            print
            return

        # simbase.air
        if at is simbase.air:
            sys.stdout.write("RefPath: simbase.air-> ")
            path = list(reversed(path))
            path.insert(0,0)
            for x in xrange(len(path)-1):
                sys.stdout.write(self.myrepr(path[x], path[x+1]))
                pass
            print
            return
        
        self.found.add(id(at))
        
        referrers = gc.get_referrers(at)
        while(referrers):
            ref = referrers.pop()
            if (ref != path):
                self.step(depth + 1, path + [ref])
            pass
        pass
    pass

    def stepGenerator(self, depth, path):
        at = path[-1]

        if inspect.isframe(at) or \
           (isinstance(at, dict) and \
            at.keys() == locals().keys()) or \
           at is self.__dict__ or \
           id(at) in self.found:
               # don't continue down this path
               raise StopIteration 

        # Now we define our 'roots'

        # __builtins__
        if at is __builtins__:
            sys.stdout.write("RefPath: __builtins__-> ")
            path = list(reversed(path))
            path.insert(0,0)
            for x in xrange(len(path)-1):
                sys.stdout.write(self.myrepr(path[x], path[x+1]))
                pass
            print
            raise StopIteration 

        # any module scope
        if inspect.ismodule(at):
            sys.stdout.write("RefPath: Module(%s)-> " % (at.__name__))
            path = list(reversed(path))
            for x in xrange(len(path)-1):
                sys.stdout.write(self.myrepr(path[x], path[x+1]))
                pass
            print
            raise StopIteration

        # simbase
        if at is simbase:
            sys.stdout.write("RefPath: simbase-> ")
            path = list(reversed(path))
            path.insert(0,0)
            for x in xrange(len(path)-1):
                sys.stdout.write(self.myrepr(path[x], path[x+1]))
                pass
            print
            raise StopIteration

        # simbase.air
        if at is simbase.air:
            sys.stdout.write("RefPath: simbase.air-> ")
            path = list(reversed(path))
            path.insert(0,0)
            for x in xrange(len(path)-1):
                sys.stdout.write(self.myrepr(path[x], path[x+1]))
                pass
            print
            raise StopIteration        

        self.found.add(id(at))
        
        referrers = gc.get_referrers(at)
        while(referrers):
            ref = referrers.pop()
            if (ref != path):
                for x in self.stepGenerator(depth + 1, path + [ref]):
                    yield None
                pass
            pass

        yield None
        pass
    pass


"""
from direct.showbase.ReferrerSearch import ReferrerSearch
door = simbase.air.doFind("DistributedBuildingDoorAI")
class A: pass
door = A()
ReferrerSearch()(door)
reload(ReferrerSearch); from direct.showbase.PythonUtil import ReferrerSearch
"""
