import FSM
from PandaModules import *
import Task
import string


class ClassicStyle(FSM.FSM):

    def __init__(self, name):
        FSM.FSM.__init__(self, name)

        self.defaultTransitions = {
            'Red' : ['Green'],
            'Yellow' : ['Red'],
            'Green' : ['Yellow'],
            }

    def enterRed(self, oldState, newState):
        print "enterRed(self, '%s', '%s')" % (oldState, newState)

    def exitRed(self, oldState, newState):
        print "exitRed(self, '%s', '%s')" % (oldState, newState)

    def enterYellow(self, oldState, newState):
        print "enterYellow(self, '%s', '%s')" % (oldState, newState)

    def exitYellow(self, oldState, newState):
        print "exitYellow(self, '%s', '%s')" % (oldState, newState)

    def enterGreen(self, oldState, newState):
        print "enterGreen(self, '%s', '%s')" % (oldState, newState)

    def exitGreen(self, oldState, newState):
        print "exitGreen(self, '%s', '%s')" % (oldState, newState)


class NewStyle(FSM.FSM):

    def enterRed(self, oldState, newState):
        print "enterRed(self, '%s', '%s')" % (oldState, newState)

    def filterRed(self, request, args):
        print "filterRed(self, '%s', %s)" % (request, args)
        if request == 'advance':
            return 'Green'
        return self.defaultFilter(request, args)

    def exitRed(self, oldState, newState):
        print "exitRed(self, '%s', '%s')" % (oldState, newState)

    def enterYellow(self, oldState, newState):
        print "enterYellow(self, '%s', '%s')" % (oldState, newState)

    def filterYellow(self, request, args):
        print "filterYellow(self, '%s', %s)" % (request, args)
        if request == 'advance':
            return 'Red'
        return self.defaultFilter(request, args)

    def exitYellow(self, oldState, newState):
        print "exitYellow(self, '%s', '%s')" % (oldState, newState)

    def enterGreen(self, oldState, newState):
        print "enterGreen(self, '%s', '%s')" % (oldState, newState)

    def filterGreen(self, request, args):
        print "filterGreen(self, '%s', %s)" % (request, args)
        if request == 'advance':
            return 'Yellow'
        return self.defaultFilter(request, args)

    def exitGreen(self, oldState, newState):
        print "exitGreen(self, '%s', '%s')" % (oldState, newState)


class ToonEyes(FSM.FSM):
    def __init__(self):
        FSM.FSM.__init__(self, 'eyes')

        self.__unblinkName = "unblink"

        # Eyes are initially open.
        self.request('Open')

    def defaultFilter(self, request, args):
        # The default filter accepts any direct state request (these
        # start with a capital letter).
        if request[0] in string.uppercase:
            return request

        # Unexpected command requests are quietly ignored.
        return None

    def enterOpen(self, oldState, newState):
        print "swap in eyes open model"

    def filterOpen(self, request, args):
        if request == 'blink':
            taskMgr.remove(self.__unblinkName)
            taskMgr.doMethodLater(0.125, self.__unblink, self.__unblinkName)
            return 'Closed'
        return self.defaultFilter(request, args)

    def __unblink(self, task):
        self.request('unblink')
        return Task.done

    def enterClosed(self, oldState, newState):
        print "swap in eyes closed model"

    def filterClosed(self, request, args):
        if request == 'unblink':
            return 'Open'
        return self.defaultFilter(request, args)

    def enterSurprised(self, oldState, newState):
        print "swap in eyes surprised model"

    def enterOff(self, oldState, newState):
        taskMgr.remove(self.__unblinkName)


####
#### Example of using ClassicStyle:
##
## >>> import SampleFSM
## >>> foo = SampleFSM.ClassicStyle('foo')
## >>> foo.request('Red')
## enterRed(self, 'Off', 'Red')
## 'Red'
## >>> foo.request('Yellow')
## Traceback (most recent call last):
##   File "<stdin>", line 1, in ?
##   File "/home/drose/player/direct/src/fsm/FSM.py", line 168, in request
##     result = func(request, args)
##   File "/home/drose/player/direct/src/fsm/FSM.py", line 210, in defaultFilter
##     self.notify.error("%s rejecting request %s from state %s." % (self.name, request, self.state))
##   File "/home/drose/player/direct/src/directnotify/Notifier.py", line 99, in error
##     raise exception(errorString)
## StandardError: foo rejecting request Yellow from state Red.
## >>> foo.request('Green')
## exitRed(self, 'Red', 'Green')
## enterGreen(self, 'Red', 'Green')
## 'Green'
## >>> 

####
#### Example of using NewStyle:
##
## >>> import SampleFSM
## >>> foo = SampleFSM.NewStyle('foo')
## >>> foo.request('Red')
## enterRed(self, 'Off', 'Red')
## 'Red'
## >>> foo.request('advance')
## filterRed(self, 'advance', ())
## exitRed(self, 'Red', 'Green')
## enterGreen(self, 'Red', 'Green')
## 'Green'
## >>> foo.request('advance')
## filterGreen(self, 'advance', ())
## exitGreen(self, 'Green', 'Yellow')
## enterYellow(self, 'Green', 'Yellow')
## 'Yellow'
## >>> foo.request('advance')
## filterYellow(self, 'advance', ())
## exitYellow(self, 'Yellow', 'Red')
## enterRed(self, 'Yellow', 'Red')
## 'Red'
## >>> foo.request('advance')
## filterRed(self, 'advance', ())
## exitRed(self, 'Red', 'Green')
## enterGreen(self, 'Red', 'Green')
## 'Green'
## >>> 

####
#### Example of using ToonEyes:
##
## >>> from ShowBaseGlobal import *
## >>> import SampleFSM
## >>> eyes = SampleFSM.ToonEyes()
## swap in eyes open model
## >>> eyes.request('blink')
## swap in eyes closed model
## 'Closed'
## >>> run()
## swap in eyes open model
## >>> eyes.request('Surprised')
## swap in eyes surprised model
## 'Surprised'
## >>> eyes.request('blink')
## >>> 
