'''
Everything regarding the concept of topic. 

Note that name 
can be in the 'dotted' format 'topic.sub[.subsub[.subsubsub[...]]]' 
or in tuple format ('topic','sub','subsub','subsubsub',...). E.g.
'nasa.rocket.apollo13' or ('nasa', 'rocket', 'apollo13').

:copyright: Copyright 2006-2009 by Oliver Schoenborn, all rights reserved.
:license: BSD, see LICENSE.txt for details.

'''

__all__ = [
    'TopicManager',
    'UndefinedTopic',
    'ListenerSpecIncomplete',
    'UndefinedSubtopic']


from callables import getID
from topicutils import ALL_TOPICS, \
    tupleize, stringize

from topicexc import \
    UndefinedTopic, \
    ListenerSpecIncomplete

from topicargspec import \
    ArgSpecGiven, \
    ArgsInfo, \
    topicArgsFromCallable

from topicobj import \
    Topic, \
    UndefinedSubtopic

from treeconfig import TreeConfig
from topicdefnprovider import MasterTopicDefnProvider
from topicmgrimpl import getRootTopicSpec


# ---------------------------------------------------------

ARGS_SPEC_ALL     = ArgSpecGiven.SPEC_GIVEN_ALL
ARGS_SPEC_NONE    = ArgSpecGiven.SPEC_GIVEN_NONE


# ---------------------------------------------------------

class TopicManager:
    '''
    Manages the registry of all topics and creation/deletion
    of topics.

    Note that all methods that start with an underscore are part
    of the private API.
    '''
    
    # Allowed return values for isTopicSpecified()
    TOPIC_SPEC_NOT_SPECIFIED   = 0 # false
    TOPIC_SPEC_ALREADY_CREATED = 1 # all other values equate to "true" but different reason
    TOPIC_SPEC_ALREADY_DEFINED = 2


    def __init__(self, treeConfig=None):
        '''The optional treeConfig is an instance of TreeConfig. A
        default one is created if not given. '''
        self._allTopics = None # root of topic tree
        self._topicsMap = {} # registry of all topics
        self.__treeConfig = treeConfig or TreeConfig()
        self.__defnProvider = MasterTopicDefnProvider(self.__treeConfig)

        # define root of all topics
        assert self._allTopics is None
        argsDocs, reqdArgs = getRootTopicSpec()
        desc = 'Root of all topics'
        specGiven = ArgSpecGiven(argsDocs, reqdArgs)
        self._allTopics = self.__createTopic((ALL_TOPICS,), desc, specGiven=specGiven)

    def addDefnProvider(self, provider):
        '''Register provider as topic specification provider. Whenever a 
        topic must be created, the first provider that has a specification
        for the created topic is used to initialize the topic. The given 
        provider must be an object that has a getDescription(topicNameTuple)
        and getArgs(topicNameTuple) that return a description string 
        and a pair (argsDocs, requiredArgs), respectively.

        Note that Nothing is done if provider already added. Returns how
        many providers have been registered, ie if new provider, will be
        1 + last call's return, otherwise (provider had already been added)
        will be same as last call's return value.'''
        return self.__defnProvider.addProvider(provider)
    
    def clearDefnProviders(self):
        '''Remove all registered topic specification providers'''
        self.__defnProvider.clear()

    def getNumDefnProviders(self):
        return self.__defnProvider.getNumProviders()

    def getTopic(self, name, okIfNone=False):
        '''Get the Topic instance that corresponds to the given topic name 
        path. By default, raises an UndefinedTopic or UndefinedSubtopic
        exception if a topic with given name doesn't exist. If
        raiseOnNone=False, returns None instead of raising an exception.'''
        topicNameDotted = stringize(name)
        #if not name:
        #    raise TopicNameInvalid(name, 'Empty topic name not allowed')
        obj = self._topicsMap.get(topicNameDotted, None)
        if obj is not None:
            return obj

        if okIfNone:
            return None

        # NOT FOUND! Determine what problem is and raise accordingly:
        # find the closest parent up chain that does exists:
        parentObj, subtopicNames = self.__getClosestParent(topicNameDotted)
        assert subtopicNames
        
        subtopicName = subtopicNames[0]
        if parentObj is self._allTopics:
            raise UndefinedTopic(subtopicName)

        raise UndefinedSubtopic(parentObj.getName(), subtopicName)

    def newTopic(self, _name, _desc, _required=(), **_argDocs):
        '''Legacy method, kept for backwards compatibility. If topic
        _name already exists, just returns it and does nothing else.
        Otherwise, use getOrCreateTopic() to create it, then set its
        description (_desc) and its listener specification (_argDocs
        and _required). See getOrCreateTopic() for info on the listener
        spec.'''
        topic = self.getTopic(_name, True)
        if topic is None:
            topic = self.getOrCreateTopic(_name)
            topic.setDescription(_desc)
            topic.setMsgArgSpec(_argDocs, _required)
        return topic

    def getOrCreateTopic(self, name, protoListener=None):
        '''Get the topic object for topic of given name, creating it 
        (and any of its missing parent topics) as necessary. This should
        be useful mostly to TopicManager itself.

        Topic creation: The topic definition will be obtained
        from the first registered TopicDefnProvider (see addTopicDefnProvider()
        method) that can provide it. If none is found, then protoListener,
        if given, will be used to extract the specification for the topic
        message arguments.
        
        So the topic object returned will be either
        1. an existing one
        2. a new one whose specification was obtained from a TopicDefnProvider
        3. a new one whose specification was inferred from protoListener
        4. a new one without any specification

        For the first three cases, the Topic is ready for sending messages.
        In the last case, topic.isSendable() is false and the specification
        will be set by the first call to subscribe() (unless you call
        topicObj.setMsgArgSpec() first to set it yourself).

        Note that if the topic gets created, missing intervening parents
        will be created with an empty specification. For instance, if topic
        A exists, and name="A.B.C", then A.B will also be created. It will
        only be complete (sendable) if a topic definition provider had
        its definition.

        Note also that if protoListener given, and topic already defined,
        the method does not check whether protoListener adheres to the
        specification.'''
        obj = self.getTopic(name, okIfNone=True)
        if obj:
            # if object is not sendable but a proto listener was given,
            # update its specification so that it is sendable
            if (protoListener is not None) and not obj.isSendable():
                allArgsDocs, required = topicArgsFromCallable(protoListener)
                obj.setMsgArgSpec(allArgsDocs, required)
            return obj

        # create missing parents
        nameTuple = tupleize(name)
        parentObj = self.__createParentTopics(nameTuple)

        # now the final topic object, args from listener if provided
        desc, specGiven = self.__defnProvider.getDefn(nameTuple)
        # POLICY: protoListener is used only if no definition available
        if specGiven is None:
            if protoListener is None:
                desc = 'UNDOCUMENTED: created without spec'
            else:
                allArgsDocs, required = topicArgsFromCallable(protoListener)
                specGiven = ArgSpecGiven(allArgsDocs, required)
                desc = 'UNDOCUMENTED: created from protoListener "%s" in module %s' % getID(protoListener)

        return self.__createTopic(nameTuple, desc, parent = parentObj, specGiven = specGiven)

    def isTopicSpecified(self, name):
        '''Returns true if the topic has already been specified, false 
        otherwise. If the return value is true, it is in fact an integer > 0 
        that says in what way it is specified: 
        
        - TOPIC_SPEC_ALREADY_DEFINED: as a definition in one of the registered
          topic definition providers
        - TOPIC_SPEC_ALREADY_CREATED: as an object in the topic tree, having a 
          complete specification
        
        So if caller just wants yes/no, just use return value as boolean as in

            if topicMgr.isTopicSpecified(name): pass

        but if reason matters, caller could use (for instance)

            if topicMgr.isTopicSpecified(name) == topicMgr.TOPIC_SPEC_ALREADY_DEFINED: pass

        NOTE: if a topic object of given 'name' exists in topic tree, but
        it does *not* have a complete specification, the return value will
        be false.
        '''
        alreadyCreated = self.getTopic(name, okIfNone=True)
        if alreadyCreated is not None and alreadyCreated.isSendable():
            return self.TOPIC_SPEC_ALREADY_CREATED

        # get definition from provider if required, or raise
        nameTuple = tupleize(name)
        if self.__defnProvider.isDefined(nameTuple):
            return self.TOPIC_SPEC_ALREADY_DEFINED

        return self.TOPIC_SPEC_NOT_SPECIFIED

    def checkAllTopicsSpecifed(self):
        '''Check all topics that have been created and raise a
        ListenerSpecIncomplete exception if one is found that does not 
        have a listener specification. '''
        for topic in self._topicsMap.itervalues():
            if not topic.isSendable():
                raise ListenerSpecIncomplete(topic.getNameTuple())

    def delTopic(self, name):
        '''Undefines the named topic. Returns True if the subtopic was
        removed, false otherwise (ie the topic doesn't exist). Also
        unsubscribes any listeners of topic. Note that it must undefine
        all subtopics to all depths, and unsubscribe their listeners. '''
        # find from which parent the topic object should be removed
        dottedName = stringize(name)
        try:
            #obj = weakref( self._topicsMap[dottedName] )
            obj = self._topicsMap[dottedName]
        except KeyError:
            return False

        #assert obj().getName() == dottedName
        assert obj.getName() == dottedName
        # notification must be before deletion in case
        self.__treeConfig.notificationMgr.notifyDelTopic(dottedName)

        #obj()._undefineSelf_(self._topicsMap)
        obj._undefineSelf_(self._topicsMap)
        #assert obj() is None

        return True

    def getTopics(self, listener):
        '''Get the list of Topic objects that given listener has 
        subscribed to. Keep in mind that the listener can get 
        messages from sub-topics of those Topics.'''
        assocTopics = []
        for topicObj in self._topicsMap.values():
            if topicObj.hasListener(listener):
                assocTopics.append(topicObj)
        return assocTopics        
        
    def __getClosestParent(self, topicNameDotted):
        '''Returns a pair, (closest parent, tuple path from parent). The
        first item is the closest parent topic that exists for given topic.
        The second one is the list of topic names that have to be created
        to create the given topic.

        So if topicNameDotted = A.B.C.D, but only A.B exists (A.B.C and
        A.B.C.D not created yet), then return is (A.B, ['C','D']).
        Note that if none of the branch exists (not even A), then return
        will be [root topic, ['A',B','C','D']). Note also that if A.B.C
        exists, the return will be (A.B.C, ['D']) regardless of whether
        A.B.C.D exists. '''
        subtopicNames = []
        headTail = topicNameDotted.rsplit('.', 1)
        while len(headTail) > 1:
            parentName = headTail[0]
            subtopicNames.insert( 0, headTail[1] )
            obj = self._topicsMap.get( parentName, None )
            if obj is not None:
                return obj, subtopicNames
            
            headTail = parentName.rsplit('.', 1)
            
        subtopicNames.insert( 0, headTail[0] )
        return self._allTopics, subtopicNames
    
    def __createParentTopics(self, topicName):
        '''This will find which parents need to be created such that
        topicName can be created (but doesn't create given topic),
        and creates them. Returns the parent object.'''
        assert self.getTopic(topicName, okIfNone=True) is None
        parentObj, subtopicNames = self.__getClosestParent(stringize(topicName))
        
        # will create subtopics of parentObj one by one from subtopicNames
        if parentObj is self._allTopics:
            nextTopicNameList = []
        else:
            nextTopicNameList = list(parentObj.getNameTuple())
        for name in subtopicNames[:-1]:
            nextTopicNameList.append(name)
            desc, specGiven = self.__defnProvider.getDefn( tuple(nextTopicNameList) )
            if desc is None:
                desc = 'UNDOCUMENTED: created as parent without specification'
            parentObj = self.__createTopic( tuple(nextTopicNameList),
                desc, specGiven = specGiven,  parent = parentObj)
            
        return parentObj
    
    def __createTopic(self, nameTuple, desc, specGiven, parent=None):
        '''Actual topic creation step. Adds new Topic instance
        to topic map, and sends notification message (of topic 
        'pubsub.newTopic') about new topic having been created.'''
        if specGiven is None:
            specGiven = ArgSpecGiven()
        parentAI = None
        if parent:
            parentAI = parent._getListenerSpec()
        argsInfo = ArgsInfo(nameTuple, specGiven, parentAI)
        if (self.__treeConfig.raiseOnTopicUnspecified
            and not argsInfo.isComplete()):
            raise ListenerSpecIncomplete(nameTuple)

        newTopicObj = Topic(self.__treeConfig, nameTuple, desc,
                            argsInfo, parent = parent)
        # sanity checks:
        assert not self._topicsMap.has_key(newTopicObj.getName())
        if parent is self._allTopics:
            assert len( newTopicObj.getNameTuple() ) == 1
        else:
            assert parent.getNameTuple() == newTopicObj.getNameTuple()[:-1]
        assert nameTuple == newTopicObj.getNameTuple()

        # store new object and notify of creation
        self._topicsMap[ newTopicObj.getName() ] = newTopicObj
        self.__treeConfig.notificationMgr.notifyNewTopic(
            newTopicObj, desc, specGiven.reqdArgs, specGiven.argsDocs)
        
        return newTopicObj


def validateNameHierarchy(topicTuple):
    '''Check that names in topicTuple are valid: no spaces, not empty.
    Raise ValueError if fails check. E.g. ('',) and ('a',' ') would
    both fail, but ('a','b') would be ok. '''
    if not topicTuple:
        topicName = stringize(topicTuple)
        errMsg = 'empty topic name'
        raise TopicNameInvalid(topicName, errMsg)
    
    for indx, topic in enumerate(topicTuple):
        errMsg = None
        if topic is None:
            topicName = list(topicTuple)
            topicName[indx] = 'None'
            errMsg = 'None at level #%s'

        elif not topic:
            topicName = stringize(topicTuple)
            errMsg = 'empty element at level #%s'

        elif topic.isspace():
            topicName = stringize(topicTuple)
            errMsg = 'blank element at level #%s'

        if errMsg:
            raise TopicNameInvalid(topicName, errMsg % indx)


