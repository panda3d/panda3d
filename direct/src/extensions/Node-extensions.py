
    """
    Node-extensions module: contains methods to extend functionality
    of the Node class
    """

    def isHidden(self):
        """Determine if a node is hidden.  Just pick the first parent,
        since this is an ambiguous question for instanced nodes"""
        import RenderRelation
        import PruneTransition
        rrClass = RenderRelation.RenderRelation.getClassType()
        if self.getNumParents(rrClass) > 0:
            arc = self.getParent(rrClass, 0)
            if arc.hasTransition(PruneTransition.PruneTransition.getClassType()):
                return 1
            else:
                return arc.getParent().isHidden()
        else:
            return 0
