
class ResourceCounter(object):
    """
    This class is an attempt to combine the RAIA idiom with reference
    counting semantics in order to model shared resources. RAIA stands
    for "Resource Allocation Is Acquisiton" (see 'Effective C++' for a
    more detailed explanation)
    
    When a resource is needed, create an appropriate ResourceCounter
    object.  If the resource is already available (meaning another
    ResourceCounter object of the same type already exists), no action
    is taken.  The resource will remain valid until all matching
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
        to manage another resource.  If you have more than one resource,
        subclass ResourceCounter again.  See the example code at the
        bottom of this file to see how to manage more than one resource with
        a single instance of an object (Useful for dependent resources).
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

        if cls.RESOURCE_COUNTER == 0:
            cls.release()

    @classmethod
    def acquire(cls):
        pass

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
            ResourceCounter.acquire()
            print '-- Acquiring Mouse'
        
        @classmethod
        def release(cls):
            ResourceCounter.release()
            print '-- Releasing Mouse'
    
        def __init__(self):
            ResourceCounter.__init__(self)
        
        def __del__(self):
            ResourceCounter.__del__(self)

    class CursorResource(ResourceCounter):
        """
        A class to demonstrate how to implement a dependent
        resource.  Notice how this class also inherits from
        ResourceCounter.  Instead of subclassing MouseCounter,
        we will just acquire it in our __init__() and release
        it in our 
        """
        @classmethod
        def acquire(cls):
            print '-- Acquiring Cursor'
            ResourceCounter.acquire()
            
        @classmethod
        def release(cls):
            print '-- Releasing Cursor'            
            ResourceCounter.release()

        def __init__(self):
            self.__mouseResource = MouseResource()
            ResourceCounter.__init__(self)
        
        def __del__(self):
            ResourceCounter.__del__(self)
            del self.__mouseResource

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
        

    def demoFunc():
        print '\nAllocate Cursor within function'
        c = CursorResource()
        
        print 'Cursor will be freed on function exit'
        

    demoFunc()
