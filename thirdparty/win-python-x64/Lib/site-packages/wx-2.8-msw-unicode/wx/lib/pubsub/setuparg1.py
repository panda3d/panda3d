'''
Import this file before the first 'from pubsub import pub' statement
to make pubsub use the *arg1* messaging protocol::

    from pubsub import setuparg1
    from pubsub import pub

This could be necessary in at least two situations: 

1. with a default pubsub installation, where *kwargs* messaging protocol
   is the default, but an application developer requires the less
   stringent *arg1* messaging protocol.
2. with a pubsub installation that has been configured to provide the 
   legacy v1 API as the default (such as in some versions of wxPython), 
   but an application developer wants to use the latest API, but with 
   the messaging protocol closest to that used in the legacy v1 API.

See the setupkwargs module for a description of the default pubsub
messaging protocol, defined as 'kwargs'.

Note that once :mod:pub has been imported, the messaging protocol
cannot be changed. Also, if migrating an application from 'arg1' to 'kwargs'
style messaging, see :func:enforceArgName().

:copyright: Copyright 2006-2009 by Oliver Schoenborn, all rights reserved.
:license: BSD, see LICENSE.txt for details.

'''

import pubsubconf
pubsubconf.setVersion(3)
import core
core.setMsgProtocol('arg1')


def enforceArgName(commonName):
    '''This will require that all listeners use the same argument
    name (commonName) as first parameter. This could be a ueful
    first step in transition an application that has been using *arg1*
    protocol to the default *kwargs* protocol, see the docs for
    details. '''
    import core
    core.setMsgDataArgName(1, commonName)
