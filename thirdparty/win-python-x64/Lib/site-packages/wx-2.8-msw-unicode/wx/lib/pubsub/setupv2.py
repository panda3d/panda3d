'''
Import this file before the first 'from pubsub import pub' statement
to make pubsub use the legacy v2 API. This API is no longer supported
or maintained.

:copyright: Copyright 2006-2009 by Oliver Schoenborn, all rights reserved.
:license: BSD, see LICENSE.txt for details.

'''

import pubsubconf
pubsubconf.setVersion(2)


