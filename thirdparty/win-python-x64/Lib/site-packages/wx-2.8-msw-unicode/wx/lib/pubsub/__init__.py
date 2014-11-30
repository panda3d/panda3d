'''
Publish-subscribe package.

This package provides the following modules:

- pub: "entry point" module for core pubsub functionality. It provides 
  functions for sending messages and subscribing listeners and various
  others.
- utils: subpackage of utility functions and classes for debugging
  messages, handling exceptions raised in listeners, and more.
- setupv1: (deprecated) module to force pubsub to use the old,
  "version 1" (aka v1) API (should only be useful to wxPython users
  for legacy code).
- setuparg1: module to setup pubsub to use "arg1" messaging protocol
- setupkwargs: module to setup pubsub to use "kwargs" messaging protocol

For instance::

    from pubsub import pub
    pub.sendMessage('topic', data1=123)

:copyright: Copyright 2006-2009 by Oliver Schoenborn, all rights reserved.
:license: BSD, see LICENSE.txt for details.

Last known commit:
- $Date: 2010-02-13 08:57:21 -0500 (Sat, 13 Feb 2010) $
- $Revision: 249 $

'''


__all__ = [
    'pub', 'utils',
    'printImported', 'setupkwargs', 'setuparg1', 'setupv1',
    ]


# set our module search path in globalsettings so setup*.py modules 
# can find and modify
import pubsubconf
pubsubconf.setPubsubInfo(__path__, globals())
del pubsubconf # prevent circular ref


def printImported():
    '''Output a list of pubsub modules imported so far'''
    import sys
    ll = [mod for mod in sys.modules.keys() if mod.find('pubsub') >= 0]
    ll.sort()
    print '\n'.join(ll)


def _tryAutoSetupV1():
    '''This function is called automatically when the pubsub module is 
    imported. It determines if the legacy "version 1" API of pubsub should
    be used automatically, by looking for a module called 'autosetuppubsubv1'
    on the module's search path. If this module is found, setupv1 is imported,
    so your application will get v1 API just by doing "from pubsub import ...". 
    If that module is not found then nothing happens and the function
    returns; your application will get the "default" API unless you 
    explicitly choose a different one. Note that autosetuppubsubv1 is never
    actually imported, just searched. '''
    try: 
        # if autosetupv1 is importable, then it means we should
        # automatically setup for version 1 API
        import imp
        imp.find_module('autosetuppubsubv1', __path__)
        
    except ImportError:
        pass
        
    else:
        import setupv1
        assert pub is not None
        assert Publisher is pub.Publisher


_tryAutoSetupV1()
