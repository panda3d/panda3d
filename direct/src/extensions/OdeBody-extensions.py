
def getConvertedJoint(self, index):
    """
    Return a downcast joint on this body.
    """
    return self.getJoint(index).convert()
