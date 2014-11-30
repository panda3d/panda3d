'''
Manages notifications by pubsub. Pubsub notifies this manager once for
every type of notification. If the corresponding notification flag is
True, all registered notification handlers are called (order undefined)
via the flag's associated method (handler.sendMessage for sendMessage
notification, etc).

Note that this manager automatically unregisters all handlers when
the Python interpreter exits, to help avoid NoneType exceptions during
shutdown. This "shutdown" starts when the last line of you "main" has
executed; the Python interpreter then starts cleaning up, garbage 
collecting everything, which could lead to various pubsub notifications
-- by then they should be of no interest to you -- such as dead
listeners, and even other notifications if a notification handler
where to call upon pubsub. 

:copyright: Copyright 2006-2009 by Oliver Schoenborn, all rights reserved.
:license: BSD, see LICENSE.txt for details.

'''
import sys

class NotificationMgr:

    def __init__(self, notificationHandler = None):
        self.__notifyOnSend = False
        self.__notifyOnSubscribe = False
        self.__notifyOnUnsubscribe = False

        self.__notifyOnNewTopic = False
        self.__notifyOnDelTopic = False
        self.__notifyOnDeadListener = False

        self.__handlers = []
        if notificationHandler is not None:
            self.addHandler(notificationHandler)

        self.__atExitRegistered = False

    def addHandler(self, handler):
        if not self.__atExitRegistered:
            self.__registerForAppExit()
        self.__handlers.append(handler)

    def getHandlers(self):
        return self.__handlers[:]

    def clearHandlers(self):
        self.__handlers = []

    def notifySubscribe(self, *args, **kwargs):
        if self.__notifyOnSubscribe and self.__handlers:
            for handler in self.__handlers:
                handler.notifySubscribe(*args, **kwargs)

    def notifyUnsubscribe(self, *args, **kwargs):
        if self.__notifyOnUnsubscribe and self.__handlers:
            for handler in self.__handlers:
                handler.notifyUnsubscribe(*args, **kwargs)

    def notifySend(self, *args, **kwargs):
        if self.__notifyOnSend and self.__handlers:
            for handler in self.__handlers:
                handler.notifySend(*args, **kwargs)

    def notifyNewTopic(self, *args, **kwargs):
        if self.__notifyOnNewTopic and self.__handlers:
            for handler in self.__handlers:
                handler.notifyNewTopic(*args, **kwargs)

    def notifyDelTopic(self, *args, **kwargs):
        if self.__notifyOnDelTopic and self.__handlers:
            for handler in self.__handlers:
                handler.notifyDelTopic(*args, **kwargs)

    def notifyDeadListener(self, *args, **kwargs):
        if self.__notifyOnDeadListener and self.__handlers:
            for handler in self.__handlers:
                handler.notifyDeadListener(*args, **kwargs)

    def getFlagStates(self):
        '''Return state of each notification flag, as a dict.'''
        return dict(
            subscribe    = self.__notifyOnSubscribe,
            unsubscribe  = self.__notifyOnUnsubscribe,
            deadListener = self.__notifyOnDeadListener,
            sendMessage  = self.__notifyOnSend,
            newTopic     = self.__notifyOnNewTopic,
            delTopic     = self.__notifyOnDelTopic,
            )
    
    def setFlagStates(self, subscribe=None, unsubscribe=None,
        deadListener=None, sendMessage=None, newTopic=None,
        delTopic=None, all=None):
        '''Set the notification flag on/off for various aspects of pubsub:

        - subscribe:    whenever a listener subscribes to a topic;
        - unsubscribe:  whenever a listener unsubscribes from a topic;
        - deadListener: whenever pubsub finds out that a subscribed 
                        listener has been garbage-collected;
        - sendMessage:  whenever sendMessage() is called;
        - newTopic:     whenever a new topic is created;
        - delTopic:     whenever a topic is "deleted" by pubsub;
        - all:          set all of the above to the given value (True or False).

        The kwargs that are None are left at their current value. The 'all'
        is set first, then the others. E.g.

            mgr.setFlagStates(all=True, delTopic=False)

        will toggle all notifications on, but will turn off the 'delTopic'
        notification.

        All registered notification handlers (see pub.addNotificationHandler())
        will be notified when the above actions are taken. 
        '''
        if all is not None:
            # ignore all other arg settings, and set all of them to true:
            numArgs = 7 # how many args in this method
            self.setFlagStates( all=None, * ((numArgs-1)*[all]) )

        if sendMessage is not None:
            self.__notifyOnSend = sendMessage
        if subscribe is not None:
            self.__notifyOnSubscribe = subscribe
        if unsubscribe is not None:
            self.__notifyOnUnsubscribe = unsubscribe

        if newTopic is not None:
            self.__notifyOnNewTopic = newTopic
        if delTopic is not None:
            self.__notifyOnDelTopic = delTopic
        if deadListener is not None:
            self.__notifyOnDeadListener = deadListener


    def __registerForAppExit(self):
        import atexit
        atexit.register(self.clearHandlers)
        self.__atExitRegistered = True
