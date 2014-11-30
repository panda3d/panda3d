'''
Import this file before the first 'from pubsub import pub' statement
to make pubsub use the *kwargs* messaging protocol::

    from pubsub import setupkwargs
    from pubsub import pub

Note that in a default pubsub installation, this protocol is
the default, such that importing setupkwargs would rarely be
required. But some pubsub installations (such as pubsub in
some versions of wxPython) use the legacy v1 API as the default.
For an application based on such an installation, but requiring
the more advanced *kwargs* protocol, this module can be imported as
described above.

See the setuparg1 module for using the same messaging protocol
as the legacy v1 API, but using the latest (ie non legacy) API.

Note that once :mod:pub has been imported, the messaging protocol
cannot be changed. Also, if migrating an application from 'arg1' to 'kwargs'
style messaging, see :func:transitionFromArg1() in this module and the
:func:enforceArgName() of setuparg1 module.

:copyright: Copyright 2006-2009 by Oliver Schoenborn, all rights reserved.
:license: BSD, see LICENSE.txt for details.

'''

import pubsubconf
pubsubconf.setVersion(3)
import core
core.setMsgProtocol('kwargs')


def transitionFromArg1(commonName):
    '''This will require that all calls to pub.sendMessage() use the
    kwargs protocol, ie named arguments for the message data. This is
    a useful step after setuparg1.enforceArgName(commonName) was used
    and the application debugged. Replace the latter with ::

        setupkwargs.transitionFromArg1(commonName)

    After this stage tested and debugged, this function call
    can be removed, and all reference to the .data attribute of the message
    object received can be removed in all listeners, allowing the
    application to run in the default messaging protocol (kwargs) used by
    pubsub version >= 3.
    '''

    import core
    core.setMsgDataArgName(2, commonName)
