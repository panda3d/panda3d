'''

:copyright: Copyright 2006-2009 by Oliver Schoenborn, all rights reserved.
:license: BSD, see LICENSE.txt for details.

'''

from topicexc import ListenerSpecInvalid


def verifyArgsDifferent(allArgs, allParentArgs, topicName):
    extra = set(allArgs).intersection(allParentArgs)
    if extra:
        msg = 'Args %%s already used in parent of "%s"' % topicName
        raise ListenerSpecInvalid( msg, tuple(extra) )


def verifySubset(all, sub, topicName, extraMsg=''):
    '''Verify that sub is a subset of all for topicName'''
    notInAll = set(sub).difference(all)
    if notInAll:
        args = ','.join(all)
        msg = 'Params [%s] missing inherited [%%s] for topic "%s"%s' % (args, topicName, extraMsg)
        raise ListenerSpecInvalid(msg, tuple(notInAll) )


