'''

:copyright: Copyright 2006-2009 by Oliver Schoenborn, all rights reserved.
:license: BSD, see LICENSE.txt for details.

'''


class ITopicTreeVisitor:
    '''
    Topic tree traverser. Provides the traverse() method
    which traverses a topic tree and calls self._onTopic() for
    each topic in the tree that satisfies self._accept().
    Additionally it calls self._startChildren() whenever it
    starts traversing the subtopics of a topic, and
    self._endChildren() when it is done with the subtopics.
    Finally, it calls self._doneTraversal() when traversal
    has been completed.

    Derive from ITopicTreeVisitor and override one or more of the
    four self._*() methods described above. Give an instance to
    an instance of pub.TopicTreeTraverser, whose traverse()
    method will cause the tree to be printed.
    '''

    def _accept(self, topicObj):
        '''Override this to filter nodes of topic tree. Must return
        True (accept node) of False (reject node). Note that rejected
        nodes cause traversal to move to next branch (no children
        traversed).'''
        return True

    def _startTraversal(self):
        '''Override this to define what to do when traversal() starts.'''
        pass

    def _onTopic(self, topicObj):
        '''Override this to define what to do for each node.'''
        pass

    def _startChildren(self):
        '''Override this to take special action whenever a
        new level of the topic hierarchy is started (e.g., indent
        some output). '''
        pass

    def _endChildren(self):
        '''Override this to take special action whenever a
        level of the topic hierarchy is completed (e.g., dedent
        some output). '''
        pass

    def _doneTraversal(self):
        '''Override this to take special action when traversal done.'''
        pass

