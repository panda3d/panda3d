""" This module defines some simple classes and instances which are
useful when writing code that integrates with JavaScript, especially
code that runs in a browser via the web plugin.

.. deprecated:: 1.10.0
   The browser plug-in is no longer supported.
"""

__all__ = ["UndefinedObject", "Undefined", "ConcreteStruct", "BrowserObject", "MethodWrapper"]

class UndefinedObject:
    """ This is a special object that is returned by the browser to
    represent an "undefined" or "void" value, typically the value for
    an uninitialized variable or undefined property.  It has no
    attributes, similar to None, but it is a slightly different
    concept in JavaScript. """

    def __bool__(self):
        return False

    __nonzero__ = __bool__ # Python 2

    def __str__(self):
        return "Undefined"

# In fact, we normally always return this precise instance of the
# UndefinedObject.
Undefined = UndefinedObject()

class ConcreteStruct:
    """ Python objects that inherit from this class are passed to
    JavaScript as a concrete struct: a mapping from string -> value,
    with no methods, passed by value.  This can be more optimal than
    traditional Python objects which are passed by reference,
    especially for small objects which might be repeatedly referenced
    on the JavaScript side. """

    def __init__(self):
        pass

    def getConcreteProperties(self):
        """ Returns a list of 2-tuples of the (key, value) pairs that
        are to be passed to the concrete instance.  By default, this
        returns all properties of the object.  You can override this
        to restrict the set of properties that are uploaded. """

        return list(self.__dict__.items())

class BrowserObject:
    """ This class provides the Python wrapper around some object that
    actually exists in the plugin host's namespace, e.g. a JavaScript
    or DOM object. """

    def __init__(self, runner, objectId):
        self.__dict__['_BrowserObject__runner'] = runner
        self.__dict__['_BrowserObject__objectId'] = objectId

        # This element is filled in by __getattr__; it connects
        # the object to its parent.
        self.__dict__['_BrowserObject__childObject'] = (None, None)

        # This is a cache of method names to MethodWrapper objects in
        # the parent object.
        self.__dict__['_BrowserObject__methods'] = {}

    def __del__(self):
        # When the BrowserObject destructs, tell the parent process it
        # doesn't need to keep around its corresponding P3D_object any
        # more.
        self.__runner.dropObject(self.__objectId)

    def __cacheMethod(self, methodName):
        """ Stores a pointer to the named method on this object, so
        that the next time __getattr__ is called, it can retrieve the
        method wrapper without having to query the browser.  This
        cache assumes that callable methods don't generally come and
        go on and object.

        The return value is the MethodWrapper object. """

        method = self.__methods.get(methodName, None)
        if method is None:
            method = MethodWrapper(self.__runner, self, methodName)
            self.__methods[methodName] = method
        return method

    def __str__(self):
        return self.toString()

    def __bool__(self):
        return True

    __nonzero__ = __bool__ # Python 2

    def __call__(self, *args, **kw):
        needsResponse = True
        if 'needsResponse' in kw:
            needsResponse = kw['needsResponse']
            del kw['needsResponse']
        if kw:
            raise ArgumentError('Keyword arguments not supported')

        try:
            parentObj, attribName = self.__childObject
            if parentObj:
                # Call it as a method.
                if parentObj is self.__runner.dom and attribName == 'alert':
                    # As a special hack, we don't wait for the return
                    # value from the alert() call, since this is a
                    # blocking call, and waiting for this could cause
                    # problems.
                    needsResponse = False

                if parentObj is self.__runner.dom and attribName == 'eval' and len(args) == 1 and isinstance(args[0], str):
                    # As another special hack, we make dom.eval() a
                    # special case, and map it directly into an eval()
                    # call.  If the string begins with 'void ', we further
                    # assume we're not waiting for a response.
                    if args[0].startswith('void '):
                        needsResponse = False
                    result = self.__runner.scriptRequest('eval', parentObj, value = args[0], needsResponse = needsResponse)
                else:
                    # This is a normal method call.
                    try:
                        result = self.__runner.scriptRequest('call', parentObj, propertyName = attribName, value = args, needsResponse = needsResponse)
                    except EnvironmentError:
                        # Problem on the call.  Maybe no such method?
                        raise AttributeError

                # Hey, the method call appears to have succeeded.
                # Cache the method object on the parent so we won't
                # have to look up the method wrapper again next time.
                parentObj.__cacheMethod(attribName)

            else:
                # Call it as a plain function.
                result = self.__runner.scriptRequest('call', self, value = args, needsResponse = needsResponse)
        except EnvironmentError:
            # Some odd problem on the call.
            raise TypeError

        return result

    def __getattr__(self, name):
        """ Remaps attempts to query an attribute, as in obj.attr,
        into the appropriate calls to query the actual browser object
        under the hood.  """

        # First check to see if there's a cached method wrapper from a
        # previous call.
        method = self.__methods.get(name, None)
        if method:
            return method

        # No cache.  Go query the browser for the desired value.
        try:
            value = self.__runner.scriptRequest('get_property', self,
                                                propertyName = name)
        except EnvironmentError:
            # Failed to retrieve the attribute.  But maybe there's a
            # method instead?
            if self.__runner.scriptRequest('has_method', self, propertyName = name):
                # Yes, so create a method wrapper for it.
                return self.__cacheMethod(name)

            raise AttributeError(name)

        if isinstance(value, BrowserObject):
            # Fill in the parent object association, so __call__ can
            # properly call a method.  (Javascript needs to know the
            # method container at the time of the call, and doesn't
            # store it on the function object.)
            value.__dict__['_BrowserObject__childObject'] = (self, name)

        return value

    def __setattr__(self, name, value):
        if name in self.__dict__:
            self.__dict__[name] = value
            return

        result = self.__runner.scriptRequest('set_property', self,
                                             propertyName = name,
                                             value = value)
        if not result:
            raise AttributeError(name)

    def __delattr__(self, name):
        if name in self.__dict__:
            del self.__dict__[name]
            return

        result = self.__runner.scriptRequest('del_property', self,
                                             propertyName = name)
        if not result:
            raise AttributeError(name)

    def __getitem__(self, key):
        """ Remaps attempts to query an attribute, as in obj['attr'],
        into the appropriate calls to query the actual browser object
        under the hood.  Following the JavaScript convention, we treat
        obj['attr'] almost the same as obj.attr. """

        try:
            value = self.__runner.scriptRequest('get_property', self,
                                                propertyName = str(key))
        except EnvironmentError:
            # Failed to retrieve the property.  We return IndexError
            # for numeric keys so we can properly support Python's
            # iterators, but we return KeyError for string keys to
            # emulate mapping objects.
            if isinstance(key, str):
                raise KeyError(key)
            else:
                raise IndexError(key)

        return value

    def __setitem__(self, key, value):
        result = self.__runner.scriptRequest('set_property', self,
                                             propertyName = str(key),
                                             value = value)
        if not result:
            if isinstance(key, str):
                raise KeyError(key)
            else:
                raise IndexError(key)

    def __delitem__(self, key):
        result = self.__runner.scriptRequest('del_property', self,
                                             propertyName = str(key))
        if not result:
            if isinstance(key, str):
                raise KeyError(key)
            else:
                raise IndexError(key)

class MethodWrapper:
    """ This is a Python wrapper around a property of a BrowserObject
    that doesn't appear to be a first-class object in the Python
    sense, but is nonetheless a callable method. """

    def __init__(self, runner, parentObj, objectId):
        self.__dict__['_MethodWrapper__runner'] = runner
        self.__dict__['_MethodWrapper__childObject'] = (parentObj, objectId)

    def __str__(self):
        parentObj, attribName = self.__childObject
        return "%s.%s" % (parentObj, attribName)

    def __bool__(self):
        return True

    __nonzero__ = __bool__ # Python 2

    def __call__(self, *args, **kw):
        needsResponse = True
        if 'needsResponse' in kw:
            needsResponse = kw['needsResponse']
            del kw['needsResponse']
        if kw:
            raise ArgumentError('Keyword arguments not supported')

        try:
            parentObj, attribName = self.__childObject
            # Call it as a method.
            if parentObj is self.__runner.dom and attribName == 'alert':
                # As a special hack, we don't wait for the return
                # value from the alert() call, since this is a
                # blocking call, and waiting for this could cause
                # problems.
                needsResponse = False

            if parentObj is self.__runner.dom and attribName == 'eval' and len(args) == 1 and isinstance(args[0], str):
                # As another special hack, we make dom.eval() a
                # special case, and map it directly into an eval()
                # call.  If the string begins with 'void ', we further
                # assume we're not waiting for a response.
                if args[0].startswith('void '):
                    needsResponse = False
                result = self.__runner.scriptRequest('eval', parentObj, value = args[0], needsResponse = needsResponse)
            else:
                # This is a normal method call.
                try:
                    result = self.__runner.scriptRequest('call', parentObj, propertyName = attribName, value = args, needsResponse = needsResponse)
                except EnvironmentError:
                    # Problem on the call.  Maybe no such method?
                    raise AttributeError

        except EnvironmentError:
            # Some odd problem on the call.
            raise TypeError

        return result

    def __setattr__(self, name, value):
        """ setattr will generally fail on method objects. """
        raise AttributeError(name)

    def __delattr__(self, name):
        """ delattr will generally fail on method objects. """
        raise AttributeError(name)
