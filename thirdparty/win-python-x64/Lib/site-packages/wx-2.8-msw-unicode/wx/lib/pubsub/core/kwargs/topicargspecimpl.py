'''

:copyright: Copyright 2006-2009 by Oliver Schoenborn, all rights reserved.
:license: BSD, see LICENSE.txt for details.

'''

import weakref

from topicutils import stringize, WeakNone
from validatedefnargs import verifySubset


### Exceptions raised during check() from sendMessage()

class SenderMissingReqdArgs(RuntimeError):
    '''
    Raised when a sendMessage() is missing arguments tagged as
    'required' by pubsub topic of message.
    '''

    def __init__(self, topicName, argNames, missing):
        argsStr = ','.join(argNames)
        missStr = ','.join(missing)
        msg = "Some required args missing in call to sendMessage('%s', %s): %s" \
            % (topicName, argsStr, missStr)
        RuntimeError.__init__(self, msg)


class SenderUnknownOptArgs(RuntimeError):
    '''
    Raised when a sendMessage() has arguments not listed among the topic's
    listener specification.
    '''

    def __init__(self, topicName, argNames, extra):
        argsStr = ','.join(argNames)
        extraStr = ','.join(extra)
        msg = "Some optional args unknown in call to sendMessage('%s', %s): %s" \
            % (topicName, argsStr, extraStr)
        RuntimeError.__init__(self, msg)


class ArgsInfo:
    '''
    Encode the Listener Protocol Specification (LPS) for a given
    topic. ArgsInfos form a tree identical to that of Topics in that
    ArgInfos have a reference to their parent and children ArgInfos,
    created for the parent and children topics.

    The only difference
    between an ArgsInfo and an ArgSpecGiven is that the latter is
    what "user thinks is ok" whereas former has been validated:
    the specification for this topic is a strict superset of the
    specification of its parent, and a strict subset of the
    specification of each of its children. Also, the instance
    can be used to check validity and filter arguments.

    The LPS can be created "empty", ie "incomplete", meaning it cannot
    yet be used to validate listener subscriptions to topics.
    '''

    SPEC_MISSING        = 10 # no args given
    SPEC_COMPLETE       = 12 # all args, but not confirmed via user spec


    def __init__(self, topicNameTuple, specGiven, parentArgsInfo):
        self.topicNameTuple = topicNameTuple
        self.allOptional = () # topic message optional arg names
        self.allDocs     = {} # doc for each arg
        self.allRequired = () # topic message required arg names
        self.argsSpecType = self.SPEC_MISSING
        self.parentAI = WeakNone()
        if parentArgsInfo is not None:
            self.parentAI = weakref.ref(parentArgsInfo)
            parentArgsInfo.__addChildAI(self)
        self.childrenAI = []

        if specGiven.isComplete():
            self.__setAllArgs(specGiven)

    def isComplete(self):
        return self.argsSpecType == self.SPEC_COMPLETE

    def getArgs(self):
        return self.allOptional + self.allRequired

    def numArgs(self):
        return len(self.allOptional) + len(self.allRequired)

    def getReqdArgs(self):
        return self.allRequired

    def getOptArgs(self):
        return self.allOptional

    def getArgsDocs(self):
        return self.allDocs.copy()

    def setArgsDocs(self, docs):
        '''docs is a mapping from arg names to their documentation'''
        if not self.isComplete():
            raise
        for arg, doc in docs.iteritems():
            self.allDocs[arg] = doc

    def check(self, msgKwargs):
        '''Check that the message arguments given satisfy the topic arg
        specification. Raises SenderMissingReqdArgs if some required
        args are missing or not known, and raises SenderUnknownOptArgs if some
        optional args are unknown. '''
        all = set(msgKwargs)
        # check that it has all required args
        needReqd = set(self.allRequired)
        hasReqd = (needReqd <= all)
        if not hasReqd:
            raise SenderMissingReqdArgs(
                self.topicNameTuple, msgKwargs.keys(), needReqd - all)

        # check that all other args are among the optional spec
        optional = all - needReqd
        ok = (optional <= set(self.allOptional))
        if not ok:
            raise SenderUnknownOptArgs( self.topicNameTuple,
                msgKwargs.keys(), optional - set(self.allOptional) )

    def filterArgs(self, msgKwargs):
        '''Returns a dict which contains only those items of msgKwargs
        which are defined for topic. E.g. if msgKwargs is {a:1, b:'b'}
        and topic arg spec is ('a',) then return {a:1}. The returned dict
        is valid only if check(msgKwargs) was called (or
        check(superset of msgKwargs) was called).'''
        assert self.isComplete()
        if len(msgKwargs) == self.numArgs():
            return msgKwargs

        # only keep the keys from msgKwargs that are also in topic's kwargs
        # method 1: SLOWEST
        #newKwargs = dict( (k,msgKwargs[k]) for k in self.__msgArgs.allOptional if k in msgKwargs )
        #newKwargs.update( (k,msgKwargs[k]) for k in self.__msgArgs.allRequired )

        # method 2: FAST:
        #argNames = self.__msgArgs.getArgs()
        #newKwargs = dict( (key, val) for (key, val) in msgKwargs.iteritems() if key in argNames )

        # method 3: FASTEST:
        argNames = set(self.getArgs()).intersection(msgKwargs)
        newKwargs = dict( (k,msgKwargs[k]) for k in argNames )

        return newKwargs

    def hasSameArgs(self, *argNames):
        '''Returns true if self has all the message arguments given, no
        more and no less. Order does not matter. So if getArgs()
        returns ('arg1', 'arg2') then self.hasSameArgs('arg2', 'arg1')
        will return true. '''
        return set(argNames) == set( self.getArgs() )

    def hasParent(self, argsInfo):
        '''return True if self has argsInfo object as parent'''
        return self.parentAI() is argsInfo

    def getCompleteAI(self):
        '''Get the closest arg spec, starting from self and moving to parent,
        that is complete. So if self.isComplete() is True, then returns self,
        otherwise returns parent (if parent.isComplete()), etc. '''
        AI = self
        while AI is not None:
            if AI.isComplete():
                return AI
            AI = AI.parentAI() # dereference weakref
        return None

    def updateAllArgsFinal(self, topicDefn):
        '''This can only be called once, if the construction was done
        with ArgSpecGiven.SPEC_GIVEN_NONE'''
        assert not self.isComplete()
        assert topicDefn.isComplete()
        self.__setAllArgs(topicDefn)

    def __addChildAI(self, childAI):
        assert childAI not in self.childrenAI
        self.childrenAI.append(childAI)

    def __notifyParentCompleted(self):
        '''Parent should call this when parent ArgsInfo has been completed'''
        assert self.parentAI().isComplete()
        if self.isComplete():
            # verify that our spec is compatible with parent's
            self.__validateArgsToParent()
            return

    def __validateArgsToParent(self):
        # validate relative to parent arg spec
        closestParentAI = self.parentAI().getCompleteAI()
        if closestParentAI is not None:
            # verify that parent args is a subset of spec given:
            topicName = stringize(self.topicNameTuple)
            verifySubset(self.getArgs(), closestParentAI.getArgs(), topicName)
            verifySubset(self.allRequired, closestParentAI.getReqdArgs(),
                         topicName, ' required args')

    def __setAllArgs(self, specGiven):
        assert specGiven.isComplete()
        self.allOptional = tuple( specGiven.getOptional() )
        self.allRequired = specGiven.reqdArgs
        self.allDocs     = specGiven.argsDocs.copy() # doc for each arg
        self.argsSpecType= self.SPEC_COMPLETE

        if self.parentAI() is not None:
            self.__validateArgsToParent()

        # notify our children
        for childAI in self.childrenAI:
            childAI.__notifyParentCompleted()


