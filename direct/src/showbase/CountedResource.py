
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
