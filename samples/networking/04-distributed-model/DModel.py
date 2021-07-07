from direct.distributed.DistributedNode import DistributedNode

class DModel(DistributedNode):
    def __init__(self, cr):
        DistributedNode.__init__(self, cr)

        # Load up the visible representation of this avatar.
        self.model = loader.loadModel('smiley.egg')
        self.model.reparentTo(self)

    def announceGenerate(self):
        """ This method is called after generate(), after all of the
        required fields have been filled in.  At the time of this call,
        the distributed object is ready for use. """

        DistributedNode.announceGenerate(self)

        # Now that the object has been fully manifested, we can parent
        # it into the scene.
        self.reparentTo(render)

    def disable(self):
        """ This method is called when the object is removed from the
        scene, for instance because it left the zone.  It is balanced
        against generate(): for each generate(), there will be a
        corresponding disable().  Everything that was done in
        generate() or announceGenerate() should be undone in disable().

        After a disable(), the object might be cached in memory in case
        it will eventually reappear.  The DistributedObject should be
        prepared to receive another generate() for an object that has
        already received disable().

        Note that the above is only strictly true for *cacheable*
        objects.  Most objects are, by default, non-cacheable; you
        have to call obj.setCacheable(True) (usually in the
        constructor) to make it cacheable.  Until you do this, your
        non-cacheable object will always receive a delete() whenever
        it receives a disable(), and it will never be stored in a
        cache.
        """

        # Take it out of the scene graph.
        self.detachNode()

        DistributedNode.disable(self)

    def delete(self):
        """ This method is called after disable() when the object is to
        be completely removed, for instance because the other user
        logged off.  We will not expect to see this object again; it
        will not be cached.  This is stronger than disable(), and the
        object may remove any structures it needs to in order to allow
        it to be completely deleted from memory.  This balances against
        __init__(): every DistributedObject that is created will
        eventually get delete() called for it exactly once. """

        # Clean out self.model, so we don't have a circular reference.
        self.model = None

        DistributedNode.delete(self)
