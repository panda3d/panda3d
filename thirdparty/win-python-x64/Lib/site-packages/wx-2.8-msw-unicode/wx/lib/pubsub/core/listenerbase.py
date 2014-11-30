'''
Higher-level classes and functions related to listening of pubsub messages.
Listeners are callable objects that get subscribed to a pubsub
topic. Listeners are deemed "invalid" or "inadequate" (used interchangeably)
by pubsub if they wouldn't be able to receive all message data of a topic
message. This module includes this validation functionality, which varies
based on the pubsub messaging protocol used. 

Note that a Listener instance holds its callable listener only by weak
reference so it doesn't prevent the callable from being garbage collected
when callable is no longer in use by the application.

In order for a listener to subscribe to a topic, it must adhere to that
topic's "listener protocol specification" (LPS). A subscription will
fail (via a ListenerInadequate exception) if this is not the case. For
instance, if topic A has an LPS "arg1, arg2=None"", then only listeners
of the form callable([self,] arg1, arg2=something) will be accepted.

:copyright: Copyright 2006-2009 by Oliver Schoenborn, all rights reserved.
:license: BSD, see LICENSE.txt for details.

'''

import weakmethod
from callables import \
    getID, getArgs,\
    ListenerInadequate, \
    CallArgsInfo, \
    AUTO_TOPIC as _AUTO_ARG


class ListenerBase:
    '''
    Any listener that is subscribed to pubsub topics is stored by weak
    reference into a Listener (derived from this class). This class uses
    introspection on the wrapped listener to determine various properties of
    interest to pubsub. Anytime a listener is returned from a pubsub
    function/method, it is in fact the Listener wrapping it that is
    returned.

    Note that listeners that have 'argName=pub.AUTO_TOPIC' as a kwarg will
    be given the Topic object for the message sent by sendMessage().
    Such a listener will cause self.wantsTopicObjOnCall() to return True.
    '''
    
    AUTO_TOPIC = _AUTO_ARG
    
    def __init__(self, callable_, argsInfo, onDead=None):
        '''Use callable_ as a listener of topicName. The argsInfo is the 
        return value from a Validator, ie an instance of callables.CallArgsInfo.
        If given, the onDead will be called with self as parameter, if/when
        callable_ gets garbage collected (callable_ is held only by weak
        reference). '''
        # set call policies
        self.acceptsAllKwargs = argsInfo.acceptsAllKwargs
        
        self._autoTopicArgName = argsInfo.autoTopicArgName
        self._callable = weakmethod.getWeakRef(callable_, self.__notifyOnDead)
        self.__onDead = onDead
        
        # save identity now in case callable dies:
        name, mod = getID(callable_)   #
        self.__nameID = name
        self.__module = mod 
        self.__id     = str(id(callable_))[-4:] # only last four digits of id
        self.__hash   = hash(callable_)
    
    def __call__(self, args, kwargs, actualTopic, allArgs=None):
        raise NotImplementedError
    
    def name(self):
        '''Return a human readable name for listener, based on the 
        listener's type name and its id (as obtained
        from id(listener)). If caller just needs name based on 
        type info, specify instance=False. Note that the listener's id()
        was saved at construction time (since it may get garbage collected
        at any time) so the return value of name() is not necessarily unique if the callable has
        died (because id's can be re-used after garbage collection).'''
        return '%s_%s'  % (self.__nameID, self.__id)

    def typeName(self):
        '''Get a type name for the listener. This is a class name or
        function name, as appropriate. '''
        return self.__nameID
    
    def module(self):
        '''Get the module in which the callable was defined.'''
        return self.__module

    def getCallable(self):
        '''Get the listener that was given at initialization. Note that
        this could be None if it has been garbage collected (e.g. if it was 
        created as a wrapper of some other callable, and not stored 
        locally).'''
        return self._callable()

    def isDead(self):
        '''Return True if this listener died (has been garbage collected)'''
        return self._callable() is None

    def wantsTopicObjOnCall(self):
        return self._autoTopicArgName is not None
    
    def _unlinkFromTopic_(self):
        '''Tell self that it is no longer used by a Topic. This allows 
        to break some cyclical references.'''
        self.__onDead = None

    def _calledWhenDead(self):
        raise RuntimeError('BUG: Dead Listener called, still subscribed!')

    def __notifyOnDead(self, ref):
        '''This gets called when listener weak ref has died. Propagate 
        info to Topic).'''
        notifyDeath = self.__onDead
        self._unlinkFromTopic_()
        if notifyDeath is not None:
            notifyDeath(self)

    def __eq__(self, rhs):
        '''Compare for equality to rhs. This returns true if rhs has our id id(rhs) is
        same as id(self) or id(callable in self). '''
        if id(self) == id(rhs):
            return True

        try:
            c1 = self._callable()
            c2 = rhs._callable()

        except Exception:
            # then rhs is not a Listener, compare with c1
            return c1 == rhs

        # both side of == are Listener, but always compare unequal if both dead
        if c2 is None and c1 is None:
            return False
        
        return c1 == c2

    def __ne__(self, rhs):
        '''Counterpart to __eq__ MUST be defined... equivalent to
        'not (self == rhs)'.'''
        return not self.__eq__(rhs)

    def __hash__(self):
        """Hash is an optimization for dict/set searches, it need not
        return different numbers for every different object. """
        return self.__hash

    def __str__(self):
        '''String rep is the callable'''
        return self.__nameID


class ValidatorBase:
    '''
    Validates listeners. It checks whether the listener given to 
    validate() method complies with required and optional arguments
    specified for topic. 
    '''
    
    def __init__(self, topicArgs, topicKwargs):
        '''topicArgs is a list of argument names that will be required when sending 
        a message to listener. Hence order of items in topicArgs matters. The topicKwargs
        is a list of argument names that will be optional, ie given as keyword arguments
        when sending a message to listener. The list is unordered. '''
        self._topicArgs   = set(topicArgs)
        self._topicKwargs = set(topicKwargs)


    def validate(self, listener):
        '''Validate that listener satisfies the requirements of 
        being a topic listener, if topic's kwargs keys are topicKwargKeys
        (so only the list of keyword arg names for topic are necessary). 
        Raises ListenerInadequate if listener not usable for topic. 
        
        Otherwise, returns an CallArgsInfo object containing information about
        the listener's call arguments, such as whether listener wants topic
        name (signified by a kwarg value = AUTO_TOPIC in listener protocol).
        E.g. def fn1(msgTopic=Listener.AUTO_TOPIC) would 
        cause validate(fn1) to return True, whereas any other kwarg name or value 
        would cause a False to be returned. 
        '''
        paramsInfo = getArgs( listener )
        self._validateArgs(listener, paramsInfo)
        return paramsInfo


    def isValid(self, listener):
        '''Return true only if listener can subscribe to messages where
        topic has kwargs keys topicKwargKeys. Just calls validate() in 
        a try-except clause.'''
        try:
            self.validate(listener)
            return True
        except ListenerInadequate:
            return False

    
    def _validateArgs(self, listener, paramsInfo):
        '''Provide implementation in derived classes'''
        raise NotImplementedError
    

