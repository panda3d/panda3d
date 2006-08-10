"""Undocumented Module"""

__all__ = ['EventGroup']

from direct.showbase import DirectObject
from direct.showbase.PythonUtil import SerialNumGen, Functor

class EventGroup(DirectObject.DirectObject):
    """This class allows you to group together multiple events and treat
    them as a single event. The EventGroup will not send out its event until
    all of its sub-events have occured."""

    _SerialNumGen = SerialNumGen()

    def __init__(self, name, subEvents=None, doneEvent=None):
        """
        Provide a meaningful name to aid debugging.

        doneEvent is optional. If not provided, a unique done event will be
        generated and is available as EventGroup.getDoneEvent().

        Examples:
        
        # waits for gotRed and gotBlue, then sends out 'gotColors'
        EventGroup('getRedAndBlue', ('gotRed', 'gotBlue'), doneEvent='gotColors')

        # waits for two interests to close, then calls self._handleBothInterestsClosed()
        # uses EventGroup.getDoneEvent() and EventGroup.newEvent() to generate unique,
        # disposable event names
        eGroup = EventGroup('closeInterests')
        self.acceptOnce(eGroup.getDoneEvent(), self._handleBothInterestsClosed)
        base.cr.closeInterest(interest1, event=eGroup.newEvent('closeInterest1'))
        base.cr.closeInterest(interest2, event=eGroup.newEvent('closeInterest2'))
        """
        self._name = name
        self._subEvents = set()
        self._completedEvents = set()
        if doneEvent is None:
            # no doneEvent provided, allocate a unique event name
            doneEvent = 'EventGroup-%s-%s-Done' % (
                EventGroup._SerialNumGen.next(), self._name)
        self._doneEvent = doneEvent
        self._completed = False

        if subEvents is not None:
            # add the events that were passed in to start with, more may be added
            # later via newEvent()
            for event in subEvents:
                self.addEvent(event)

    def destroy(self):
        if hasattr(self, '_name'):
            # keep this around
            #del self._doneEvent
            del self._name
            del self._subEvents
            del self._completedEvents
            self.ignoreAll()

    def getName(self):
        return self._name

    def getDoneEvent(self):
        return self._doneEvent

    def isCompleted(self):
        return self._completed

    def addEvent(self, eventName):
        """ Adds a new event to the list of sub-events that we're waiting on.
        Returns the name of the event. """
        if self._completed:
            self.notify.error('addEvent(\'%s\') called on completed EventGroup \'%s\'' % (
                eventName, self.getName()))
        if eventName in self._subEvents:
            self.notify.error('addEvent(\'%s\'): event already in EventGroup \'%s\'' % (
                eventName, self.getName()))
        self._subEvents.add(eventName)
        self.acceptOnce(eventName, Functor(self._subEventComplete, eventName))
        return eventName

    def newEvent(self, name):
        """ Pass in an event name and it will be unique-ified for you and added
        to this EventGroup. TIP: there's no need to repeat information in this event
        name that is already in the name of the EventGroup object.
        Returns the new event name. """
        return self.addEvent('%s-SubEvent-%s-%s' % (
            self._name, EventGroup._SerialNumGen.next(), name))

    def _subEventComplete(self, subEventName, *args, **kwArgs):
        if subEventName in self._completedEvents:
            self.notify.warning('_subEventComplete: \'%s\' already received' %
                                subEventName)
        else:
            self._completedEvents.add(subEventName)
            if self._completedEvents == self._subEvents:
                self._signalComplete()

    def _signalComplete(self):
        self._completed = True
        messenger.send(self._doneEvent)
        self.destroy()
        
    def __repr__(self):
        return '%s(\'%s\', %s, doneEvent=\'%s\') # completed=%s' % (
            self.__class__.__name__,
            self._name,
            tuple(self._subEvents),
            self._doneEvent,
            tuple(self._completedEvents))
