
from MessengerGlobal import *
from direct.task.TaskManagerGlobal import *
from direct.directnotify.DirectNotifyGlobal import *

class EventManager:

    notify = None
    
    def __init__(self, eventQueue = None):
        """
        Create a C++ event queue and handler
        """
        # Make a notify category for this class (unless there already is one)
        if (EventManager.notify == None):
            EventManager.notify = directNotify.newCategory("EventManager")

        self.eventQueue = eventQueue
        self.eventHandler = None

    def doEvents(self):
        """
        Process all the events on the C++ event queue
        """
        while (not self.eventQueue.isQueueEmpty()):
            self.processEvent(self.eventQueue.dequeueEvent())

    def eventLoopTask(self, task):
        """
        Process all the events on the C++ event queue
        """
        self.doEvents()
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
        else:
            # Must be some user defined type, return the ptr
            # which will be downcast to that type
            return eventParameter.getPtr()
        
    def processEvent(self, event):
        """
        Process a C++ event
        """
        # Get the event name
        eventName = event.getName()
        if eventName:
            paramList = []
            for i in range(event.getNumParameters()):
                eventParameter = event.getParameter(i)
                eventParameterData = self.parseEventParameter(eventParameter)
                paramList.append(eventParameterData)
            # Do not print the new frame debug, it is too noisy!
            if (EventManager.notify.getDebug() and eventName != 'NewFrame'):
                EventManager.notify.debug('received C++ event named: ' + eventName +
                                          ' parameters: ' + `paramList`)
            # Send the event, we used to send it with the event 
            # name as a parameter, but now you can use extraArgs for that
            if paramList:
                messenger.send(eventName, paramList)
            else:
                messenger.send(eventName)
            # Also send the event down into C++ land
            if self.eventHandler:
                self.eventHandler.dispatchEvent(event)
            
        else:
            # An unnamed event from C++ is probably a bad thing
            EventManager.notify.warning('unnamed event in processEvent')


    def restart(self):
        from pandac.PandaModules import EventQueue, EventHandler
        
        if self.eventQueue == None:
            self.eventQueue = EventQueue.getGlobalEventQueue()

        if self.eventHandler == None:
            if self.eventQueue == EventQueue.getGlobalEventQueue():
                # If we are using the global event queue, then we also
                # want to use the global event handler.
                self.eventHandler = EventHandler.getGlobalEventHandler(self.eventQueue)
            else:
                # Otherwise, we need our own event handler.
                self.eventHandler = EventHandler(self.eventQueue)

        taskMgr.add(self.eventLoopTask, 'eventManager')

    def shutdown(self):
        taskMgr.remove('eventManager')
