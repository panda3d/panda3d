'''

:copyright: Copyright 2006-2009 by Oliver Schoenborn, all rights reserved.
:license: BSD, see LICENSE.txt for details.

'''

from topicutils import stringize


class ListenerNotValidatable(RuntimeError):
    '''
    Raised when an attempt is made to validate a listener relative to a 
    topic that doesn't have (yet) a Listener Protocol Specification.
    '''
    def __init__(self):
        msg = 'Topics args not set yet, cannot validate listener'
        RuntimeError.__init__(self, msg)


class UndefinedTopic(RuntimeError):
    '''
    Raised when an attempt is made to retrieve a Topic object
    for a topic name that hasn't yet been created.
    '''
    def __init__(self, topicName, msgFormat=None):
        if msgFormat is None:
            msgFormat = 'Topic "%s" doesn\'t exist'
        RuntimeError.__init__(self, msgFormat % topicName)


class UndefinedSubtopic(UndefinedTopic):
    '''
    Raised when an attempt is made to retrieve a Topic object
    for a subtopic name that hasn't yet been created within
    its parent topic.
    '''
    def __init__(self, parentName, subName):
        msgFormat = 'Topic "%s" doesn\'t have "%%s" as subtopic' % parentName
        UndefinedTopic.__init__(self, subName, msgFormat)


class ListenerSpecIncomplete(RuntimeError):
    '''
    Raised when an attempt is made to create a topic for which
    a specification is not available, but pub.setTopicUnspecifiedFatal()
    was called.
    '''
    def __init__(self, topicNameTuple):
        msg = "No topic specification for topic '%s'."  \
            % stringize(topicNameTuple)
        RuntimeError.__init__(self, msg +
            " See pub.getOrCreateTopic(), pub.addTopicDefnProvider(), and/or pub.setTopicUnspecifiedFatal()")


class ListenerSpecInvalid(RuntimeError):
    '''
    Raised when an attempt is made to define a topic's Listener Protocol
    Specification to something that is not valid.

    The argument names that are invalid can be put in the 'args' list,
    and the msg should say what is the problem and contain "%s" for the
    args, such as ListenerSpecInvalid('duplicate args %s', ('arg1', 'arg2')).
    '''

    def __init__(self, msg, args):
        argsMsg = msg % ','.join(args)
        RuntimeError.__init__(self, 'Invalid listener spec: ' + argsMsg)


class ExcHandlerError(RuntimeError):
    '''
    When an exception gets raised within some listener during a
    sendMessage(), the registered handler (see pub.setListenerExcHandler())
    gets called (via its __call__ method) and the send operation can
    resume on remaining listeners.  However, if the handler itself
    raises an exception while it is being called, the send operation
    must be aborted: an ExcHandlerError exception gets raised.
    '''

    def __init__(self, badExcListenerID, topicObj, origExc=None):
        '''The badExcListenerID is the name of the listener that raised
        the original exception that handler was attempting to handle.
        The topicObj is the pub.Topic object for the topic of the
        sendMessage that had an exception raised. 
        The origExc is currently not used. '''
        self.badExcListenerID = badExcListenerID
        import traceback
        self.exc = traceback.format_exc()
        msg = 'The exception handler registered with pubsub raised an ' \
            + 'exception, *while* handling an exception raised by listener ' \
            + ' "%s" of topic "%s"):\n%s' \
            % (self.badExcListenerID, topicObj.getName(), self.exc)
        RuntimeError.__init__(self, msg)


