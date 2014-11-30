'''
Some utility classes for exception handling of exceptions raised
within listeners:

- IExcHandler: base class for any handler that would be given to
  pub.setListenerExcHandler(). The derived class will probably
  want to make use of TracebackInfo.
- TracebackInfo: convenient way of getting stack trace of latest
  exception raised. The handler can create the instance to retrieve
  the stack trace and then log it, present it to user, etc.
- ExcPublisher: example handler that publishes a message containing
  traceback info

:copyright: Copyright 2006-2009 by Oliver Schoenborn, all rights reserved.
:license: BSD, see LICENSE.txt for details.

'''


import sys, traceback


class TracebackInfo:
    '''
    Represent the traceback information for when an exception is 
    raised -- but not caught -- in a listener. The complete 
    traceback cannot be stored since this leads to circular 
    references (see docs for sys.exc_info()) which keeps 
    listeners alive even after the application is no longer 
    referring to them. 
    
    Instances of this object are given to listeners of the 
    'uncaughtExcInListener' topic as the excTraceback kwarg.
    The instance calls sys.exc_info() to get the traceback
    info but keeps only the following info: 
    
     * self.ExcClass: the class of exception that was raised and not caught
     * self.excArg: the argument given to exception when raised
     * self.traceback: list of quadruples as returned by traceback.extract_tb()
       
    Normally you just need to call one of the two getFormatted() methods. 
    '''
    def __init__(self):
        tmpInfo = sys.exc_info()
        self.ExcClass = tmpInfo[0]
        self.excArg   = tmpInfo[1]
        # for the traceback, skip the first 3 entries, since they relate to
        # implementation details for pubsub. 
        self.traceback = traceback.extract_tb(tmpInfo[2])[3:]
        # help avoid circular refs
        del tmpInfo

    def getFormattedList(self):
        '''Get a list of strings as returned by the traceback module's
        format_list() and format_exception_only() functions.'''
        tmp = traceback.format_list(self.traceback)
        tmp.extend( traceback.format_exception_only(self.ExcClass, self.excArg) )
        return tmp
    
    def getFormattedString(self):
        '''Get a string similar to the stack trace that gets printed 
        to stdout by Python interpreter when an exception is not caught.'''
        return ''.join(self.getFormattedList())

    def __str__(self):
        return self.getFormattedString()


class IExcHandler:
    '''
    Interface class for the exception handler. Such handler is called
    whenever a listener raises an exception during a pub.sendMessage().
    You may give an instance of a class derived from IExcHandler to 
    pub.setListenerExcHandler(). Example::

        from pubsub.utils.exchandling import IExcHandler
        class MyHandler(IExcHandler):
            def __call__(self, listenerID, topicObj):
                ... do something with listenerID ...
    '''
    def __call__(self, listenerID, topicObj):
        raise NotImplementedError('%s must override __call__()' % self.__class__)


class ExcPublisher(IExcHandler):
    '''
    Example exception handler that simply publishes the exception traceback.
    The topic used will be self.topicUncaughtExc.
    '''

    # name of the topic
    topicUncaughtExc = 'uncaughtExcInListener'

    def __init__(self, topicMgr=None):
        '''If topic manager is specified, will automatically call init().
        Otherwise, caller must call init() after pubsub imported. See
        pub.setListenerExcHandler().'''
        if topicMgr is not None:
            self.init(topicMgr)

    def init(self, topicMgr):
        '''Must be called only after pubsub has been imported since this
        handler creates a pubsub topic.'''
        obj = topicMgr.getOrCreateTopic(self.topicUncaughtExc)
        obj.setDescription('generated when a listener raises an exception')
        obj.setMsgArgSpec( dict(
            listenerStr  = 'string representation of listener',
            excTraceback = 'instance of TracebackInfo containing exception info'))
        self.__topicObj = obj
        
    def __call__(self, listenerID, topicObj):
        '''Handle the exception raised by given listener. Send the
        Traceback to all subscribers of topic self.topicUncaughtExc. '''
        tbInfo = TracebackInfo()
        self.__topicObj.publish(listenerStr=listenerID, excTraceback=tbInfo)


