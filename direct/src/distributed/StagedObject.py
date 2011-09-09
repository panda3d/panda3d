
class StagedObject:
    """
    Use this class as a mixin to provide an interface for onStage/offStage objects.

    The idea here is that a DistributedObject could be present and active due to
    simple visibility, but we want to hide or otherwise disable it for some reason.
    """
    
    UNKNOWN = -1
    OFF     = 0
    ON      = 1

    def __init__(self, initState = UNKNOWN):
        """
        Only sets the initial state of this object.  This will not
        call any "handle" functions.
        """
        self.__state = initState
        pass

    def goOnStage(self, *args, **kw):
        """
        If a stage switch is needed, the correct "handle" function
        will be called.  Otherwise, nothing happens.
        """
        # This is the high level function that clients of
        # your class should call to set the on/off stage state.

        if not self.isOnStage():
            self.handleOnStage(*args, **kw)
            pass
        pass
    
    def handleOnStage(self):
        """
        Override this function to provide your on/off stage funcitionality.
        
        Don't forget to call down to this one, though.
        """
        self.__state = StagedObject.ON
        pass
    
    def goOffStage(self, *args, **kw):
        """
        If a stage switch is needed, the correct "handle" function
        will be called.  Otherwise, nothing happens.
        """
        # This is the high level function that clients of
        # your class should call to set the on/off stage state.

        if not self.isOffStage():
            self.handleOffStage(*args, **kw)
            pass
        pass
    
    def handleOffStage(self):
        """
        Override this function to provide your on/off stage funcitionality.
        
        Don't forget to call down to this one, though.
        """
        self.__state = StagedObject.OFF
        pass
    
    def isOnStage(self):
        return self.__state == StagedObject.ON

    def isOffStage(self):
        return self.__state == StagedObject.OFF

