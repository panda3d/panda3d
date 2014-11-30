'''
Core package of pubsub, holding the publisher, listener, and topic
object modules. Functions defined here are used internally by
pubsub so that the right modules can be found later, based on the
selected messaging protocol.

Indeed some of the API depends on the messaging
protocol used. For instance sendMessage(), defined in publisher.py,
has a different signature (and hence implementation) for the kwargs
protocol than for the arg1 protocol.

The most convenient way to
support this is to put the parts of the package that differ based
on protocol in separate folders, and add one of those folders to
the package's __path__ variable (defined automatically by the Python
interpreter when __init__.py is executed). For instance, code
specific to the kwargs protocol goes in the kwargs folder, and code
specific to the arg1 protocol in the arg1 folder. Then when doing
"from pubsub.core import listener", the correct listener.py will be
found for the specified protocol. The default protocol is kwargs.

Only one protocol can be used in an application. The default protocol,
if none is chosen by user, is kwargs, as selected by the call to
_prependModulePath() at end of this file. 

:copyright: Copyright 2006-2009 by Oliver Schoenborn, all rights reserved.
:license: BSD, see LICENSE.txt for details.

'''


def setMsgProtocol(protocol):
    import policies

    policies.msgDataProtocol = protocol

    # add appropriate subdir for protocol-specific implementation
    if protocol == 'kwargs':
        _replaceModulePath0('kwargs')
    else:
        _replaceModulePath0('arg1')


def setMsgDataArgName(stage, listenerArgName, senderArgNameAny=False):
    import policies
    policies.senderKwargNameAny = senderArgNameAny
    policies.msgDataArgName = listenerArgName
    policies.msgProtocolTransStage = stage
    #print `policies.msgProtocolTransStage`, `policies.msgDataProtocol`, \
    #      `policies.senderKwargNameAny`, `policies.msgDataArgName`
    #print 'override "arg1" protocol arg name:', argName


def _replaceModulePath0(dirname):
    '''Replace the first package-path item (in __path__) with dirname.
    The dirname will be prepended with the package's path, assumed to
    be the last item in __path__.'''
    corepath = __path__
    assert len(corepath) > 1
    initpyLoc = corepath[-1]
    import os
    corepath[0] = os.path.join(initpyLoc, dirname)


def _prependModulePath(extra):
    '''Insert extra at beginning of package's path list. Should only be
    called once, at package load time, to set the folder used for
    implementation specific to the default message protocol.'''
    corepath = __path__
    initpyLoc = corepath[-1]
    import os
    corepath.insert(0, os.path.join(initpyLoc, extra))
    

# default protocol:
_prependModulePath('kwargs')
