'''
Import this file before the first 'from pubsub import pub' statement
to make pubsub use the legacy v1 API.
This would typically be useful only for wxPython applications that use
a version of wxPython where legacy v1 API is *not* the default:

    from wx.lib.pubsub import setupv1
    from wx.lib.pubsub import pub

:copyright: Copyright 2006-2009 by Oliver Schoenborn, all rights reserved.
:license: BSD, see LICENSE.txt for details.

'''

import pubsubconf
pubsubconf.setVersion(1)


