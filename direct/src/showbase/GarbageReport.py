from direct.directnotify import DirectNotifyGlobal
from direct.showbase import PythonUtil
import gc

class GarbageReport:
    notify = DirectNotifyGlobal.directNotify.newCategory("GarbageReport")

    def __init__(self, name=None):
        wasOn = PythonUtil.gcDebugOn()
        if not wasOn:
            gc.set_debug(gc.DEBUG_LEAK)
        gc.collect()
        garbage = list(gc.garbage)
        del gc.garbage[:]
        if not wasOn:
            gc.set_debug(0)

        numGarbage = len(garbage)
        # grab the referrers
        referrersByReference = {}
        referrersByNumber = {}
        for i in xrange(numGarbage):
            referrers = gc.get_referrers(garbage[i])
            referrersByReference[i] = list(referrers)
            # look to see if each referrer is another garbage item
            referrersByNumber[i] = list()
            for referrer in referrers:
                try:
                    num = garbage.index(referrer)
                except:
                    num = 'NG' # Not Garbage
                referrersByNumber[i].append(num)
        # grab the referents
        referentsByReference = {}
        referentsByNumber = {}
        for i in xrange(numGarbage):
            referents = gc.get_referents(garbage[i])
            referentsByReference[i] = list(referents)
            # look to see if each referent is another garbage item
            referentsByNumber[i] = list()
            for referent in referents:
                try:
                    num = garbage.index(referent)
                except:
                    num = 'NG' # Not Garbage
                referentsByNumber[i].append(num)

        s = '\n===== GarbageReport: \'%s\' (%s items) =====' % (name, numGarbage)
        if numGarbage > 0:
            # log each individual item with a number in front of it
            s += '\n\n===== Garbage Items ====='
            digits = 0
            n = numGarbage
            while n > 0:
                digits += 1
                n /= 10
            format = '\n%0' + '%s' % digits + 'i:%s \t%s'
            for i in range(len(garbage)):
                s += format % (i, type(garbage[i]), garbage[i])

            format = '\n%0' + '%s' % digits + 'i:%s'
            s += '\n\n===== Referrers By Number (what is referring to garbage item?) NG=NonGarbage ====='
            for i in xrange(numGarbage):
                s += format % (i, referrersByNumber[i])
            s += '\n\n===== Referents By Number (what is garbage item referring to?) NG=NonGarbage ====='
            for i in xrange(numGarbage):
                s += format % (i, referentsByNumber[i])
            s += '\n\n===== Referrers (what is referring to garbage item?) ====='
            for i in xrange(numGarbage):
                s += format % (i, referrersByReference[i])
            s += '\n\n===== Referents (what is garbage item referring to?) ====='
            for i in xrange(numGarbage):
                s += format % (i, referentsByReference[i])

        self.notify.info(s)
