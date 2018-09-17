/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file NativeOStream.java
 * @author rdb
 * @date 2018-02-10
 */

package org.panda3d.android;

import java.io.OutputStream;

/**
 * An implementation of OutputStream that puts its data into a C++ ostream
 * pointer, passed as long.
 */
public class NativeOStream extends OutputStream {
    private long streamPtr = 0;

    public NativeOStream(long ptr) {
        streamPtr = ptr;
    }

    @Override
    public void flush() {
        nativeFlush(streamPtr);
    }

    @Override
    public void write(int b) {
        nativePut(streamPtr, b);
    }

    @Override
    public void write(byte[] buffer) {
        nativeWrite(streamPtr, buffer, 0, buffer.length);
    }

    @Override
    public void write(byte[] buffer, int offset, int length) {
        nativeWrite(streamPtr, buffer, offset, length);
    }

    private static native void nativeFlush(long ptr);
    private static native void nativePut(long ptr, int b);
    private static native void nativeWrite(long ptr, byte[] buffer, int offset, int length);
}
