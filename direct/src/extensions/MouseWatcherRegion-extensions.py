
    """
    MouseWatcherRegion-extensions module: contains methods to extend
    functionality of the MouseWatcherRegion class
    """

    def setRelative(self, np, left, right, bottom, top):
        """setRelation(NodePath np, float left, float right,
                       float bottom, float top)

        Sets the region to represnt the indicated rectangle, relative
        to the given NodePath.  It is assumed that np represents some
        node parented within the render2d hierarchy.

        """
        from pandac import Point3
        
        # Get the relative transform to the node.
        mat = np.getMat(render2d)

        # Use this matrix to transform the corners of the region.
        ll = mat.xformPoint(Point3.Point3(left, 0, bottom))
        ur = mat.xformPoint(Point3.Point3(right, 0, top))

        # Set the frame to the transformed coordinates.
        self.setFrame(ll[0], ur[0], ll[2], ur[2])
        
