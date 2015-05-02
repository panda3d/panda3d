// Filename: PandaActivity.java
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

import android.app.NativeActivity;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import org.panda3d.android.NativeIStream;

////////////////////////////////////////////////////////////////////
//       Class : PandaActivity
// Description : The entry point for a Panda-based activity.  Loads
//               the Panda libraries and also provides some utility
//               functions.
////////////////////////////////////////////////////////////////////
public class PandaActivity extends NativeActivity {
    protected static BitmapFactory.Options readBitmapSize(long istreamPtr) {
        BitmapFactory.Options options = new BitmapFactory.Options();
        options.inJustDecodeBounds = true;
        options.inScaled = false;
        NativeIStream stream = new NativeIStream(istreamPtr);
        BitmapFactory.decodeStream(stream, null, options);
        return options;
    }

    protected static Bitmap readBitmap(long istreamPtr, int sampleSize) {
        BitmapFactory.Options options = new BitmapFactory.Options();
        //options.inPreferredConfig = Bitmap.Config.RGBA_8888;
        options.inScaled = false;
        options.inSampleSize = sampleSize;
        NativeIStream stream = new NativeIStream(istreamPtr);
        return BitmapFactory.decodeStream(stream, null, options);
    }

    static {
        System.loadLibrary("gnustl_shared");
        System.loadLibrary("p3dtool");
        System.loadLibrary("p3dtoolconfig");
        System.loadLibrary("pandaexpress");
        System.loadLibrary("panda");
        System.loadLibrary("p3android");
        System.loadLibrary("p3framework");
        System.loadLibrary("pandaegg");
        System.loadLibrary("pandagles");
    }
}
