
#---------------------------------------------------------------------------
"""
This module provides a publish-subscribe mechanism that allows one part 
of your application to send a "message" and many other parts to receive 
it, without any knowledge of each other. This helps decouple 
modules of your application. 

Receivers are referred to as "listeners", having subscribed to messages 
of a specific "topic". Sending a message is as simple as calling a 
function with a topic name and (optionally) some data that will be sent 
to the listeners.  

Any callable object can be a listener, if it can be called with one 
parameter. Topic names form a hierarchy. Example use::
 
    from wx.lib.pubsub import Publisher as pub
    def aCallable(msg):
        print 'data received', msg.data
    pub.subscribe(aCallable, 'some.topic')
    pub.sendMessage('some.topic', data=123)
    // should see 'data received 123'
    
An important feature of pubsub is that it does not affect callables' 
lifetime: as soon as a listener is no longer in use in your application 
except for being subscribed to pubsub topics, the Python interpreter 
will be able to garbage collect it and pubsub will automatically 
unsubscribe it. See Publisher class docs for more details. 

The version of pubsub in wx.lib is referred to as "pubsub 1". Thanks 
to Robb Shecter and Robin Dunn for having provided its basis pre-2004
and for offering that I take over its maintenance. Pubsub has since 
evolved into a separate package called PyPubSub hosted on SourceForge
(http://pubsub.sourceforge.net), aka "pubsub 3". It has better support 
for message data via keyword arguments, notifications for calls into
pubsub, exception trapping for listeners, and topic tree documentation.

:Author:      Oliver Schoenborn
:Copyright: Copyright 2006-2009 by Oliver Schoenborn, all rights reserved.
:License: BSD, see LICENSE.txt for details.
"""

_implNotes = """
Implementation notes
--------------------

In class PublisherClass, I represent the topics-listener set as a tree
where each node is a topic, and contains a list of listeners of that
topic, and a dictionary of subtopics of that topic. When the publisher
is told to send a message for a given topic, it traverses the tree
down to the topic for which a message is being generated, all
listeners on the way get sent the message.

PublisherClass currently uses a weak listener topic tree to store the
topics for each listener, and if a listener dies before being
unsubscribed, the tree is notified, and the tree eliminates the
listener from itself.

Ideally, _TopicTreeNode would be a generic _TreeNode with named
subnodes, and _TopicTreeRoot would be a generic _Tree with named
nodes, and PublisherClass would store listeners in each node and a topic
tuple would be converted to a path in the tree.  This would lead to a
much cleaner separation of concerns. But time is over, time to move on.
"""


PUBSUB_VERSION = 1 # must be copied into Publisher singleton
VERSION_STR = "1.1.200904.r159"

#---------------------------------------------------------------------------

# for function and method parameter counting:
from inspect import getargspec, ismethod, isfunction
# for weakly bound methods:
from weakref import ref as WeakRef

# from pubsub, for weakly bound methods:
from core.weakmethod import getWeakRef as _getWeakRef

# -----------------------------------------------------------------------------

def _isbound(method):
    """Return true if method is a bound method, false otherwise"""
    assert ismethod(method)
    return method.im_self is not None


def _paramMinCountFunc(function):
    """Given a function, return pair (min,d) where min is minimum # of
    args required, and d is number of default arguments."""
    assert isfunction(function)
    (args, va, kwa, dflt) = getargspec(function)
    lenDef = len(dflt or ())
    lenArgs = len(args or ())
    lenVA = int(va is not None)
    return (lenArgs - lenDef + lenVA, lenDef)


def _paramMinCount(callableObject):
    """
    Given a callable object (function, method or callable instance),
    return pair (min,d) where min is minimum # of args required, and d
    is number of default arguments. The 'self' parameter, in the case
    of methods, is not counted.
    """
    try:
        func = callableObject.__call__.im_func
    except AttributeError:
        try:
            func = callableObject.im_func
        except AttributeError:
            try:
                return _paramMinCountFunc(callableObject)
            except exc:
                raise 'Cannot determine type of callable: %s' % repr(callableObject)
 
    min, d = _paramMinCountFunc(func)
    return min-1, d


def _tupleize(items):
    """Convert items to tuple if not already one, 
    so items must be a list, tuple or non-sequence"""
    if isinstance(items, list):
        raise TypeError, 'Not allowed to tuple-ize a list'
    elif isinstance(items, (str, unicode)) and items.find('.') != -1:
        items = tuple(items.split('.'))
    elif not isinstance(items, tuple):
        items = (items,)
    return items


def _getCallableName(callable):
    """Get name for a callable, ie function, bound 
    method or callable instance"""
    if ismethod(callable):
        return '%s.%s ' % (callable.im_self, callable.im_func.func_name)
    elif isfunction(callable):
        return '%s ' % callable.__name__
    else:
        return '%s ' % callable
    
    
def _removeItem(item, fromList):
    """Attempt to remove item from fromList, return true 
    if successful, false otherwise."""
    try: 
        fromList.remove(item)
        return True
    except ValueError:
        return False
        

# -----------------------------------------------------------------------------

def getStrAllTopics():
    """Function to call if, for whatever reason, you need to know 
    explicitely what is the string to use to indicate 'all topics'."""
    return ''


# alias, easier to see where used
ALL_TOPICS = getStrAllTopics()

# -----------------------------------------------------------------------------


class _NodeCallback:
    """Encapsulate a weak reference to a method of a _TopicTreeNode
    in such a way that the method can be called, if the node is 
    still alive, but the callback does not *keep* the node alive.
    Also, define two methods, preNotify() and noNotify(), which can 
    be redefined to something else, very useful for testing. 
    """
    
    def __init__(self, obj):
        self.objRef = _getWeakRef(obj)
        
    def __call__(self, weakCB):
        notify = self.objRef()
        if notify is not None: 
            self.preNotify(weakCB)
            notify(weakCB)
        else: 
            self.noNotify()
            
    def preNotify(self, dead):
        """'Gets called just before our callback (self.objRef) is called"""
        pass
        
    def noNotify(self):
        """Gets called if the _TopicTreeNode for this callback is dead"""
        pass


class _TopicTreeNode:
    """A node in the topic tree. This contains a list of callables
    that are interested in the topic that this node is associated
    with, and contains a dictionary of subtopics, whose associated
    values are other _TopicTreeNodes. The topic of a node is not stored
    in the node, so that the tree can be implemented as a dictionary
    rather than a list, for ease of use (and, likely, performance).
    
    Note that it uses _NodeCallback to encapsulate a callback for 
    when a registered listener dies, possible thanks to WeakRef.
    Whenever this callback is called, the onDeadListener() function, 
    passed in at construction time, is called (unless it is None).
    """
    
    def __init__(self, topicPath, onDeadListenerWeakCB):
        self.__subtopics = {}
        self.__callables = []
        self.__topicPath = topicPath
        self.__onDeadListenerWeakCB = onDeadListenerWeakCB
        
    def getPathname(self): 
        """The complete node path to us, ie., the topic tuple that would lead to us"""
        return self.__topicPath
    
    def createSubtopic(self, subtopic, topicPath):
        """Create a child node for subtopic"""
        return self.__subtopics.setdefault(subtopic,
                    _TopicTreeNode(topicPath, self.__onDeadListenerWeakCB))
    
    def hasSubtopic(self, subtopic):
        """Return true only if topic string is one of subtopics of this node"""
        return self.__subtopics.has_key(subtopic)
    
    def getNode(self, subtopic):
        """Return ref to node associated with subtopic"""
        return self.__subtopics[subtopic]
    
    def addCallable(self, callable):
        """Add a callable to list of callables for this topic node"""
        try:
            id = self.__callables.index(_getWeakRef(callable))
            return self.__callables[id]
        except ValueError:
            wrCall = _getWeakRef(callable, _NodeCallback(self.__notifyDead))
            self.__callables.append(wrCall)
            return wrCall
            
    def getCallables(self):
        """Get callables associated with this topic node"""
        return [cb() for cb in self.__callables if cb() is not None]
    
    def hasCallable(self, callable):
        """Return true if callable in this node"""
        try: 
            self.__callables.index(_getWeakRef(callable))
            return True
        except ValueError:
            return False
    
    def sendMessage(self, message):
        """Send a message to our callables"""
        deliveryCount = 0
        for cb in self.__callables[:]:
            listener = cb()
            if listener is not None:
                listener(message)
                deliveryCount += 1
        return deliveryCount
    
    def removeCallable(self, callable):
        """Remove weak callable from our node (and return True). 
        Does nothing if not here (and returns False)."""
        try: 
            self.__callables.remove(_getWeakRef(callable))
            return True
        except ValueError:
            return False
        
    def clearCallables(self):
        """Abandon list of callables to caller. We no longer have 
        any callables after this method is called."""
        tmpList = [cb for cb in self.__callables if cb() is not None]
        self.__callables = []
        return tmpList
        
    def __notifyDead(self, dead):
        """Gets called when a listener dies, thanks to WeakRef"""
        #print 'TreeNODE', `self`, 'received death certificate for ', dead
        self.__cleanupDead()
        if self.__onDeadListenerWeakCB is not None:
            cb = self.__onDeadListenerWeakCB()
            if cb is not None: 
                cb(dead)
        
    def __cleanupDead(self):
        """Remove all dead objects from list of callables"""
        self.__callables = [cb for cb in self.__callables if cb() is not None]
        
    def __str__(self):
        """Print us in a not-so-friendly, but readable way, good for debugging."""
        strVal = []
        for callable in self.getCallables():
            strVal.append(_getCallableName(callable))
        for topic, node in self.__subtopics.iteritems():
            strVal.append(' (%s: %s)' %(topic, node))
        return ''.join(strVal)
      
      
class _TopicTreeRoot(_TopicTreeNode):
    """
    The root of the tree knows how to access other node of the 
    tree and is the gateway of the tree user to the tree nodes. 
    It can create topics, and and remove callbacks, etc. 
    
    For efficiency, it stores a dictionary of listener-topics, 
    so that unsubscribing a listener just requires finding the 
    topics associated to a listener, and finding the corresponding
    nodes of the tree. Without it, unsubscribing would require 
    that we search the whole tree for all nodes that contain 
    given listener. Since Publisher is a singleton, it will 
    contain all topics in the system so it is likely to be a large
    tree. However, it is possible that in some runs, unsubscribe()
    is called very little by the user, in which case most unsubscriptions
    are automatic, ie caused by the listeners dying. In this case, 
    a flag is set to indicate that the dictionary should be cleaned up
    at the next opportunity. This is not necessary, it is just an 
    optimization.
    """
    
    def __init__(self):
        self.__callbackDict  = {}
        self.__callbackDictCleanup = 0
        # all child nodes will call our __rootNotifyDead method
        # when one of their registered listeners dies 
        _TopicTreeNode.__init__(self, (ALL_TOPICS,), 
                                _getWeakRef(self.__rootNotifyDead))
        
    def addTopic(self, topic, listener):
        """Add topic to tree if doesnt exist, and add listener to topic node"""
        assert isinstance(topic, tuple)
        topicNode = self.__getTreeNode(topic, make=True)
        weakCB = topicNode.addCallable(listener)
        assert topicNode.hasCallable(listener)

        theList = self.__callbackDict.setdefault(weakCB, [])
        assert self.__callbackDict.has_key(weakCB)
        # add it only if we don't already have it
        try:
            weakTopicNode = WeakRef(topicNode)
            theList.index(weakTopicNode)
        except ValueError:
            theList.append(weakTopicNode)
        assert self.__callbackDict[weakCB].index(weakTopicNode) >= 0
        
    def getTopics(self, listener):
        """Return the list of topics for given listener"""
        weakNodes = self.__callbackDict.get(_getWeakRef(listener), [])
        return [weakNode().getPathname() for weakNode in weakNodes 
                if weakNode() is not None]

    def isSubscribed(self, listener, topic=None):
        """Return true if listener is registered for topic specified. 
        If no topic specified, return true if subscribed to something.
        Use topic=getStrAllTopics() to determine if a listener will receive 
        messages for all topics."""
        weakCB = _getWeakRef(listener)
        if topic is None: 
            return self.__callbackDict.has_key(weakCB)
        else:
            topicPath = _tupleize(topic)
            for weakNode in self.__callbackDict[weakCB]:
                if topicPath == weakNode().getPathname():
                    return True
            return False
            
    def unsubscribe(self, listener, topicList):
        """Remove listener from given list of topics. If topicList
        doesn't have any topics for which listener has subscribed,
        nothing happens."""
        weakCB = _getWeakRef(listener)
        if not self.__callbackDict.has_key(weakCB):
            return
        
        cbNodes = self.__callbackDict[weakCB] 
        if topicList is None:
            for weakNode in cbNodes:
                weakNode().removeCallable(listener)
            del self.__callbackDict[weakCB] 
            return

        for weakNode in cbNodes:
            node = weakNode()
            if node is not None and node.getPathname() in topicList:
                success = node.removeCallable(listener)
                assert success == True
                cbNodes.remove(weakNode)
                assert not self.isSubscribed(listener, node.getPathname())

    def unsubAll(self, topicList, onNoSuchTopic):
        """Unsubscribe all listeners registered for any topic in 
        topicList. If a topic in the list does not exist, and 
        onNoSuchTopic is not None, a call
        to onNoSuchTopic(topic) is done for that topic."""
        for topic in topicList:
            node = self.__getTreeNode(topic)
            if node is not None:
                weakCallables = node.clearCallables()
                for callable in weakCallables:
                    weakNodes = self.__callbackDict[callable]
                    success = _removeItem(WeakRef(node), weakNodes)
                    assert success == True
                    if weakNodes == []:
                        del self.__callbackDict[callable]
            elif onNoSuchTopic is not None: 
                onNoSuchTopic(topic)
            
    def sendMessage(self, topic, message, onTopicNeverCreated):
        """Send a message for given topic to all registered listeners. If 
        topic doesn't exist, call onTopicNeverCreated(topic)."""
        # send to the all-toipcs listeners
        deliveryCount = _TopicTreeNode.sendMessage(self, message)
        # send to those who listen to given topic or any of its supertopics
        node = self
        for topicItem in topic:
            assert topicItem != ''
            if node.hasSubtopic(topicItem):
                node = node.getNode(topicItem)
                deliveryCount += node.sendMessage(message)
            else: # topic never created, don't bother continuing
                if onTopicNeverCreated is not None:
                    onTopicNeverCreated(topic)
                break
        return deliveryCount

    def numListeners(self):
        """Return a pair (live, dead) with count of live and dead listeners in tree"""
        dead, live = 0, 0
        for cb in self.__callbackDict:
            if cb() is None: 
                dead += 1
            else:
                live += 1
        return live, dead
    
    # clean up the callback dictionary after how many dead listeners
    callbackDeadLimit = 10

    def __rootNotifyDead(self, dead):
        #print 'TreeROOT received death certificate for ', dead
        self.__callbackDictCleanup += 1
        if self.__callbackDictCleanup > _TopicTreeRoot.callbackDeadLimit:
            self.__callbackDictCleanup = 0
            oldDict = self.__callbackDict
            self.__callbackDict = {}
            for weakCB, weakNodes in oldDict.iteritems():
                if weakCB() is not None:
                    self.__callbackDict[weakCB] = weakNodes
        
    def __getTreeNode(self, topic, make=False):
        """Return the tree node for 'topic' from the topic tree. If it 
        doesnt exist and make=True, create it first."""
        # if the all-topics, give root; 
        if topic == (ALL_TOPICS,):
            return self
            
        # not root, so traverse tree
        node = self
        path = ()
        for topicItem in topic:
            path += (topicItem,)
            if topicItem == ALL_TOPICS:
                raise ValueError, 'Topic tuple must not contain ""'
            if make: 
                node = node.createSubtopic(topicItem, path)
            elif node.hasSubtopic(topicItem):
                node = node.getNode(topicItem)
            else:
                return None
        # done
        return node
        
    def printCallbacks(self):
        strVal = ['Callbacks:\n']
        for listener, weakTopicNodes in self.__callbackDict.iteritems():
            topics = [topic() for topic in weakTopicNodes if topic() is not None]
            strVal.append('  %s: %s\n' % (_getCallableName(listener()), topics))
        return ''.join(strVal)
        
    def __str__(self):
        return 'all: %s' % _TopicTreeNode.__str__(self)
    
    
# -----------------------------------------------------------------------------

class _SingletonKey: 
    """Used to "prevent" instantiating a _PublisherClass 
    from outside the module"""
    pass


class PublisherClass:
    """
    The publish/subscribe manager.  It keeps track of which listeners
    are interested in which topics (see subscribe()), and sends a
    Message for a given topic to listeners that have subscribed to
    that topic, with optional user data (see sendMessage()).
    
    The three important concepts for pubsub are:
        
    - listener: a function, bound method or
      callable object that can be called with one parameter
      (not counting 'self' in the case of methods). The parameter
      will be a reference to a Message object. E.g., these listeners
      are ok::
          
          class Foo:
              def __call__(self, a, b=1): pass # can be called with only one arg
              def meth(self,  a):         pass # takes only one arg
              def meth2(self, a=2, b=''): pass # can be called with one arg
        
          def func(a, b=''): pass
          
          Foo foo
        
          import pubsub as Publisher
          Publisher.subscribe(foo)           # functor
          Publisher.subscribe(foo.meth)      # bound method
          Publisher.subscribe(foo.meth2)     # bound method
          Publisher.subscribe(func)          # function
          
      The three types of callables all have arguments that allow a call 
      with only one argument. In every case, the parameter 'a' will contain
      the message. 
    
    - topic: a single word, a tuple of words, or a string containing a
      set of words separated by dots, for example: 'sports.baseball'.
      A tuple or a dotted notation string denotes a hierarchy of
      topics from most general to least. For example, a listener of
      this topic::
    
          ('sports','baseball')
    
      would receive messages for these topics::
    
          ('sports', 'baseball')                 # because same
          ('sports', 'baseball', 'highscores')   # because more specific
    
      but not these::
    
           'sports'      # because more general
          ('sports',)    # because more general
          () or ('')     # because only for those listening to 'all' topics
          ('news')       # because different topic
          
    - message: this is an instance of Message, containing the topic for 
      which the message was sent, and any data the sender specified. 
    
    :note: This class is not directly visible to importers of pubsub.
           A singleton instance of it, named Publisher, is created by 
           the module at load time, allowing to write 'Publisher.method()'.
           All the singleton's methods are made accessible at module
           level so that knowing about the singleton is not necessary.
           This does imply that help docs generated from this module via 
           help(pubsub) will show several module-level functions, whose 
           first parameter is `self`. You should ignore that parameter 
           and consider it to be implicitely refering to the singleton. 
           E.g. if help() lists `getDeliveryCount(self)`, you call it as 
           `pubsub.getDeliveryCount()`.

    """
    
    __ALL_TOPICS_TPL = (ALL_TOPICS, )
    PUBSUB_VERSION = PUBSUB_VERSION
    
    def __init__(self, singletonKey):
        """Construct a Publisher. This can only be done by the pubsub 
        module. You just use pubsub.Publisher()."""
        if not isinstance(singletonKey, _SingletonKey):
            raise invalid_argument("Use Publisher() to get access to singleton")
        self.__messageCount  = 0
        self.__deliveryCount = 0
        self.__topicTree     = _TopicTreeRoot()

    #
    # Public API
    #

    def getDeliveryCount(self):
        """How many listeners have received a message since beginning of run"""
        return self.__deliveryCount
    
    def getMessageCount(self):
        """How many times sendMessage() was called since beginning of run"""
        return self.__messageCount
    
    def subscribe(self, listener, topic = ALL_TOPICS):
        """
        Subscribe listener for given topic. If topic is not specified,
        listener will be subscribed for all topics (that listener will 
        receive a Message for any topic for which a message is generated). 
        
        This method may be called multiple times for one listener,
        registering it with many topics.  It can also be invoked many
        times for a particular topic, each time with a different
        listener.  See the class doc for requirements on listener and
        topic.

        :note: The listener is held only by *weak*
               reference.  This means you must ensure you have at
               least one strong reference to listener, otherwise it
               will be DOA ("dead on arrival"). This is particularly
               easy to forget when wrapping a listener method in a
               proxy object (e.g. to bind some of its parameters),
               e.g.::
        
                  class Foo: 
                      def listener(self, event): pass
                  class Wrapper:
                      def __init__(self, fun): self.fun = fun
                      def __call__(self, *args): self.fun(*args)
                  foo = Foo()
                  Publisher().subscribe( Wrapper(foo.listener) ) # whoops: DOA!
                  wrapper = Wrapper(foo.listener)
                  Publisher().subscribe(wrapper) # good!
        
        :note: Calling this method for the same listener, with two
               topics in the same branch of the topic hierarchy, will
               cause the listener to be notified twice when a message
               for the deepest topic is sent. E.g.
               subscribe(listener, 't1') and then subscribe(listener,
               ('t1','t2')) means that when calling sendMessage('t1'),
               listener gets one message, but when calling
               sendMessage(('t1','t2')), listener gets message twice.
        
        """
        self.validate(listener)

        if topic is None: 
            raise TypeError, 'Topic must be either a word, tuple of '\
                             'words, or getStrAllTopics()'
            
        self.__topicTree.addTopic(_tupleize(topic), listener)

    def isSubscribed(self, listener, topic=None):
        """Return true if listener has subscribed to topic specified. 
        If no topic specified, return true if subscribed to something.
        Use topic=getStrAllTopics() to determine if a listener will receive 
        messages for all topics."""
        return self.__topicTree.isSubscribed(listener, topic)
            
    def validate(self, listener):
        """Similar to isValid(), but raises a TypeError exception if not valid"""
        # check callable
        if not callable(listener):
            raise TypeError, 'Listener '+`listener`+' must be a '\
                             'function, bound method or instance.'
        # ok, callable, but if method, is it bound:
        elif ismethod(listener) and not _isbound(listener):
            raise TypeError, 'Listener '+`listener`+\
                             ' is a method but it is unbound!'
                             
        # check that it takes the right number of parameters
        min, d = _paramMinCount(listener)
        if min > 1:
            raise TypeError, 'Listener '+`listener`+" can't"\
                             ' require more than one parameter!'
        if min <= 0 and d == 0:
            raise TypeError, 'Listener '+`listener`+' lacking arguments!'
                             
        assert (min == 0 and d>0) or (min == 1)

    def isValid(self, listener):
        """Return true only if listener will be able to subscribe to 
        Publisher."""
        try: 
            self.validate(listener)
            return True
        except TypeError:
            return False

    def unsubAll(self, topics=None, onNoSuchTopic=None):
        """Unsubscribe all listeners subscribed for topics. Topics can 
        be a single topic (string or tuple) or a list of topics (ie 
        list containing strings and/or tuples). If topics is not 
        specified, all listeners for all topics will be unsubscribed, 
        ie. there will be no topics and no listeners
        left. If onNoSuchTopic is given, it will be called as 
        onNoSuchTopic(topic) for each topic that is unknown.
        """
        if topics is None: 
            del self.__topicTree
            self.__topicTree = _TopicTreeRoot()
            return
        
        # make sure every topics are in tuple form
        if isinstance(topics, list):
            topicList = [_tupleize(x) for x in topics]
        else:
            topicList = [_tupleize(topics)]
            
        # unsub every listener of topics
        self.__topicTree.unsubAll(topicList, onNoSuchTopic)
            
    def unsubscribe(self, listener, topics=None):
        """Unsubscribe listener. If topics not specified, listener is
        completely unsubscribed. Otherwise, it is unsubscribed only 
        for the topic (the usual tuple) or list of topics (ie a list
        of tuples) specified. Nothing happens if listener is not actually
        subscribed to any of the topics.
        
        Note that if listener subscribed for two topics (a,b) and (a,c), 
        then unsubscribing for topic (a) will do nothing. You must 
        use getAssociatedTopics(listener) and give unsubscribe() the returned 
        list (or a subset thereof).
        """
        self.validate(listener)
        topicList = None
        if topics is not None:
            if isinstance(topics, list):
                topicList = [_tupleize(x) for x in topics]
            else:
                topicList = [_tupleize(topics)]
            
        self.__topicTree.unsubscribe(listener, topicList)
        
    def getAssociatedTopics(self, listener):
        """Return a list of topics the given listener is registered with. 
        Returns [] if listener never subscribed.
        
        :attention: when using the return of this method to compare to
                expected list of topics, remember that topics that are
                not in the form of a tuple appear as a one-tuple in
                the return. E.g. if you have subscribed a listener to
                'topic1' and ('topic2','subtopic2'), this method
                returns::
            
                associatedTopics = [('topic1',), ('topic2','subtopic2')]
        """
        return self.__topicTree.getTopics(listener)
    
    def sendMessage(self, topic=ALL_TOPICS, data=None, onTopicNeverCreated=None):
        """Send a message for given topic, with optional data, to
        subscribed listeners. If topic is not specified, only the
        listeners that are interested in all topics will receive message. 
        The onTopicNeverCreated is an optional callback of your choice that 
        will be called if the topic given was never created (i.e. it, or 
        one of its subtopics, was never subscribed to by any listener). 
        It will be called as onTopicNeverCreated(topic)."""
        aTopic  = _tupleize(topic)
        message = Message(aTopic, data)
        self.__messageCount += 1
        
        # send to those who listen to all topics
        self.__deliveryCount += \
            self.__topicTree.sendMessage(aTopic, message, onTopicNeverCreated)
        
    #
    # Private methods
    #

    def __call__(self):
        """Allows for singleton"""
        return self
    
    def __str__(self):
        return str(self.__topicTree)

# Create the Publisher singleton. We prevent users from (inadvertently)
# instantiating more than one object, by requiring a key that is 
# accessible only to module.  From
# this point forward any calls to Publisher() will invoke the __call__
# of this instance which just returns itself.
#
# The only flaw with this approach is that you can't derive a new
# class from Publisher without jumping through hoops.  If this ever
# becomes an issue then a new Singleton implementaion will need to be
# employed.
_key = _SingletonKey()
# The singleton instance of PublisherClass
Publisher = PublisherClass(_key)


#---------------------------------------------------------------------------

from core.datamsg import Message

#---------------------------------------------------------------------------

def setupForTesting():
    """This is used only by testing modules. Use at your own risk ;)"""
    global TopicTreeRoot, TopicTreeNode, paramMinCount, setDeadCallback
    TopicTreeRoot = _TopicTreeRoot
    TopicTreeNode = _TopicTreeNode
    paramMinCount = _paramMinCount

    def _setDeadCallback(newCallback):
        """When a message is sent via sendMessage(), the listener is tested 
        for "livelyhood" (ie there must be at least one place in your code 
        that is still referring to it). If it is dead, newCallback will be 
        called as newCallback(weakref), where weakref is the weak reference
        object created for the listener when the listener subscribed. 
        This is useful primarily for testing."""
        _NodeCallback.preNotify = newCallback

    setDeadCallback = _setDeadCallback

#---------------------------------------------------------------------------

import pubsubconf
pubsubconf.pubModuleLoaded()
del pubsubconf
