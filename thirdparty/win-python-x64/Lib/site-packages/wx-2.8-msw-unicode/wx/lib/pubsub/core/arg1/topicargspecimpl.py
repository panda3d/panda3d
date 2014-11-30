'''

:copyright: Copyright 2006-2009 by Oliver Schoenborn, all rights reserved.
:license: BSD, see LICENSE.txt for details.

'''

import weakref
from topicutils import WeakNone

class SenderMissingReqdArgs(RuntimeError):
    '''
    Ignore: Not used for this message protocol.
    '''
    pass


class SenderUnknownOptArgs(RuntimeError):
    '''
    Ignore: Not used for this message protocol.
    '''
    pass


class ArgsInfo:
    '''
    Encode the Listener Function Parameter Specification (LPS) for a given
    topic. In the DataMsg API of pubsub, this is the same for all
    topics, so this is quite a simple class, required only because
    the part of pubsub that uses ArgsInfos supports several API
    versions using one common API. So the only difference between
    an ArgsInfo and an ArgSpecGiven is that ArgsInfo refers to
    parent topic's ArgsInfo; other data members are the same.

    Note that the LPS is always complete because it is known:
    it consists of one required argument named 'data' and no
    optional arguments.
    '''

    SPEC_MISSING        = 10 # no args given
    SPEC_COMPLETE       = 12 # all args, but not confirmed via user spec

    def __init__(self, topicNameTuple, specGiven, parentArgsInfo):
        self.__argsDocs = specGiven.argsDocs or {'data':'message data'}
        
        self.argsSpecType = self.SPEC_COMPLETE
        self.allOptional = ()        # list of topic message optional argument names
        self.allRequired = ('data',) # list of topic message required argument names

        self.parentAI = WeakNone()
        if parentArgsInfo is not None:
            self.parentAI = weakref.ref(parentArgsInfo)

    def isComplete(self):
        return True

    def getArgs(self):
        return self.allOptional + self.allRequired

    def numArgs(self):
        return len(self.allOptional) + len(self.allRequired)

    def getArgsDocs(self):
        return self.__argsDocs.copy()


