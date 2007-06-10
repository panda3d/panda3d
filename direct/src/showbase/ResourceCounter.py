
class ResourceCounter(object):
    """
    This class is an attempt to combine the RAIA idiom with reference
    counting semantics in order to model shared resources. RAIA stands
    for "Resource Allocation Is Acquisition" (see 'Effective C++' for a
    more in-depth explanation)
    
    When a resource is needed, create an appropriate ResourceCounter
    object.  If the resource is already available (meaning another
    ResourceCounter object of the same type already exists), no action
    is taken.  Otherwise, acquire() is invoked, and the resource is
    allocated. The resource will remain valid until all matching
    ResourceCounter objects have been deleted.  When no objects of
    a particular ResourceCounter type exist, the release() function for
    that type is invoked and the managed resource is cleaned up.

    Usage:
        Define a subclass of ResourceCounter that defines the
        classmethods acquire() and release().  In these two
        functions, define your resource allocation and cleanup code.

    IMPORTANT:
        If you define your own __init__ and __del__ methods, you
        MUST be sure to call down to the ones defined in ResourceCounter.
    
    Notes:
        Until we figure out a way to wrangle a bit more functionality out
        of Python, you MUST NOT define acquire() and release() again in
        any subclasses of your ResourceCounter subclass in an attempt
        to manage another resource.  In debug mode, this will raise a
        runtime assertion. If you have more than one resource, you should
        subclass ResourceCounter again.  See the example code at the
        bottom of this file to see how to manage more than one resource
        with a single instance of an object (Useful for dependent resources).
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
        cls.RESOURCE_COUNTER -= 1
        if cls.RESOURCE_COUNTER < 1:
            cls.release()

    @classmethod
    def acquire(cls):
        cls.RESOURCE_COUNTER -= 1
        assert cls.__mro__[1] == ResourceCounter, \
               (lambda: \
                'acquire() should only be defined in ResourceCounter\'s immediate subclass: %s' \
                 % cls.__mro__[list(cls.__mro__).index(ResourceCounter) - 1].__name__)()

        cls.RESOURCE_COUNTER += 1

    @classmethod
    def release(cls, *args, **kwargs):
        pass
    
    def __init__(self):
        self.incrementCounter()

    def __del__(self):
        self.decrementCounter()

        
if __debug__ and __name__ == '__main__':
    class MouseResource(ResourceCounter):
        """
        A simple class to demonstrate the acquisition of a resource.
        """
        @classmethod
        def acquire(cls):
            super(MouseResource, cls).acquire()
            print '-- Acquire Mouse'
        
        @classmethod
        def release(cls):
            print '-- Release Mouse'
            super(MouseResource, cls).release()

    
        def __init__(self):
            super(MouseResource, self).__init__()            

        def __del__(self):
            super(MouseResource, self).__del__()

    class CursorResource(ResourceCounter):
        """
        A class to demonstrate how to implement a dependent
        resource.  Notice how this class also inherits from
        ResourceCounter.  Instead of subclassing MouseCounter,
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
            self.__mouseResource = MouseResource()
            super(CursorResource, self).__init__()            
        
        def __del__(self):
            super(CursorResource, self).__del__()
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
    print 'Free up Mouse'
    del m
    print 'Free up Cursor'
    del c

    # example of an invalid subclass
    try:
        print '\nAllocate Invalid'
        i = InvalidResource()
        print 'Free up Invalid'
    except AssertionError,e:
        print e

    def demoFunc():
        print '\nAllocate Cursor within function'
        c = CursorResource()

        print 'Cursor will be freed on function exit'
        
    demoFunc()


