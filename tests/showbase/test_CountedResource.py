from direct.showbase.CountedResource import CountedResource


def test_CountedResource():
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
            print('-- Acquire Mouse')

        @classmethod
        def release(cls):
            # First, release the resource this class is
            # managing.
            print('-- Release Mouse')

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
            print('-- Acquire Cursor')

        @classmethod
        def release(cls):
            print('-- Release Cursor')

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
            print('-- Acquire Invalid')

        @classmethod
        def release(cls):
            print('-- Release Invalid')
            super(InvalidResource, cls).release()

    print('\nAllocate Mouse')
    m = MouseResource()
    print('Free up Mouse')
    del m

    print('\nAllocate Cursor')
    c = CursorResource()
    print('Free up Cursor')
    del c

    print('\nAllocate Mouse then Cursor')
    m = MouseResource()
    c = CursorResource()
    print('Free up Cursor')
    del c
    print('Free up Mouse')
    del m

    print('\nAllocate Mouse then Cursor')
    m = MouseResource()
    c = CursorResource()
    print('Free up Mouse')
    del m
    print('Free up Cursor')
    del c

    print('\nAllocate Cursor then Mouse')
    c = CursorResource()
    m = MouseResource()
    print('Free up Mouse')
    del m
    print('Free up Cursor')
    del c

    print('\nAllocate Cursor then Mouse')
    c = CursorResource()
    m = MouseResource()
    print('Free up Cursor')
    del c

    # example of an invalid subclass
    try:
        print('\nAllocate Invalid')
        i = InvalidResource()
        print('Free up Invalid')
    except AssertionError as e:
        print(e)
    print('')

    print('Free up Mouse')
    del m

    def demoFunc():
        print('\nAllocate Cursor within function')
        c = CursorResource()

        print('Cursor will be freed on function exit')

    demoFunc()
