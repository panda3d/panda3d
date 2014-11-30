"""
Configuration of pubsub API to use either the *arg1* or *kwargs*
messaging protocol in API, or to help transition from the former to the
latter. This should be used only internally by pubsub. 

:copyright: Copyright 2006-2009 by Oliver Schoenborn, all rights reserved.
:license: BSD, see LICENSE.txt for details.
     
"""


pubModuleInfo = None


def setVersion(id):
    pubModuleInfo.setVersion(id)


class _PubModuleInfo:
    '''Used to keep track of which pub module is loaded, based on version
    selected. '''

    DEFAULT_VERSION = 3

    def __init__(self, moduleSearchPaths, moduleDict):
        self.__searchPaths = moduleSearchPaths
        self.__pubsubDict  = moduleDict
        self.__version = self.DEFAULT_VERSION
        # nothing else to do for default version

    def setVersion(self, versionID):
        if versionID == self.__version:
            return

        modulePath = self.__searchPaths

        # cleanup first if necessary:

        if self.__version == self.DEFAULT_VERSION:
            # nothing to cleanup
            pass

        else:
            # remove the now obsolete path element
            assert len(modulePath) > 1
            del modulePath[0]

            if self.__version == 1:
                self.__unsetupV1()

        assert len(modulePath) == 1

        # now add path if necessary:
        if versionID == self.DEFAULT_VERSION:
            # path is '.', ie modulePath[-1]
            pass

        else:
            # just prepend the path:
            ourInitpyPath = modulePath[-1]
            paths = {1: 'pubsub1', 2: 'pubsub2', self.DEFAULT_VERSION: None}
            import os
            path = os.path.join(ourInitpyPath, paths[versionID])
            modulePath.insert(0, path)

        assert versionID != self.__version
        self.__version = versionID

        if self.__version == 1:
            self.__setupForV1()


    def __setupForV1(self):
        '''Add the pub module and Publisher singleton to the given map
        (pubsubInitGlobals), assumed to be pubsub.__init__'s global dict.
        The pub module will provide the legacy "version 1" API for pubsub,
        and Publisher will be the singleton stored in that module.'''
        import pub
        from pub import Publisher

        self.__pubsubDict['Publisher'] = Publisher
        self.__pubsubDict['pub'] = pub


    def __unsetupV1(self):
        '''This should be called by pubsub.setup!* modules in case setupv1 was used.'''
        self.__removePubModule()
        del self.__pubsubDict['pub']
        del self.__pubsubDict['Publisher']


    def __removePubModule(self):
        '''Remove the given module object from the sys.modules map.'''
        pubsub1 = self.__pubsubDict['pub']
        import sys
        for (modName, modObj) in sys.modules.iteritems():
            if modObj is pubsub1:
                del sys.modules[modName]
                break


def setPubsubInfo(moduleSearchPath, moduleDict):
    '''This gets called by pubsub's __init__.py so that setupv1 will be
    able to add the pub and Publisher instances to pubsub module, and so
    that setupkwargs and setuparg1 can clean things up if they are
    imported after setupv1 was used. '''
    global pubModuleInfo
    assert pubModuleInfo is None
    pubModuleInfo = _PubModuleInfo(moduleSearchPath, moduleDict)


def pubModuleLoaded():
    '''This gets called by pub once loaded. Helps avoid circular refs.'''
    #global pubModuleInfo
    #pubModuleInfo = None

    