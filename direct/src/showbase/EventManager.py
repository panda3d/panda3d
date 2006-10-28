"""Undocumented Module"""

__all__ = ['EventManager']


from MessengerGlobal import *
from direct.task.TaskManagerGlobal import taskMgr
from direct.directnotify.DirectNotifyGlobal import *
from direct.task.Task import Task

# This module may not import pandac.PandaModules, since it is imported
# by the Toontown Launcher before the complete PandaModules have been
# downloaded.
#from pandac.PandaModules import *

class EventManager:

    notify = None

    # for efficiency, only call import once per module
    EventStorePandaNode = None
    EventQueue = None
    EventHandler = None
    
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
        elif (eventParameter.isWstring()):
            return eventParameter.getWstringValue()
        elif (eventParameter.isTypedRefCount()):
            return eventParameter.getTypedRefCountValue()
        else:
            # Must be some user defined type, return the ptr
            # which will be downcast to that type
            ptr = eventParameter.getPtr()

            if EventManager.EventStorePandaNode is None:
                from pandac.PandaModules import EventStorePandaNode
                EventManager.EventStorePandaNode = EventStorePandaNode
            if isinstance(ptr, EventManager.EventStorePandaNode):
                # Actually, it's a kludgey wrapper around a PandaNode
                # pointer.  Return the node.
                ptr = ptr.getValue()

            return ptr
        
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
        if None in (EventManager.EventQueue, EventManager.EventHandler):
            from pandac.PandaModules import EventQueue, EventHandler
            EventManager.EventQueue = EventQueue
            EventManager.EventHandler = EventHandler
        
        if self.eventQueue == None:
            self.eventQueue = EventManager.EventQueue.getGlobalEventQueue()

        if self.eventHandler == None:
            if self.eventQueue == EventManager.EventQueue.getGlobalEventQueue():
                # If we are using the global event queue, then we also
                # want to use the global event handler.
                self.eventHandler = EventManager.EventHandler.getGlobalEventHandler(self.eventQueue)
            else:
                # Otherwise, we need our own event handler.
                self.eventHandler = EventManager.EventHandler(self.eventQueue)

        taskMgr.add(self.eventLoopTask, 'eventManager')

    def shutdown(self):
        taskMgr.remove('eventManager')
