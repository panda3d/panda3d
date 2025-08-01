"""Contains the EventManager class.  See :mod:`.EventManagerGlobal` for the
global eventMgr instance."""

from __future__ import annotations

__all__ = ['EventManager']


from typing import Any

from direct.directnotify.DirectNotifyGlobal import directNotify
from direct.directnotify.Notifier import Notifier
from direct.task.TaskManagerGlobal import taskMgr
from direct.showbase.MessengerGlobal import messenger
from panda3d.core import (
    ConfigVariableBool,
    Event,
    EventHandler,
    EventParameter,
    EventQueue,
    PStatCollector,
    PythonTask,
)


class EventManager:

    notify: Notifier | None = None

    def __init__(self, eventQueue: EventQueue | None = None) -> None:
        """
        Create a C++ event queue and handler
        """
        # Make a notify category for this class (unless there already is one)
        if EventManager.notify is None:
            EventManager.notify = directNotify.newCategory("EventManager")

        self.eventQueue = eventQueue
        self.eventHandler: EventHandler | None = None

        self._wantPstats = ConfigVariableBool('pstats-eventmanager', False)

    def doEvents(self) -> None:
        """
        Process all the events on the C++ event queue
        """
        # use different methods for handling events with and without pstats tracking
        # for efficiency
        if self._wantPstats:
            processFunc = self.processEventPstats
        else:
            processFunc = self.processEvent
        assert self.eventQueue is not None
        isEmptyFunc = self.eventQueue.isQueueEmpty
        dequeueFunc = self.eventQueue.dequeueEvent
        while not isEmptyFunc():
            processFunc(dequeueFunc())

    def eventLoopTask(self, task: PythonTask) -> int:
        """
        Process all the events on the C++ event queue
        """
        self.doEvents()
        messenger.send("event-loop-done")
        return task.cont

    def parseEventParameter(self, eventParameter: EventParameter) -> Any:
        """
        Extract the actual data from the eventParameter
        """
        if eventParameter.isInt():
            return eventParameter.getIntValue()
        elif eventParameter.isDouble():
            return eventParameter.getDoubleValue()
        elif eventParameter.isString():
            return eventParameter.getStringValue()
        elif eventParameter.isWstring():
            return eventParameter.getWstringValue()
        elif eventParameter.isTypedRefCount():
            return eventParameter.getTypedRefCountValue()
        elif eventParameter.isEmpty():
            return None
        else:
            # Must be some user defined type, return the ptr
            # which will be downcast to that type.
            return eventParameter.getPtr()

    def processEvent(self, event: Event) -> None:
        """
        Process a C++ event
        Duplicate any changes in processEventPstats
        """
        # **************************************************************
        # ******** Duplicate any changes in processEventPstats *********
        # **************************************************************
        # Get the event name
        eventName = event.name
        if eventName:
            paramList = []
            for eventParameter in event.parameters:
                eventParameterData = self.parseEventParameter(eventParameter)
                paramList.append(eventParameterData)

            # Do not print the new frame debug, it is too noisy!
            assert EventManager.notify is not None
            if EventManager.notify.getDebug() and eventName != 'NewFrame':
                EventManager.notify.debug('received C++ event named: ' + eventName +
                                          ' parameters: ' + repr(paramList))
            # **************************************************************
            # ******** Duplicate any changes in processEventPstats *********
            # **************************************************************
            # Send the event, we used to send it with the event
            # name as a parameter, but now you can use extraArgs for that
            messenger.send(eventName, paramList)

            # Also send the event down into C++ land
            handler = self.eventHandler
            if handler:
                handler.dispatchEvent(event)

        else:
            # An unnamed event from C++ is probably a bad thing
            assert EventManager.notify is not None
            EventManager.notify.warning('unnamed event in processEvent')

    def processEventPstats(self, event: Event) -> None:
        """
        Process a C++ event with pstats tracking
        Duplicate any changes in processEvent
        """
        # ********************************************************
        # ******** Duplicate any changes in processEvent *********
        # ********************************************************
        # Get the event name
        eventName = event.name
        if eventName:
            paramList = []
            for eventParameter in event.parameters:
                eventParameterData = self.parseEventParameter(eventParameter)
                paramList.append(eventParameterData)

            # Do not print the new frame debug, it is too noisy!
            assert EventManager.notify is not None
            if EventManager.notify.getDebug() and eventName != 'NewFrame':
                EventManager.notify.debug('received C++ event named: ' + eventName +
                                          ' parameters: ' + repr(paramList))
            # Send the event, we used to send it with the event
            # name as a parameter, but now you can use extraArgs for that
            # ********************************************************
            # ******** Duplicate any changes in processEvent *********
            # ********************************************************
            name = eventName
            hyphen = name.find('-')
            if hyphen >= 0:
                name = name[0:hyphen]
            pstatCollector = PStatCollector('App:Tasks:eventManager:' + name)
            pstatCollector.start()
            if self.eventHandler:
                cppPstatCollector = PStatCollector(
                    'App:Tasks:eventManager:' + name + ':C++')

            messenger.send(eventName, paramList)

            # Also send the event down into C++ land
            handler = self.eventHandler
            if handler:
                cppPstatCollector.start()
                handler.dispatchEvent(event)
                cppPstatCollector.stop()

            pstatCollector.stop()

        else:
            # An unnamed event from C++ is probably a bad thing
            assert EventManager.notify is not None
            EventManager.notify.warning('unnamed event in processEvent')

    def restart(self) -> None:
        if self.eventQueue is None:
            self.eventQueue = EventQueue.getGlobalEventQueue()

        if self.eventHandler is None:
            if self.eventQueue == EventQueue.getGlobalEventQueue():
                # If we are using the global event queue, then we also
                # want to use the global event handler.
                self.eventHandler = EventHandler.getGlobalEventHandler()
            else:
                # Otherwise, we need our own event handler.
                self.eventHandler = EventHandler(self.eventQueue)

        taskMgr.add(self.eventLoopTask, 'eventManager')

    def shutdown(self) -> None:
        taskMgr.remove('eventManager')

        # Flush the event queue.  We do this after removing the task
        # since the task removal itself might also fire off an event.
        if self.eventQueue is not None:
            self.eventQueue.clear()

    do_events = doEvents
    process_event = processEvent
