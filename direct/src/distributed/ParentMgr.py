"""ParentMgr module: contains the ParentMgr class"""

from direct.directnotify import DirectNotifyGlobal
from direct.showbase.PythonUtil import isDefaultValue


class ParentMgr:
    # This is now used on the AI as well.
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
        # a node that has not yet registered as a parent
        self.pendingParentToken2children = {}
        # Multiple reparent requests may come in for a given child
        # before that child can successfully be reparented. We need to
        # make sure that each child is only scheduled to be parented to
        # a single parent, at most.
        # For efficient removal of pending children, we keep a dict
        # of pending children to the token of the parent they're waiting for
        self.pendingChild2parentToken = {}

    def destroy(self):
        del self.token2nodepath
        del self.pendingParentToken2children
        del self.pendingChild2parentToken

    def privRemoveReparentRequest(self, child):
        """ this internal function removes any currently-pending reparent
        request for the given child nodepath """
        if child in self.pendingChild2parentToken:
            self.notify.debug("cancelling pending reparent of %s to '%s'" %
                              (repr(child),
                               self.pendingChild2parentToken[child]))
            parentToken = self.pendingChild2parentToken[child]
            del self.pendingChild2parentToken[child]
            self.pendingParentToken2children[parentToken].remove(child)

    def requestReparent(self, child, parentToken):
        if parentToken in self.token2nodepath:
            # this parent has registered
            # this child may already be waiting on a different parent;
            # make sure they aren't any more
            self.privRemoveReparentRequest(child)
            self.notify.debug("performing wrtReparent of %s to '%s'" %
                              (repr(child), parentToken))
            child.wrtReparentTo(self.token2nodepath[parentToken])
        else:
            if isDefaultValue(parentToken):
                self.notify.error('child %s requested reparent to default-value token: %s' % (repr(child), parentToken))
            self.notify.debug(
                "child %s requested reparent to parent '%s' that is not (yet) registered" %
                (repr(child), parentToken))
            # cancel any pending reparent on behalf of this child
            self.privRemoveReparentRequest(child)
            # make note of this pending parent request
            self.pendingChild2parentToken[child] = parentToken
            self.pendingParentToken2children.setdefault(parentToken, [])
            self.pendingParentToken2children[parentToken].append(child)
            # there is no longer any valid place for the child in the
            # scenegraph; put it under hidden
            child.reparentTo(hidden)

    def registerParent(self, token, parent):
        if token in self.token2nodepath:
            self.notify.error(
                "registerParent: token '%s' already registered, referencing %s" %
                (token, repr(self.token2nodepath[token])))

        if isDefaultValue(token):
            self.notify.error('parent token (for %s) cannot be a default value (%s)' % (repr(parent), token))

        if type(token) is int:
            if token > 0xFFFFFFFF:
                self.notify.error('parent token %s (for %s) is out of uint32 range' % (token, repr(parent)))

        self.notify.debug("registering %s as '%s'" % (repr(parent), token))
        self.token2nodepath[token] = parent

        # if we have any pending children, add them
        if token in self.pendingParentToken2children:
            children = self.pendingParentToken2children[token]
            del self.pendingParentToken2children[token]
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
                # (The situation in the factory is a little different;
                # the distributed toons of your companions are never
                # disabled, since the toons are in the factory's uberzone.
                # They send out requests to be parented to nodes that
                # may be distributed objects, which may not be generated
                # on your client)
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
                # remove this child from the child->parent table
                assert self.pendingChild2parentToken[child] == token
                del self.pendingChild2parentToken[child]

    def unregisterParent(self, token):
        if token not in self.token2nodepath:
            self.notify.warning("unregisterParent: unknown parent token '%s'" %
                                token)
            return
        self.notify.debug("unregistering parent '%s'" % (token))
        del self.token2nodepath[token]
