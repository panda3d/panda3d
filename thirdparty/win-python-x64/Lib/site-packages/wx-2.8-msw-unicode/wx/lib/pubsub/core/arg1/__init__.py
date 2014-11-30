'''
This is not really a package init file, it is only here to simplify the
packaging and installation of pubsub.core's protocol-specific subfolders
by setuptools.  The python modules in this folder are automatically made
part of pubsub.core via pubsub.core's __path__. Hence, this should not
be imported directly, it is part of pubsub.core when the messaging
protocol is "arg1" (and not usable otherwise).

:copyright: Copyright 2006-2009 by Oliver Schoenborn, all rights reserved.
:license: BSD, see LICENSE.txt for details.

'''


msg = 'Should not import this directly, used by pubsub.core if applicable'
raise RuntimeError(msg)