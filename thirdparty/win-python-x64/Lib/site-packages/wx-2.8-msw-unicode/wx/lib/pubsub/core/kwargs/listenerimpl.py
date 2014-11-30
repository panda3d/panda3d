'''

:copyright: Copyright 2006-2009 by Oliver Schoenborn, all rights reserved.
:license: BSD, see LICENSE.txt for details.

'''

from listenerbase import ListenerBase, ValidatorBase
from callables import ListenerInadequate


class Listener(ListenerBase):
    def __call__(self, kwargs, actualTopic, allKwargs=None):
        '''Call the listener with **kwargs. Note that it raises RuntimeError 
        if listener is dead. Should always return True (False would require
        the callable_ be dead but self hasn't yet been notified of it...).'''
        if self.acceptsAllKwargs:
            kwargs = allKwargs or kwargs # if allKwargs is None then use kwargs

        if self._autoTopicArgName is not None:
            kwargs = kwargs.copy()
            kwargs[self._autoTopicArgName] = actualTopic

        cb = self._callable()
        if cb is None:
            self._calledWhenDead()
        cb(**kwargs)

        return True


class ListenerValidator(ValidatorBase):
    '''
    Do not accept any required args or *args; accept any **kwarg, 
    and require that the Listener have at least all the kwargs (can 
    have extra) of Topic.
    '''
    
    def _validateArgs(self, listener, paramsInfo):
        # accept **kwargs
        # accept *args
        
        # check if listener missing params (only possible if
        # paramsInfo.acceptsAllKwargs is False)
        allTopicMsgArgs = self._topicArgs | self._topicKwargs
        allParams = set(paramsInfo.allParams)
        if not paramsInfo.acceptsAllKwargs:
            missingParams = allTopicMsgArgs - allParams
            if missingParams:
                msg = 'needs to accept %s more args (%s)' \
                    % (len(missingParams), ''.join(missingParams))
                raise ListenerInadequate(msg, listener, missingParams)
        else:
            # then can accept that some parameters missing from listener 
            # signature
            pass
            
        # check if there are unknown parameters in listener signature:
        extraArgs = allParams - allTopicMsgArgs
        if extraArgs:
            if allTopicMsgArgs:
                msg = 'args (%s) not allowed, should be (%s)' \
                    % (','.join(extraArgs), ','.join(allTopicMsgArgs))
            else:
                msg = 'no args allowed, has (%s)' % ','.join(extraArgs)
            raise ListenerInadequate(msg, listener, extraArgs)

        # we accept listener that has fewer required paams than TMS
        # since all args passed by name (previous showed that spec met
        # for all parameters).

        # now make sure listener doesn't require params that are optional in TMS:
        extraArgs = set( paramsInfo.getRequiredArgs() ) - self._topicArgs
        if extraArgs:
            msg = 'params (%s) missing default values' % (','.join(extraArgs),)
            raise ListenerInadequate(msg, listener, extraArgs)

