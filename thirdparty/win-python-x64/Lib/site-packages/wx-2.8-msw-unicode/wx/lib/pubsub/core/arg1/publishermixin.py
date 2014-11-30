'''
Mixin for publishing messages to a topic's listeners. This will be
mixed into topicobj.Topic so that a user can use a Topic object to
send a message to the topic's listeners via a publish() method.

Note that it is important that the PublisherMixin NOT modify any
state data during message sending, because in principle it could
happen that a listener causes another message of same topic to be
sent (presumably, the listener has a way of preventing infinite
loop).

:copyright: Copyright 2006-2009 by Oliver Schoenborn, all rights reserved.
:license: BSD, see LICENSE.txt for details.

'''


class PublisherMixin:
    def __init__(self):
        pass

    def publish(self, data=None):
        self._publish(data)

    ############## IMPLEMENTATION ###############

    def _mix_prePublish(self, data, topicObj=None, iterState=None):
        '''Called just before the __sendMessage, to perform any argument
        checking, set iterState, etc'''
        return None

    def _mix_callListener(self, listener, data, iterState):
        '''Send the data to given listener.'''
        listener(self, data)
