'''
Mixin for publishing messages to a topic's listeners. This will be
mixed into topicobj.Topic so that a user can use a Topic object to
send a message to the topic's listeners via a publish() method.

:copyright: Copyright 2006-2009 by Oliver Schoenborn, all rights reserved.
:license: BSD, see LICENSE.txt for details.

'''


from publisherbase import PublisherBase
import policies


class PublisherKwargs(PublisherBase):
    '''
    Publisher used for kwargs protocol, ie when sending message data
    via kwargs.
    '''

    def sendMessage(self, _topicName, **kwargs):
        '''Send message of type _topicName to all subscribed listeners,
        with message data in kwargs. If topicName is a subtopic, listeners
        of topics more general will also get the message. Note also that
        kwargs must be compatible with topic.

        Note that any listener that lets a raised exception escape will
        interrupt the send operation, unless an exception handler was
        specified via pub.setListenerExcHandler().
        '''
        topicMgr = self.getTopicMgr()
        topicObj = topicMgr.getOrCreateTopic(_topicName)

        # don't care if topic not final: topicObj.getListeners()
        # will return nothing if not final but notification will still work

        topicObj.publish(**kwargs)

    def getMsgProtocol(self):
        return 'kwargs'


class PublisherArg1Stage2(PublisherKwargs):
    '''
    This is used when transitioning from arg1 to kwargs
    messaging protocol.
    '''

    class SenderTooManyKwargs(RuntimeError):
        def __init__(self, kwargs, commonArgName):
            extra = kwargs.copy()
            del extra[commonArgName]
            msg = 'Sender has too many kwargs (%s)' % (extra.keys(),)
            RuntimeError.__init__(self, msg)

    class SenderWrongKwargName(RuntimeError):
        def __init__(self, actualKwargName, commonArgName):
            msg = 'Sender uses wrong kwarg name ("%s" instead of "%s")' \
                % (actualKwargName, commonArgName)
            RuntimeError.__init__(self, msg)

    def __init__(self, treeConfig = None):
        PublisherKwargs.__init__(self, treeConfig)
        from datamsg import Message
        self.Msg = Message

    def sendMessage(self, _topicName, **kwarg):
        commonArgName = policies.msgDataArgName
        if len(kwarg) > 1:
            raise self.SenderTooManyKwargs(kwarg, commonArgName)
        elif len(kwarg) == 1 and not kwarg.has_key(commonArgName):
            raise self.SenderWrongKwargName(kwarg.keys()[0], commonArgName)
        
        data = kwarg.get(commonArgName, None)
        kwargs = { commonArgName: self.Msg( _topicName, data) }
        PublisherKwargs.sendMessage( self, _topicName, **kwargs )

    def getMsgProtocol(self):
        return 'kwarg1'


if policies.msgProtocolTransStage is None:
    Publisher = PublisherKwargs
else:
    Publisher = PublisherArg1Stage2
    #print 'Using protocol', Publisher
    

