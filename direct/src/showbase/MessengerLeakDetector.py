from direct.directnotify.DirectNotifyGlobal import directNotify
from direct.showbase.DirectObject import DirectObject
from direct.showbase.Job import Job
import gc, sys

if sys.version_info >= (3, 0):
    import builtins
else:
    import __builtin__ as builtins


class MessengerLeakObject(DirectObject):
    def __init__(self):
        self.accept('leakEvent', self._handleEvent)
    def _handleEvent(self):
        pass

def _leakMessengerObject():
    leakObject = MessengerLeakObject()

class MessengerLeakDetector(Job):
    # check for objects that are only referenced by the messenger
    # and would otherwise be garbage collected
    notify = directNotify.newCategory("MessengerLeakDetector")

    def __init__(self, name):
        Job.__init__(self, name)
        self.setPriority(Job.Priorities.Normal*2)
        jobMgr.add(self)

    def run(self):
        # set of ids of objects that we know are always attached to builtin;
        # if an object is attached to one of these, it's attached to builtin
        # this cuts down on the amount of searching that needs to be done
        builtinIds = set()
        builtinIds.add(id(builtins.__dict__))
        try:
            builtinIds.add(id(base))
            builtinIds.add(id(base.cr))
            builtinIds.add(id(base.cr.doId2do))
        except:
            pass
        try:
            builtinIds.add(id(simbase))
            builtinIds.add(id(simbase.air))
            builtinIds.add(id(simbase.air.doId2do))
        except:
            pass
        try:
            builtinIds.add(id(uber))
            builtinIds.add(id(uber.air))
            builtinIds.add(id(uber.air.doId2do))
        except:
            pass

        while True:
            yield None
            objects = list(messenger._Messenger__objectEvents.keys())
            assert self.notify.debug('%s objects in the messenger' % len(objects))
            for object in objects:
                yield None
                assert self.notify.debug('---> new object: %s' % itype(object))
                # try to find a path to builtin that doesn't involve the messenger
                # lists of objects for breadth-first search
                # iterate through one list while populating other list
                objList1 = []
                objList2 = []
                curObjList = objList1
                nextObjList = objList2
                visitedObjIds = set()

                # add the id of the object, and the messenger containers so that
                # the search for builtin will stop at the messenger; we're looking
                # for any path to builtin that don't involve the messenger
                visitedObjIds.add(id(object))
                visitedObjIds.add(id(messenger._Messenger__objectEvents))
                visitedObjIds.add(id(messenger._Messenger__callbacks))

                nextObjList.append(object)
                foundBuiltin = False

                # breadth-first search, go until you run out of new objects or you find __builtin__
                while len(nextObjList):
                    if foundBuiltin:
                        break
                    # swap the lists, prepare for the next pass
                    curObjList = nextObjList
                    nextObjList = []
                    assert self.notify.debug('next search iteration, num objects: %s' % len(curObjList))
                    for curObj in curObjList:
                        if foundBuiltin:
                            break
                        yield None
                        referrers = gc.get_referrers(curObj)
                        assert self.notify.debug('curObj: %s @ %s, %s referrers, repr=%s' % (
                            itype(curObj), hex(id(curObj)), len(referrers), fastRepr(curObj, maxLen=2)))
                        for referrer in referrers:
                            #assert self.notify.debug('referrer: %s' % itype(curObj))
                            yield None
                            refId = id(referrer)
                            # don't go in a loop
                            if refId in visitedObjIds:
                                #assert self.notify.debug('already visited')
                                continue
                            # don't self-reference
                            if referrer is curObjList or referrer is nextObjList:
                                continue
                            if refId in builtinIds:
                                # not a leak, there is a path to builtin that does not involve the messenger
                                #assert self.notify.debug('object has another path to __builtin__, it\'s not a messenger leak')
                                foundBuiltin = True
                                break
                            else:
                                visitedObjIds.add(refId)
                                nextObjList.append(referrer)

                if not foundBuiltin:
                    self.notify.warning(
                        '%s is referenced only by the messenger' % (itype(object)))
