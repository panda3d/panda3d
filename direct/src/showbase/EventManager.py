"""Undocumented Module"""

__all__ = ['EventManager']


from MessengerGlobal import *
from direct.directnotify.DirectNotifyGlobal import *
from direct.task.TaskManagerGlobal import taskMgr
from panda3d.core import PStatCollector, EventQueue, EventHandler

class EventManager:

    notify = None

    # delayed import, since this is imported by the Toontown Launcher
    # before the complete PandaModules have been downloaded.
    PStatCollector = None
    
    def __init__(self, eventQueue = None):
        """
        Create a C++ event queue and handler
        """
        # Make a notify category for this class (unless there already is one)
        if (EventManager.notify == None):
            EventManager.notify = directNotify.newCategory("EventManager")

        self.eventQueue = eventQueue
        self.eventHandler = None

        self._wantPstats = None # no config at this point

    def doEvents(self):
        """
        Process all the events on the C++ event queue
        """
        if self._wantPstats is None:
            self._wantPstats = config.GetBool('pstats-eventmanager', 0)
            EventManager.PStatCollector = PStatCollector
        # use different methods for handling events with and without pstats tracking
        # for efficiency
        if self._wantPstats:
            processFunc = self.processEventPstats
        else:
            processFunc = self.processEvent
        while (not self.eventQueue.isQueueEmpty()):
            processFunc(self.eventQueue.dequeueEvent())

    def eventLoopTask(self, task):
        """
        Process all the events on the C++ event queue
        """
        self.doEvents()
        messenger.send("event-loop-done")
        return task.cont

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
        elif (eventParameter.isEmpty()):
            return None
        else:
            # Must be some user defined type, return the ptr
            # which will be downcast to that type.
            return eventParameter.getPtr()
        
    def processEvent(self, event):
        """
        Process a C++ event
        Duplicate any changes in processEventPstats
        """
        # **************************************************************
        # ******** Duplicate any changes in processEventPstats *********
        # **************************************************************
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
                                          ' parameters: ' + repr(paramList))
            # **************************************************************
            # ******** Duplicate any changes in processEventPstats *********
            # **************************************************************
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

    def processEventPstats(self, event):
        """
        Process a C++ event with pstats tracking
        Duplicate any changes in processEvent
        """
        # ********************************************************
        # ******** Duplicate any changes in processEvent *********
        # ********************************************************
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
                                          ' parameters: ' + repr(paramList))
            # Send the event, we used to send it with the event 
            # name as a parameter, but now you can use extraArgs for that
            # ********************************************************
            # ******** Duplicate any changes in processEvent *********
            # ********************************************************
            if self._wantPstats:
                name = eventName
                hyphen = name.find('-')
                if hyphen >= 0:
                    name = name[0:hyphen]
                pstatCollector = EventManager.PStatCollector('App:Show code:eventManager:' + name)
                pstatCollector.start()
                if self.eventHandler:
                    cppPstatCollector = EventManager.PStatCollector(
                        'App:Show code:eventManager:' + name + ':C++')

            if paramList:
                messenger.send(eventName, paramList)
            else:
                messenger.send(eventName)
            # Also send the event down into C++ land
            if self.eventHandler:
                if self._wantPstats:
                    cppPstatCollector.start()
                self.eventHandler.dispatchEvent(event)
            # ********************************************************
            # ******** Duplicate any changes in processEvent *********
            # ********************************************************

            if self._wantPstats:
                if self.eventHandler:
                    cppPstatCollector.stop()
                pstatCollector.stop()
            
        else:
            # An unnamed event from C++ is probably a bad thing
            EventManager.notify.warning('unnamed event in processEvent')


    def restart(self):
        if self.eventQueue == None:
            self.eventQueue = EventQueue.getGlobalEventQueue()

        if self.eventHandler == None:
            if self.eventQueue == EventQueue.getGlobalEventQueue():
                # If we are using the global event queue, then we also
                # want to use the global event handler.
                self.eventHandler = EventHandler.getGlobalEventHandler()
            else:
                # Otherwise, we need our own event handler.
                self.eventHandler = EventHandler(self.eventQueue)

        taskMgr.add(self.eventLoopTask, 'eventManager')

    def shutdown(self):
        taskMgr.remove('eventManager')

        # Flush the event queue.  We do this after removing the task
        # since the task removal itself might also fire off an event.
        if self.eventQueue is not None:
            self.eventQueue.clear()
