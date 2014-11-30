'''
The root topic of all topics is different based on messaging protocol.

:copyright: Copyright 2006-2009 by Oliver Schoenborn, all rights reserved.
:license: BSD, see LICENSE.txt for details.

'''

import policies


def getRootTopicSpec():
    '''If using "arg1" messaging protocol, then root topic has one arg;
    if policies.msgDataArgName is something, then use it as arg name.'''
    argName = policies.msgDataArgName or 'data'
    argsDocs = {argName : 'data for message sent'}
    reqdArgs = (argName,)
    return argsDocs, reqdArgs

