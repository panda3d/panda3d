
class CountedResource(object):
    """
    This class is an attempt to combine the RAIA idiom with reference
    counting semantics in order to model shared resources. RAIA stands
    for "Resource Allocation Is Acquisition" (see 'Effective C++' for a
    more in-depth explanation)
    
    When a resource is needed, create an appropriate CountedResource
    object.  If the resource is already available (meaning another
    CountedResource object of the same type already exists), no action
    is taken.  Otherwise, acquire() is invoked, and the resource is
    allocated. The resource will remain valid until all matching
    CountedResource objects have been deleted.  When no objects of
    a particular CountedResource type exist, the release() function for
    that type is invoked and the managed resource is cleaned up.

    Usage:
        Define a subclass of CountedResource that defines the
        @classmethods acquire() and release().  In these two
        functions, define your resource allocation and cleanup code.

    IMPORTANT:
        If you define your own __init__ and __del__ methods, you
        MUST be sure to call down to the ones defined in
        CountedResource.
    
    Notes:
        Until we figure out a way to wrangle a bit more functionality
        out of Python, you MUST NOT inherit from any class that has
        CountedResource as its base class. In debug mode, this will
        raise a runtime assertion during the invalid class's call to
        __init__(). If you have more than one resource that you want to
        manage/access with a single object, you should subclass
        CountedResource again. See the example code at the bottom of
        this file to see how to accomplish this (This is useful for
        dependent resources).
    """
    
    @classmethod
    def incrementCounter(cls):
        try:
            cls.RESOURCE_COUNTER += 1
        except AttributeError:
            cls.RESOURCE_COUNTER = 1
            
        if cls.RESOURCE_COUNTER == 1:
            cls.acquire()

    @classmethod
    def decrementCounter(cls):
        try:
            cls.RESOURCE_COUNTER_INIT_FAILED
            del cls.RESOURCE_COUNTER_INIT_FAILED
        except AttributeError:
            cls.RESOURCE_COUNTER -= 1
            if cls.RESOURCE_COUNTER < 1:
                cls.release()

    @classmethod
    def getCount(cls):
        return cls.RESOURCE_COUNTER
    
    @classmethod
    def acquire(cls):
        pass
    
    @classmethod
    def release(cls):
        pass
    
    def __init__(self):
        cls = type(self)
        cls.RESOURCE_COUNTER_INIT_FAILED = True
        assert cls.mro()[1] == CountedResource, \
               (lambda: \
                '%s cannot be subclassed.' \
                 % cls.mro()[list(cls.mro()).index(CountedResource) - 1].__name__)()
        del cls.RESOURCE_COUNTER_INIT_FAILED
        self.incrementCounter()

    def __del__(self):
        self.decrementCounter()

        
if __debug__ and __name__ == '__main__':
    class MouseResource(CountedResource):
        """
        A simple class to demonstrate the acquisition of a resource.
        """
        @classmethod
        def acquire(cls):
            # The call to the super-class's acquire() is
            # not necessary at the moment, but may be in
            # the future, so do it now for good measure.
            super(MouseResource, cls).acquire()

            # Now acquire the resource this class is
            # managing.
            print '-- Acquire Mouse'
        
        @classmethod
        def release(cls):
            # First, release the resource this class is
            # managing.
            print '-- Release Mouse'

            # The call to the super-class's release() is
            # not necessary at the moment, but may be in
            # the future, so do it now for good measure.
            super(MouseResource, cls).release()

    
        def __init__(self):
            super(MouseResource, self).__init__()

        def __del__(self):
            super(MouseResource, self).__del__()

    class CursorResource(CountedResource):
        """
        A class to demonstrate how to implement a dependent
        resource.  Notice how this class also inherits from
        CountedResource.  Instead of subclassing MouseCounter,
        we will just acquire it in our __init__() and release
        it in our __del__().
        """
        @classmethod
        def acquire(cls):
            super(CursorResource, cls).acquire()
            print '-- Acquire Cursor'
            
        @classmethod
        def release(cls):
            print '-- Release Cursor'

            super(CursorResource, cls).release()

        def __init__(self):
            # The required resource references should
            # be stored on 'self' since we want to
            # release it when the object is deleted.
            self.__mouseResource = MouseResource()

            # Call the super-classes __init__()
            # after all required resources are
            # referenced.
            super(CursorResource, self).__init__()            
        
        def __del__(self):
            # Free up the most dependent resource
            # first, the one this class is managing.
            super(CursorResource, self).__del__()
            
            # Now unlink any required resources.
            del self.__mouseResource

    class InvalidResource(MouseResource):
        @classmethod
        def acquire(cls):
            super(InvalidResource, cls).acquire()
            print '-- Acquire Invalid'
            
        @classmethod
        def release(cls):
            print '-- Release Invalid'
            super(InvalidResource, cls).release()
            
    print '\nAllocate Mouse'
    m = MouseResource()
    print 'Free up Mouse'
    del m
    
    print '\nAllocate Cursor'
    c = CursorResource()
    print 'Free up Cursor'
    del c

    print '\nAllocate Mouse then Cursor'
    m = MouseResource()
    c = CursorResource()
    print 'Free up Cursor'
    del c
    print 'Free up Mouse'
    del m
    
    print '\nAllocate Mouse then Cursor'
    m = MouseResource()
    c = CursorResource()
    print 'Free up Mouse'
    del m
    print 'Free up Cursor'
    del c
    
    print '\nAllocate Cursor then Mouse'
    c = CursorResource()
    m = MouseResource()
    print 'Free up Mouse'
    del m
    print 'Free up Cursor'
    del c
    
    print '\nAllocate Cursor then Mouse'
    c = CursorResource()
    m = MouseResource()
    print 'Free up Cursor'
    del c
    
    # example of an invalid subclass
    try:
        print '\nAllocate Invalid'
        i = InvalidResource()
        print 'Free up Invalid'
    except AssertionError,e:
        print e
    print

    print 'Free up Mouse'
    del m

    def demoFunc():
        print '\nAllocate Cursor within function'
        c = CursorResource()

        print 'Cursor will be freed on function exit'
        
    demoFunc()



