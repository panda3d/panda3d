'''

:copyright: Copyright 2006-2009 by Oliver Schoenborn, all rights reserved.
:license: BSD, see LICENSE.txt for details.

'''

class Message:
    """
    A simple container object for the two components of a message: the 
    topic and the user data. An instance of Message is given to your 
    listener when called by sendMessage(topic). The data is accessed
    via the 'data' attribute, and can be type of object. 
    """
    def __init__(self, topic, data):
        self.topic = topic
        self.data  = data

    def __str__(self):
        return '[Topic: '+`self.topic`+',  Data: '+`self.data`+']'

