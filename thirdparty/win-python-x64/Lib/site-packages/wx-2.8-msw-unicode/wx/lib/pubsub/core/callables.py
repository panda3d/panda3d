'''
Low level functions and classes related to callables.

The AUTO_TOPIC
is the "marker" to use in callables to indicate that when a message
is sent to those callables, the topic object for that message should be
added to the data sent via the call arguments. See the docs in
CallArgsInfo regarding its autoTopicArgName data member.

:copyright: Copyright 2006-2009 by Oliver Schoenborn, all rights reserved.
:license: BSD, see LICENSE.txt for details.

'''

from inspect import getargspec, ismethod, isfunction


AUTO_TOPIC    = '## your listener wants topic name ## (string unlikely to be used by caller)'


def getModule(obj):
    '''Get the module in which an object was defined. Returns '__main__' 
    if no module defined (which usually indicates either a builtin, or
    a definition within main script). '''
    if hasattr(obj, '__module__'):
        module = obj.__module__
    else:
        module = '__main__'
    return module


def getID(callable_):
    '''Get name and module name for a callable, ie function, bound 
    method or callable instance, by inspecting the callable. E.g. 
    getID(Foo.bar) returns ('Foo.bar', 'a.b') if Foo.bar was
    defined in module a.b. '''
    sc = callable_
    if ismethod(sc):
        module = getModule(sc.im_self)
        id = '%s.%s' % (sc.im_self.__class__.__name__, sc.im_func.func_name)
    elif isfunction(sc):
        module = getModule(sc)
        id = sc.__name__
    else: # must be a functor (instance of a class that has __call__ method)
        module = getModule(sc)
        id = sc.__class__.__name__
        
    return id, module


def getRawFunction(callable_):
    '''Given a callable, return (offset, func) where func is the
    function corresponding to callable, and offset is 0 or 1 to 
    indicate whether the function's first argument is 'self' (1)
    or not (0). Raises ValueError if callable_ is not of a
    recognized type (function, method or has __call__ method).'''
    firstArg = 0
    if isfunction(callable_):
        #print 'Function', getID(callable_)
        func = callable_
    elif ismethod(callable_):
        #print 'Method', getID(callable_)
        func = callable_
        if func.im_self is not None:
            # Method is bound, don't care about the self arg
            firstArg = 1
    elif hasattr(callable_, '__call__'):
        #print 'Functor', getID(callable_)
        func = callable_.__call__
        firstArg = 1  # don't care about the self arg
    else:
        msg = 'type "%s" not supported' % type(callable_).__name__
        raise ValueError(msg)
    
    return func, firstArg

    
class ListenerInadequate(TypeError):
    '''
    Raised when an attempt is made to subscribe a listener to 
    a topic, but listener does not satisfy the topic's listener protocol
    specification (LPS). This specification is inferred from the first
    listener subscribed to a topic, or from an imported topic tree
    specification (see pub.importTopicTree()).
    '''
    
    def __init__(self, msg, listener, *args):
        idStr, module = getID(listener)
        msg = 'Listener "%s" (from module "%s") inadequate: %s' % (idStr, module, msg)
        TypeError.__init__(self, msg)
        self.msg    = msg
        self.args   = args
        self.module = module
        self.idStr  = idStr
        
    def __str__(self):
        return self.msg


class CallArgsInfo:
    '''
    Represent the "signature" or protocol of a listener in the context of 
    topics. 
    '''
    
    def __init__(self, func, firstArgIdx): #args, firstArgIdx, defaultVals, acceptsAllKwargs=False):
        '''Inputs: 
        - Args and defaultVals are the complete set of arguments and
          default values as obtained form inspect.getargspec();
        - The firstArgIdx points to the first item in
          args that is of use, so it is typically 0 if listener is a function,
          and 1 if listener is a method.
        - The acceptsAllKwargs should be true
          if the listener has **kwargs in its protocol. 
        
        After construction,
        - self.allParams will contain the subset of 'args' without first
          firstArgIdx items,
        - self.numRequired will indicate number of required arguments
          (ie self.allParams[:self.numRequired] are the required args names);
        - self.acceptsAllKwargs = acceptsAllKwargs
        - self.autoTopicArgName will be the name of argument
          in which to put the topic object for which pubsub message is 
          sent, or None. This is identified by the argument that has a
          default value of AUTO_TOPIC.

        For instance, listener(self, arg1, arg2=AUTO_TOPIC, arg3=None) will
        have self.allParams = (arg1, arg2, arg3), self.numRequired=1, and
        self.autoTopicArgName = 'arg2', whereas
        listener(self, arg1, arg3=None) will have
        self.allParams = (arg1, arg3), self.numRequired=1, and
        self.autoTopicArgName = None.'''

        #args, firstArgIdx, defaultVals, acceptsAllKwargs
        (allParams, varParamName, varOptParamName, defaultVals) = getargspec(func)
        if defaultVals is None:
            defaultVals = []
        else:
            defaultVals = list(defaultVals)

        self.acceptsAllKwargs      = (varOptParamName is not None)
        self.acceptsAllUnnamedArgs = (varParamName    is not None)
        
        self.allParams = allParams
        del self.allParams[0:firstArgIdx] # does nothing if firstArgIdx == 0

        self.numRequired = len(self.allParams) - len(defaultVals)
        assert self.numRequired >= 0

        # if listener wants topic, remove that arg from args/defaultVals
        self.autoTopicArgName = None
        if defaultVals:
            self.__setupAutoTopic(defaultVals)

    def getAllArgs(self):
        return tuple( self.allParams )

    def getOptionalArgs(self):
        return tuple( self.allParams[self.numRequired:] )

    def getRequiredArgs(self):
        '''Return a tuple of names indicating which call arguments
        are required to be present when pub.sendMessage(...) is called. '''
        return tuple( self.allParams[:self.numRequired] )

    def __setupAutoTopic(self, defaults):
        '''Does the listener want topic of message? Returns < 0 if not, 
        otherwise return index of topic kwarg within args.'''
        for indx, defaultVal in enumerate(defaults):
            if defaultVal == AUTO_TOPIC:
                #del self.defaults[indx]
                firstKwargIdx = self.numRequired
                self.autoTopicArgName = self.allParams.pop(firstKwargIdx + indx)
                break
        

def getArgs(listener):
    '''Returns an instance of CallArgsInfo for the given listener.
    Raises ListenerInadequate if listener is not a callable.'''
    # figure out what is the actual function object to inspect:
    try:
        func, firstArgIdx = getRawFunction(listener)
    except ValueError, exc:
        raise ListenerInadequate(str(exc), listener)

    return CallArgsInfo(func, firstArgIdx)


