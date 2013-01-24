// Filename: NativeIStream.java
// Created by:  rdb (22Jan13)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) Carnegie Mellon University.  All rights reserved.
//
// All use of this software is subject to the terms of the revised BSD
// license.  You should have received a copy of this license along
// with this source code in a file named "LICENSE."
//
////////////////////////////////////////////////////////////////////

package org.panda3d.android;

import java.io.InputStream;

////////////////////////////////////////////////////////////////////
//       Class : NativeIStream
// Description : An implementation of InputStream that gets its
//               data from a C++ istream pointer, passed as long.
////////////////////////////////////////////////////////////////////
public class NativeIStream extends InputStream {
    private long streamPtr = 0;

    public NativeIStream(long ptr) {
        streamPtr = ptr;
    }

    @Override
    public int read() {
        return nativeGet(streamPtr);
    }

    @Override
    public int read(byte[] buffer, int offset, int length) {
        return nativeRead(streamPtr, buffer, offset, length);
    }

    @Override
    public long skip(long n) {
        return nativeIgnore(streamPtr, n);
    }

    private static native int nativeGet(long ptr);
    private static native int nativeRead(long ptr, byte[] buffer, int offset, int length);
    private static native long nativeIgnore(long ptr, long offset);
}
