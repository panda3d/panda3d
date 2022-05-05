package org.jnius;

import java.lang.reflect.InvocationHandler;
import java.lang.reflect.Method;

/**
 * Special support for pyjnius.
 */
public class NativeInvocationHandler implements InvocationHandler {
    private long _ptr;

    public NativeInvocationHandler(long ptr) {
        _ptr = ptr;
    }

    public long getPythonObjectPointer() {
        return _ptr;
    }

    public Object invoke(Object proxy, Method method, Object[] args) {
        return invoke0(proxy, method, args);
    }

    native Object invoke0(Object proxy, Method method, Object[] args);
}
