
from libpandaexpressModules import *
from MessengerGlobal import *
from TaskManagerGlobal import *
from DirectNotifyGlobal import *

class EventManager:

    notify = None
    
    def __init__(self):
        """
        Create a C++ event queue and handler
        """

        # Make a notify category for this class (unless there already is one)
        if (EventManager.notify == None):
            EventManager.notify = directNotify.newCategory("EventManager")

        # EventManager.notify.setDebug(1)

        self.eventQueue = EventQueue.getGlobalEventQueue()
        self.eventHandler = EventHandler(self.eventQueue)

    def eventLoop(self, state):
        """
        Process all the events on the C++ event queue
        """
        while (not self.eventQueue.isQueueEmpty()):
            event = self.eventQueue.dequeueEvent()
            self.processEvent(event)
        return Task.cont

    def parseEventParameter(self, eventParameter):
        """
        Extract the actual data from the eventParameter
        """
        if (eventParameter.isInt()):
            return eventParameter.getIntValue()
        elif (eventParameter.isDouble()):
            return eventParameter.getDoubleValue()
        elif (eventParameter.isString()):
            return eventParameter.getStringValue()
        # Must be some user defined type, return the ptr
        # which will be downcast to that type
        else:
            return eventParameter.getPtr()
        
            

    def processEvent(self, event):
        """
        Process a C++ event
        """
        # If the event has a name, throw a Python event with the Pythonified name
	if event.hasName():
            # Get the event name
            eventName = event.getName()

            numParameters = event.getNumParameters()
            paramList = []
            for i in range(numParameters):
                eventParameter = event.getParameter(i)
                eventParameterData = self.parseEventParameter(eventParameter)
                paramList.append(eventParameterData)

            # Do not print the new frame debug, it is too noisy!
            if (eventName != 'NewFrame'):
                EventManager.notify.debug('received C++ event named: ' + eventName +
                                          ' parameters: ' + `paramList`)


            # Send the event, we used to send it with the event 
            # name as a parameter, but now you can use extraArgs for that
            if paramList:
                messenger.send(eventName, paramList)
            else:
                messenger.send(eventName)

            # Also send the event down into C++ land
            self.eventHandler.dispatchEvent(event)
            
        # An unnamed event from C++ is probably a bad thing
        else:
            EventManager.notify.warning('unnamed event in processEvent')

    def restart(self):
        taskMgr.spawnTaskNamed(Task.Task(self.eventLoop), 'eventManager')

    def shutdown(self):
        taskMgr.removeTasksNamed('eventManager')
