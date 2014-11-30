'''
Provides the Publisher class, which manages subscribing callables to 
topics and sending messages. 

:copyright: Copyright 2006-2009 by Oliver Schoenborn, all rights reserved.
:license: BSD, see LICENSE.txt for details.

'''

from topicmgr import TopicManager, TreeConfig


class PublisherBase:
    '''
    Represent the class that send messages to listeners of given 
    topics and that knows how to subscribe/unsubscribe listeners
    from topics. 
    '''
    def __init__(self, treeConfig = None):
        '''If treeConfig is None, a default one is created from an
        instance of topics.TreeConfig.'''
        self.__treeConfig = treeConfig or TreeConfig()
        self.__topicMgr = TopicManager(self.__treeConfig)
    
    def getTopicMgr(self):
        '''Get the topic manager created for this publisher.'''
        return self.__topicMgr
    
    def getListenerExcHandler(self):
        '''Get the listener exception handler that was registered
        via setListenerExcHandler(), or None of none registered.'''
        return self.__treeConfig.listenerExcHandler

    def setListenerExcHandler(self, handler):
        '''Set the handler to call when a listener raises an exception
        during a sendMessage(). Without a handler, the send operation
        aborts, whereas with one, the exception information is sent to
        it (where it can be logged, printed, whatever), and
        sendMessage() continues to send messages to remaining listeners.
        The handler must adhere to the pubsub.utils.exchandling.IExcHandler
        API. '''
        self.__treeConfig.listenerExcHandler = handler

    def addNotificationHandler(self, handler):
        '''The handler should be a class that adheres to the API of
        pubsub.utils.INotificationHandler. Whenever one of several
        special operations is performed on pubsub (such as sendMessage),
        each handler registered will get a notification with appropriate
        information about the event.  '''
        self.__treeConfig.notificationMgr.addHandler(handler)

    def clearNotificationHandlers(self):
        '''Remove all notification handlers that were added via
        self.addNotificationHandler(). '''
        self.__treeConfig.notificationMgr.clearHandlers()
        
    def setNotificationFlags(self, **kwargs):
        '''Set the notification flags on or off for each supported
        notification type.'''
        self.__treeConfig.notificationMgr.setFlagStates(**kwargs)

    def getNotificationFlags(self):
        '''Return a dictionary with the states'''
        return self.__treeConfig.notificationMgr.getFlagStates()

    def setTopicUnspecifiedFatal(self, newVal=True, checkExisting=True):
        '''Changes the creation policy for topics. 
        
        If newVal=False, then any future pubsub operation that requires 
        a topic to be created without a listener specification being 
        available, will succeed. This is the default behavior for the 
        pubsub package. This makes pubsub easier to use, but allows topic 
        names with typos to go uncaught in common operations such as 
        sendMessage() and subscribe(). Note that checkExisting is not 
        used for newVal=False.
        
        When called with newVal=True, any future pubsub operation that
        requires a topic to be created without a listener specification being 
        available, will cause pubsub to raise a ListenerSpecIncomplete
        exception. If checkExisting is not given or True, all existing
        topics are validated. A ListenerSpecIncomplete exception will be
        raised if one is found to be incomplete (has isSendable() false).
        
        The previous value of the setting is returned. 
        
        A listener specification is only available to pubsub during a call to 
        pub.subscribe() and a two-argument call to pub.getOrCreateTopic().
        Operations that prefer or require a specification but are not 
        given one, such as sendMessage() and the one-argument call to 
        getOrCreateTopic(), will query each Topic Definition Provider
        registered via pub.addTopicDefnProvider() (if any) until one is
        found. If none is found, the operation will either A. continue
        silently with a non-sendable topic (ie one without a listener
        specification), or B. raise a ListenerSpecIncomplete. Default
        pubsub behavior is (A). Using newVal=True changes this to B,
        newVal=False changes this back to A.

        Note that this method can be used in several ways:

        1. Only use it in your application when something is not working
           as expected: just add a call at the beginning of your app when
           you have a problem with topic messages not being received
           (for instance), and remove it when you have fixed the problem.

        2. Use it from the beginning of your app and never use newVal=False:
           add a call at the beginning of your app and you leave it in
           (forever), and use Topic Definition Providers to provide the
           listener specifications. These are easy to use via the
           pub.importTopicTree().

        3. Use it as in #1 during app development, and once stable, use
           #2. This is easiest to do in combination with
           pub.exportTopicTree().
         '''
        oldVal = self.__treeConfig.raiseOnTopicUnspecified
        self.__treeConfig.raiseOnTopicUnspecified = newVal

        if newVal and checkExisting:
            self.__topicMgr.checkAllTopicsSpecifed()
            
        return oldVal

    def __call__(self):
        '''For backwards compatilibity with pubsub v1 (wxPython).'''
        return self
    
    def sendMessage(self, topicName, *args, **kwargs):
        '''This will be overridden by derived classes that implement
        message-sending for different messaging protocols.'''
        raise NotImplementedError
    
    def subscribe(self, listener, topicName):
        '''Subscribe listener to named topic. Raises ListenerInadequate
        if listener isn't compatible with the topic's args. Returns
        (pub.Listener, success), where success is False if listener was already
        subscribed. The pub.Listener wraps the listener subscribed and
        provides various introspection-based info about the listener. 

        Note that if 'subscribe' notification is on, the handler's
        'notifySubscribe' method is called after subscription.'''
        topicObj = self.__topicMgr.getOrCreateTopic(topicName)
        subscribedListener, success = topicObj.subscribe(listener)
        return subscribedListener, success

    def unsubscribe(self, listener, topicName):
        '''Unsubscribe from given topic. Returns the pub.Listener 
        instance that was used to wrap listener at subscription
        time. Raises an UndefinedTopic or UndefinedSubtopic if
        topicName doesn't exist.
        
        Note that if 'unsubscribe' notification is on, the handler's 
        notifyUnsubscribe() method will be called after unsubscribing. '''
        topicObj = self.__topicMgr.getTopic(topicName)
        unsubdLisnr = topicObj.unsubscribe(listener)
                
        return unsubdLisnr
    
    def unsubAll(self, topicName = None, 
        listenerFilter = None, topicFilter = None):
        '''By default (no args given), unsubscribe all listeners from all
        topics. A listenerFilter can be given so that only the listeners
        that satisfy listenerFilter(listener) == True will be unsubscribed
        (with listener being a pub.Listener wrapper instance for each listener
        subscribed). A topicFilter can also be given so that only topics
        that satisfy topicFilter(topic name) == True will be affected. 
        If only one topic should have listeners unsubscribed, then a topic 
        name 'topicName' can be given *instead* instead of a topic filter. 
        
        Returns the list of all listeners (instances of pub.Listener) that 
        were unsubscribed from the topic tree).

        Note: this method will generate one 'unsubcribe' notification message
        (see pub.setNotificationFlags()) for each listener unsubscribed.'''
        unsubdListeners = []
        
        if topicName is None: 
            # unsubscribe all listeners from all topics
            topicsMap = self.__topicMgr._topicsMap
            for topicName, topicObj in topicsMap.iteritems():
                if topicFilter is None or topicFilter(topicName):
                    tmp = topicObj.unsubscribeAllListeners(listenerFilter)
                    unsubdListeners.extend(tmp)
        
        else:
            topicObj = self.__topicMgr.getTopic(topicName)
            unsubdListeners = topicObj.unsubscribeAllListeners(listenerFilter)
            
        return unsubdListeners


