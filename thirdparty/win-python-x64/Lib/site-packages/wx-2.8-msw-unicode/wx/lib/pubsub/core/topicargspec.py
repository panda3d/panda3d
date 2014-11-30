'''
Definitions related to listener signature specification.

:copyright: Copyright 2006-2009 by Oliver Schoenborn, all rights reserved.
:license: BSD, see LICENSE.txt for details.

'''


from listener import getArgs as getListenerArgs
from validatedefnargs import ListenerSpecInvalid
from topicargspecimpl import SenderMissingReqdArgs, SenderUnknownOptArgs, ArgsInfo


def topicArgsFromCallable(_callable):
    '''Get the topic arguments and list of those that are required, 
    by introspecting given listener. Returns a pair, (args, required)
    where args is a dictionary of allowed arguments, and required
    states which args are required rather than optional.'''
    argsInfo = getListenerArgs(_callable)
    required = argsInfo.getRequiredArgs()
    defaultDoc = 'UNDOCUMENTED'
    args = dict.fromkeys(argsInfo.allParams, defaultDoc)
    return args, required


class ArgSpecGiven:
    '''
    The listener protocol specification (LPS) given for a topic
    *by an application* (user).
    This consists of each argument name that listener should have in its
    call protocol, plus which ones are required in any sendMessage(), and a
    documentation string for each argument. This instance will be transformed
    into an ArgsInfo object which is basically a superset of that information,
    needed to make sure the arguments specifications satisfy
    pubsub policies for chosen API version.
    '''

    SPEC_GIVEN_NONE     = 1 # specification not given
    SPEC_GIVEN_ALL      = 3 # all args specified

    def __init__(self, argsDocs=None, reqdArgs=None):
        self.reqdArgs = tuple(reqdArgs or ())

        if argsDocs is None:
            self.argsSpecType = ArgSpecGiven.SPEC_GIVEN_NONE
            self.argsDocs = {}
        else:
            self.argsSpecType = ArgSpecGiven.SPEC_GIVEN_ALL
            self.argsDocs = argsDocs

            # check that all args marked as required are in argsDocs
            missingArgs = set(self.reqdArgs).difference(self.argsDocs.keys())
            if missingArgs:
                msg = 'Params [%s] missing inherited required args [%%s]' % ','.join(argsDocs.keys())
                raise ListenerSpecInvalid(msg, missingArgs)

    def setAll(self, allArgsDocs, reqdArgs = None):
        self.argsDocs     = allArgsDocs
        self.reqdArgs     = reqdArgs or ()
        self.argsSpecType = ArgSpecGiven.SPEC_GIVEN_ALL

    def isComplete(self):
        '''Returns True if the definition is usable, false otherwise.'''
        return self.argsSpecType == ArgSpecGiven.SPEC_GIVEN_ALL

    def getOptional(self):
        return tuple( set( self.argsDocs.keys() ).difference( self.reqdArgs ) )

    def __str__(self):
        return "%s, %s, %s" % \
            (self.argsDocs, self.reqdArgs, self.argsSpecType)


