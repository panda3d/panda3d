'''

:copyright: Copyright 2006-2009 by Oliver Schoenborn, all rights reserved.
:license: BSD, see LICENSE.txt for details.

'''

from notificationmgr import NotificationMgr


class TreeConfig:
    '''
    Each topic tree has its own topic manager and configuration,
    such as notification and exception handling.
    '''

    def __init__(self, notificationHandler=None, listenerExcHandler=None):
        self.notificationMgr = NotificationMgr(notificationHandler)
        self.listenerExcHandler = listenerExcHandler
        self.raiseOnTopicUnspecified = False


