"""ParentMgr module: contains the ParentMgr class"""

from ShowBaseGlobal import *
from ToontownGlobals import *
import DirectNotifyGlobal

class ParentMgr:
    """ParentMgr holds a table of nodes that avatars may be parented to
    in a distributed manner. All clients within a particular zone maintain
    identical tables of these nodes, and the nodes are referenced by 'tokens'
    which the clients can pass to each other to communicate distributed
    reparenting information.

    The functionality of ParentMgr used to be implemented with a simple
    token->node dictionary. As distributed 'parent' objects were manifested,
    they would add themselves to the dictionary. Problems occured when
    distributed avatars were manifested before the objects to which they
    were parented to.

    Since the order of object manifestation depends on the order of the
    classes in the DC file, we could maintain an ordering of DC definitions
    that ensures that the necessary objects are manifested before avatars.
    However, it's easy enough to keep a list of pending reparents and thus
    support the general case without requiring any strict ordering in the DC.
    """

    notify = DirectNotifyGlobal.directNotify.newCategory('ParentMgr')
    
    def __init__(self):
        self.token2nodepath = {}
        # these are nodepaths that have requested to be parented to
        # a node that has not yet been registered
        self.pendingChildren = {}
        # for efficient removal of pending children, we keep a dict
        # of pending children to their pending parent
        self.pendingChild2parentToken = {}

    def destroy(self):
        del self.token2nodepath
        del self.pendingChildren
        del self.pendingChild2parentToken

    def privRemoveReparentRequest(self, child):
        """ this internal function removes any currently-pending reparent
        request for the child nodepath """
        if child in self.pendingChild2parentToken:
            self.notify.debug("cancelling pending reparent of %s to '%s'" %
                              (repr(child),
                               self.pendingChild2parentToken[child]))
            parentToken = self.pendingChild2parentToken[child]
            del self.pendingChild2parentToken[child]
            self.pendingChildren[parentToken].remove(child)

    def requestReparent(self, child, parentToken):
        if self.token2nodepath.has_key(parentToken):
            # this parent has registered
            # this child may already be waiting on a different parent;
            # make sure they aren't any more
            self.privRemoveReparentRequest(child)
            self.notify.debug("performing wrtReparent of %s to '%s'" %
                              (repr(child), parentToken))
            child.wrtReparentTo(self.token2nodepath[parentToken])
        else:
            self.notify.warning(
                "child %s requested reparent to '%s', not in list" %
                (repr(child), parentToken))
            if not self.pendingChildren.has_key(parentToken):
                self.pendingChildren[parentToken] = []
            # cancel any pending reparent on behalf of this child
            self.privRemoveReparentRequest(child)
            # make note of this pending parent request
            self.pendingChild2parentToken[child] = parentToken
            self.pendingChildren[parentToken].append(child)
            # there is no longer any valid place for the child in the
            # scenegraph; put it under hidden
            child.reparentTo(hidden)
            
    def registerParent(self, token, parent):
        if self.token2nodepath.has_key(token):
            self.notify.error(
                "token '%s' already in the table, referencing %s" %
                (token, repr(self.token2nodepath[token])))

        self.notify.debug("registering %s as '%s'" % (repr(parent), token))
        self.token2nodepath[token] = parent

        # if we have any pending children, add them
        if self.pendingChildren.has_key(token):
            children = self.pendingChildren[token]
            for child in children:
                # NOTE: We do a plain-old reparentTo here (non-wrt)
                # under the assumption that the node has been
                # positioned as if it is already under the new parent.
                #
                # The only case that I can think of where the parent
                # node would not have been registered at the time of
                # the reparent request is when we're entering a new
                # zone and manifesting remote toons along with
                # other distributed objects, and a remote toon is
                # requesting to be parented to geometry owned by a
                # distributed object that has not yet been manifested.
                #
                # It is therefore important for that remote toon to
                # have his position set as a required field, relative
                # to the parent node, after the reparent request.
                # If the node has already been registered, the toon will
                # be in the correct position. Otherwise, the toon will
                # have the correct position but the wrong parent node,
                # until this code runs and corrects the toon's parent
                # node. Since we don't start rendering until all objects
                # in a new zone have been generated, all of that action
                # will happen in a single frame, and the net result will
                # be that the toon will be in the right place when
                # rendering starts.
                self.notify.debug("performing reparent of %s to '%s'" %
                                  (repr(child), token))
                child.reparentTo(self.token2nodepath[token])
            del self.pendingChildren[token]

    def unregisterParent(self, token):
        if not self.token2nodepath.has_key(token):
            self.notify.warning("unknown parent token '%s'" % token)
            return
        self.notify.debug("unregistering parent '%s'" % (token))
        del self.token2nodepath[token]
