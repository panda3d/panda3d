'''
This module provides publish-subscribe functions that allow
your methods, functions, and any other callable object to subscribe to 
messages of a given topic, sent from anywhere in your application. 
It therefore provides a powerful decoupling mechanism, e.g. between 
GUI and application logic: senders and listeners don't need to know about 
each other. 

E.g. the following sends a message of type 'MsgType' to a listener, 
carrying data 'some data' (in this case, a string, but could be anything)::

    import pubsub2 as ps
    class MsgType(ps.Message):
        pass
    def listener(msg, data):
        print 'got msg', data
    ps.subscribe(listener, MsgType)
    ps.sendMessage(MsgType('some data'))

The only requirement on your listener is that it be a callable that takes
the message instance as the first argument, and any args/kwargs come after. 
Contrary to pubsub, with pubsub2 the data sent with your message is 
specified in the message instance constructor, and those parameters are
passed on directly to your listener via its parameter list. 

The important concepts of pubsub2 are: 

- topic: the message type. This is a 'dotted' sequence of class names, 
  defined in your messaging module e.g. yourmsgs.py. The sequence
  denotes a hierarchy of topics from most general to least. 
  For example, a listener of this topic::

      Sports.Baseball

  would receive messages for these topics::

      Sports.Baseball              # because same
      Sports.Baseball.Highscores   # because more specific

  but not these::

      Sports     # because more general
      News       # because different topic
    
  Defining a topic hierarchy is trivial: in yourmsgs.py you would do e.g.::

      import pubsub2 as ps
      class Sports(ps.Message):
        class Baseball(ps.Message):
            class Highscores(ps.Message): pass
            class Lowscores(ps.Message):  pass
        class Hockey(ps.Message): 
            class Highscores(ps.Message): pass
            
      ps.setupMsgTree(Sports) # don't forget this!
            
  Note that the above allows you to document your message topic tree 
  using standard Python techniques, and to define specific __init__()
  for your data. 

- listener: a function, bound method or callable object. The first 
  argument will be a reference to a Message object. 
  The order of call of the listeners is not specified. Here are 
  examples of valid listeners (see the Sports.subscribe() calls)::
      
      class Foo:
          def __call__(self, m):       pass
          def meth(self,  m):          pass
          def meth2(self, m, arg1=''): pass # arg1 is optional so valid
      foo = Foo()
    
      def func(m, arg1=None, arg2=''): pass # both arg args are optional
      
      from yourmsgs import Sports
      Sports.Hockey.subscribe(func)       # function
      Sports.Baseball.subscribe(foo.meth) # bound method
      Sports.Hockey.subscribe(foo.meth2)  # bound method
      Sports.Hockey.subscribe(foo)        # functor (Foo.__call__)
      
  In every case, the parameter `m` will contain the message instance, 
  and the remaining arguments are those given to the message constructor.

- message: an instance of a message of a certain type. You create the 
  instance, giving it data via keyword arguments, which become instance
  attributes. E.g. ::

      from yourmsgs import sendMessage, Sports
      sendMessage( Sports.Hockey(a=1, b='c') )
    
  will cause the previous example's `func` listener to get an instance 
  m of Sports.Hockey, with m.a==1 and m.b=='c'. 

  Note that every message instance has a subTopic attribute. If this 
  attribute is not None, it means that the message instance is 
  not for the topic given to the sendMessage(), but for a more 
  generic topic (closer to the root of the message type tree)::

      def handleSports(msg):
        assert msg.subTopic == Sports.Hockey
      def handleHockey(msg):
        assert msg.subTopic == None
      Sports.Hockey.subscribe(handleHockey)
      Sports.subscribe(handleSports)
      sendMessage(Sports.Hockey())

- sender: the part of your code that calls send()::

    # Sports.Hockey is defined in yourmsgs.py, so:
    from yourmsgs import sendMessage, Sports
    # now send something:
    msg = Sports.Hockey(arg1)
    sendMessage( msg ) 

  Note that the above will cause your listeners to be called as 
  f(msg, arg1). 

- log output: using a messaging system has the disadvantage that 
  "tracking" data/events can be more difficult. As an aid, 
  information is sent to a log function, which by default just 
  discards the information. You can set your own logger via 
  setLog() or logToStdOut().  

  An extra string can be given in the send() or 
  subscribe() calls. For send(), this string allows you to identify 
  the "send point": if you don't see it on your log output, then
  you know that your code doesn't reach the call to send(). For 
  subscribe(), it identifies the listener with a string of your choice, 
  otherwise it would be the (rather cryptic) Python name for the listener 
  callable. 

- exceptions while sending: what should happen if a listener (or something
  it calls) raises an exception? The listeners must be independent of each 
  other because the order of calls is not specified. Certain types of 
  exceptions might be handlable by the sender, so simply stopping the 
  send loop is rather extreme. Instead, the send() aggregates the exception
  objects and when it has sent to all listeners, raises a ListenerError 
  exception. This has an attribute `exceptions` that is a list of 
  ExcInfo instances, one for each exception raised during the send(). 

- infinite recursion: it is possible, though not likely, that one of your
  messages causes another message to get sent, which in turn causes the 
  first type of message to get sent again, thereby leading to an infinite
  loop. There is currently no guard against this, though adding one would
  not be difficult.

To summarize: 

- First, create a file e.g. yourmsgs.py in which you define and document
  your message topics tree and in which you call setupMsgTree();
- Subscribe your listeners to some of those topics by importing yourmsgs.py, 
  and calling subscribe() on the message topic to listen for;
- Anywhere in your code, you can send a message by importing yourmsgs.py, 
  and calling `sendMessage( MsgTopicSeq(data) )` or MsgTopic(data).send()
- Debugging your messaging: 
  - If you are not seeing all the messages that you expect, add some 
    identifiers to the send/subscribe calls. 
  - Turn logging on with logToStdOut() (or use setLog(yourLogFunction)
  - The class mechanism will lead to runtime exception if msg topic doesn't
    exist. 

Note: Listeners (callbacks) are held only by weak reference, which in 
general is adequate (this prevents the messaging system from keeping alive
callables that are no longer used by anyone). However, if you want the 
callback to be a wrapper around one of your functions, that wrapper must 
be stored somewhere so that the weak reference isn't the only reference 
to it (which will cause it to die). 


:Author:      Oliver Schoenborn
:Since:       Apr 2004
:Version:     2.01
:Copyright:   \(c) 2007-2009 Oliver Schoenborn
:License:     see LICENSE.txt

'''

PUBSUB_VERSION = 2
VERSION_STR = "2.0a.200810.r153"


import sys, traceback
from core import weakmethod

__all__ = [
    # listener stuff:
    'Listener', 'ListenerError', 'ExcInfo',
    
    # topic stuff:
    'Message',
    
    # publisher stuff:
    'subscribe', 'unsubscribe', 'sendMessage', 
    
    # misc:
    'PUBSUB_VERSION', 'logToStdOut', 'setLog', 'setupMsgTree',
]

def subscribe(listener, MsgClass, id=None):
    '''DEPRECATED (use MsgClass.subscribe() instead). Subscribe 
    listener to messages of type MsgClass. 
    If id is given, it is used to identify the listener in a more 
    human-readable fashion in log messages. Note that log messages 
    are only produced if setLog() was given a non-null writer. '''
    MsgClass.subscribe(listener, id)


def unsubscribe(listener, MsgClass, id=None):
    '''DEPRECATED (use MsgClass.subscribe() instead). Unsubscribe 
    listener to messages of type MsgClass. 
    If id is given, it is used to identify the listener in a more 
    human-readable fashion in log messages. Note that log messages 
    are only produced if setLog() was given a non-null writer. '''
    MsgClass.unsubscribe(listener, id)


def sendMessage(msg, id=None):
    '''Send a message to its registered listeners. The msg is an instance of 
    class derived from Message. If id is given, it is used to identify the 
    sender in a more human-readable fashion in log messages. Note that log 
    messages are only produced if setLog() was given a non-null writer. Note
    also that all listener exceptions are caught, so that all listeners get
    a chance at receiving the message. Once all 
    listeners have been sent the message, a ListenerException will be raised
    containing a list of all exceptions raised during the send.''' 
    msg.send(id)


class ExcInfo:
    '''Represent an exception raised by a listener. It contains the info 
    returned by sys.exc_info() (self.type, self.arg, self.traceback), as
    well as the sender ID (self.senderID), and ID of listener that raised 
    the exception (self.listenerID).'''
    def __init__(self, senderID, listenerID, excInfo):
        self.type = excInfo[0] # class of exception
        self.arg  = excInfo[1] # value given to constructor
        self.traceback  = excInfo[2] # traceback
        self.senderID   = senderID or 'anonymous' # id of sender for which raised
        self.listenerID = listenerID # id of listener in which raised
        
    def __str__(self):
        '''Regular stack-trace message'''
        return ''.join(traceback.format_exception(
            self.type, self.arg, self.traceback))


class ListenerError(RuntimeError):
    '''Gets raised when one or more listeners raise an exception
    while they receive a message. 
    An attribute `exceptions` is a list of ExcInfo objects, one for each 
    exception raised.'''
    def __init__(self, exceps):
        self.exceptions = exceps
        RuntimeError.__init__(self, '%s exceptions raised' % len(exceps))
    def getTracebacks(self):
        '''Get a list of strings, one for each exception's traceback'''
        return [str(ei) for ei in self.exceptions]
    def __str__(self):
        '''Create one long string, where tracebacks are separated by ---'''
        sep = '\n%s\n\n' % ('-'*15)
        return sep.join( self.getTracebacks() )


# the logger used by all text output; defaults to null logger
_log = None


def setLog(writer):
    '''Set the logger used by this module. The 'writer' must be a 
    callable taking one argument (a text string to be logged), 
    or an object that has a write() method, or None to turn off logging. 
    If this function is not called, no logging occurs. Setting a logger
    may be useful to help discover when certain messages are sent but 
    not received, etc. '''
    global _log
    if callable(writer):
        _log = writer
    elif writer is not None:
        _log = writer.write
    else:
        _log = None
        

def logToStdOut():
    '''Shortcut for import sys; setLog(sys.stdout). '''
    import sys
    setLog(sys.stdout)


def setupMsgTree(RootClass, yourModuleLocals=None):
    '''Call this function to setup your message module for use by pubsub2. 
    The RootClass is your class (derived from Message) that is at the root
    of your message tree. The yourModuleLocals, if given, should be 
    locals(). E.g.
    
        #yourMsgs.py:
        import pubsub2 as ps
        class A(ps.Message):
            class B(ps.Message):
                pass
        ps.setupMsgTree(A, locals())
    
    The above does two things: 1. when a message of type B eventually
    gets sent, listeners for messages of type A will also receive it 
    since A is more generic than B; 2. when a module does 
    "import yourMsgs", that module sees pubsub2's functions and 
    classes as though they were in yourMsgs.py, so you can write
    e.g. "yourMsgs.sendMessage()" rather than "yourMsgs.pubsub2.sendMessage()"
    or "import pubsub2; pubsub2.sendMessage()". '''
    RootClass._setupChaining()
    if yourModuleLocals is not None:
        gg = [(key, val) for key, val in globals().iteritems() 
              if not key.startswith('_') and key not in ('setupMsgTree','weakmethod')]
        yourModuleLocals.update(dict(gg))


class Listener:
    '''
    Represent a listener of messages of a given class. An identifier 
    string can accompany the callback, it will be used in text messages.
    Note that callback must give callable(callback) == True.
    Note also that two Listener object compare as equal if they 
    are for the same callback, regardless of id: 
    >>> Listener(cb, 'id1') == Listener(cb, 'id2')
    True
    '''
    
    def __init__(self, callback, id=None):
        assert callable(callback), '%s is not callable' % callback
        self.__callable = weakmethod.getWeakRef(callback)
        self.id = id
        self.weakID = str(self) # save this now in case callable weak ref dies
    
    def getCallable(self):
        '''Get the callback that was given at construction. Note that 
        this could be None if it no longer exists in system (if it was 
        created as a wrapper of some other callable, and not stored 
        locally).'''
        return self.__callable()
    
    def __call__(self, *args, **kwargs):
        cb = self.__callable()
        if cb:
            cb(*args, **kwargs)
        else:
            msg = 'Callback %s no longer exists (maybe it was wrapped?)' % self.weakID
            raise RuntimeError(msg)
        
    def __eq__(self, rhs):
        return self.__callable() == rhs.__callable()
    
    def __str__(self):
        '''String rep is the id, if given, or if not, the str(callback)'''
        return self.id or str(self.__callable())


class Message:
    '''
    Represent a message to be sent from a sender to a listener. 
    This class should be derived, and the derived class should 
    be documented, to help explain the message and its data. 
    E.g. provide a documented __init__() to help explain the data
    carried by the message, the purpose of this type of message, etc.
    '''
    
    _listeners   = None # class-wide registry of listeners
    _parentClass = None # class-wide parent of messages of our type
    _type = 'Message'   # a string for type
    _childrenClasses = None # keep track of children
    
    def __init__(self, subTopic=None, **kwargs):
        '''The kwargs will be given to listener callback when 
        message delivered. Subclasses of Message can define an __init__
        that has specific attributes to better document the message
        data.'''
        self.__kwargs = kwargs
        self.subTopic = subTopic
    
    def __getattr__(self, name):
        if name not in self.__kwargs:
            raise AttributeError("%s instance has no attribute '%s'" \
                % (self.__class__.__name__, name))
        return self.__kwargs[name]

    def send(self, senderID=None):
        '''Send this instance to registered listeners, including listeners
        of more general versions of this message topic. If any listener raises
        an exception, a ListenerError is raised after all listeners have been
        sent the message. The senderID is used in logged output (if setLog() 
        was called) and in ListenerError. '''
        exceps = self.__deliver(senderID)
        
        # make parents up chain send with same data
        ParentCls = self._parentClass
        while ParentCls is not None:
            subTopic = self.subTopic or self.__class__
            msg = ParentCls(subTopic=subTopic, **self.__kwargs)
            ParentCls, exceptInfo = msg.sendSpecific(senderID)
            exceps.extend(exceptInfo)
            
        if exceps:
            raise ListenerError(exceps)
        
    def sendSpecific(self, senderID=None):
        '''Send self to registered listeners, but don't "continue up the 
        message tree", ie listeners of more general versions of this topic
        will not receive the message. See send() for description of senderID.
        Returns self's parent message class and a list of exceptions 
        raised by listeners.'''
        exceptInfo = self.__deliver(senderID)
        return self._parentClass, exceptInfo
        
    def __deliver(self, senderID):
        '''Do the actual message delivery. Logs output if setLog() was 
        called, and accumulates exception information.'''
        if not self._listeners:
            if _log and senderID: 
                _log( 'No listeners of %s for sender "%s"\n' 
                    % (self.getType(), senderID) )
            return []
        
        if _log and senderID:
            _log( 'Message of type %s from sender "%s" should reach %s listeners\n'
                % (self.getType(), senderID, len(self._listeners)) )
            
        received = 0
        exceptInfo = []
        for listener in self._listeners:
            if _log and (senderID or listener.id):
                _log( 'Sending message from sender "%s" to listener "%s"\n' 
                    % (senderID or 'anonymous', str(listener)))
                    
            try:
                listener(self)
                received += 1
            except Exception:
                excInfo = ExcInfo(senderID, str(listener), sys.exc_info())
                exceptInfo.append( excInfo )
    
        if _log and senderID:
            _log( 'Delivered message from sender "%s" to %s listeners\n'
                % (senderID, received))
        
        return exceptInfo
    
    @classmethod 
    def getType(cls):
        '''Return a string representing the type of this message, 
        e.g. A.B.C.'''
        return cls._type
    
    @classmethod
    def hasListeners(cls):
        '''Return True only if at least one listener is registered 
        for this class of messages.'''
        return cls._listeners is not None
    
    @classmethod
    def hasListenersAny(cls):
        '''Return True only if at least one listener is registered 
        for this class of messages OR any of the more general topics.'''
        hasListeners = cls.hasListeners()
        parent = cls._parentClass
        while parent and not hasListeners:
            hasListeners = parent.hasListeners()
            parent = parent._parentClass
        return hasListeners
    
    @classmethod
    def countListeners(cls):
        '''Count how many listeners this class has registered'''
        if cls._listeners:
            return len(cls._listeners)
        return 0
    
    @classmethod
    def countAllListeners(cls):
        '''Count how many listeners will get this type of message'''
        count = cls.countListeners()
        parent = cls._parentClass
        while parent:
            count += parent.countListeners()
            parent = parent._parentClass
        return count

    @classmethod
    def subscribe(cls, who, id=None):
        '''Subscribe `who` to messages of our class.'''
        if _log and id:
            _log( 'Subscribing %s to messages of type %s\n' 
                % (id or who, cls.getType()) )
            
        listener = Listener(who, id)
        if cls._listeners is None: 
            cls._listeners = [listener]
            
        else:
            if listener in cls._listeners:
                idx = cls._listeners.index(listener)
                origListener = cls._listeners[idx]
                if listener.id != origListener.id:
                    if _log:
                        _log('Changing id of Listener "%s" to "%s"\n' 
                             % (origListener.id or who, listener.id or 'anonymous'))
                    origListener.id = listener.id
                    
                elif _log and listener.id:
                    _log( 'Listener %s already subscribed (as "%s")\n' % (who, id) )
                
            else:
                cls._listeners.append( listener )
            
    @classmethod
    def unsubscribe(cls, listener):
        '''Unsubscribe the given listener (given as `who` in subscribe()).
        Does nothing if listener not registered. Unsubscribes all direct 
        listeners if listener is the string 'all'. '''
        if listener == 'all':
            cls._listeners = None
            if _log: 
                _log('Unsubscribed all listeners')
            return 
        
        ll = Listener(listener)
        try:
            idx = cls._listeners.index(ll)
            llID = cls._listeners[idx].id
            del cls._listeners[idx]
        except ValueError:
            if _log:
                _log('Could not unsubscribe listener "%s" from %s' \
                    % (llID or listener, cls._type))
        else:
            if _log:
                _log('Unsubscribed listener "%s"' % llID or listener)
    
    @classmethod
    def clearSubscriptions(cls):
        '''Unsubscribe all listeners of this message type. Same as 
        unsubscribe('all').'''
        cls.unsubscribe('all')
        
        '''Remove all registered listeners from this type of message'''
        cls._listeners = None

    @classmethod
    def getListeners(cls):
        '''Get a list of listeners for this message class. Each 
        item is an instance of Listener.'''
        #_log( 'Listeners of %s: %s' % (cls, cls._listeners) )
        if not cls._listeners:
            return []
        return cls._listeners[:] # return a copy!
    
    @classmethod
    def getAllListeners(cls):
        '''This returns all listeners that will be notified when a send()
        is done on this message type. The return is a dictionary where 
        key is message type, and value is the list of listeners registered 
        for that message type. E.g. A.B.getAllListeners() returns 
        `{['A':[lis1,lis2],'A.B':[lis3]}`.'''        
        ll = {}
        ll[cls._type] = cls.getListeners()
        parent = cls._parentClass
        while parent:
            parentLL = parent.getListeners()
            if parentLL:
                ll[parent._type] = parentLL
            parent = parent._parentClass
        return ll
        
    @classmethod
    def _setupChaining(cls, parents=None):
        '''Chain all the message classes children of cls so that, when a 
        message of type 'cls.childA.subChildB' is sent, listeners of 
        type cls.childA and of type cls get it too. '''
        # parent:
        if parents:
            cls._parentClass = parents[-1]
            lineage = parents[:] + [cls]
            cls._type = '.'.join(item.__name__ for item in lineage)
            if _log:
                _log( '%s will chain up to %s\n' 
                    % (cls._type, cls._parentClass.getType()) )
        else:
            cls._parentClass = None
            lineage = [cls]
            cls._type = cls.__name__
            if _log:
                _log( '%s is at root (top) of messaging tree\n' % cls._type )
        
        # go down into children:
        cls._childrenClasses = []
        for childName, child in vars(cls).iteritems():
            if (not childName.startswith('_')) and issubclass(child, Message):
                cls._childrenClasses.append(child)
                child._setupChaining(lineage)



#---------------------------------------------------------------------------

import pubsubconf
pubsubconf.pubModuleLoaded()
del pubsubconf
