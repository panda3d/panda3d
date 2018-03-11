/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file PandaActivity.java
 * @author rdb
 * @date 2013-01-22
 */

package org.panda3d.android;

import android.app.NativeActivity;
import android.content.Intent;
import android.net.Uri;
import android.widget.Toast;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import org.panda3d.android.NativeIStream;
import org.panda3d.android.NativeOStream;

/**
 * The entry point for a Panda-based activity.  Loads the Panda libraries and
 * also provides some utility functions.
 */
public class PandaActivity extends NativeActivity {
    private static final Bitmap.Config sConfigs[] = {
            null,
            Bitmap.Config.ALPHA_8,
            null,
            Bitmap.Config.RGB_565,
            Bitmap.Config.ARGB_4444,
            Bitmap.Config.ARGB_8888,
            null, //Bitmap.Config.RGBA_F16,
            null, //Bitmap.Config.HARDWARE,
        };
    private static final Bitmap.CompressFormat sFormats[] = {
            Bitmap.CompressFormat.JPEG,
            Bitmap.CompressFormat.PNG,
            Bitmap.CompressFormat.WEBP,
        };

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
        // options.inPreferredConfig = Bitmap.Config.RGBA_8888;
        options.inScaled = false;
        options.inSampleSize = sampleSize;
        NativeIStream stream = new NativeIStream(istreamPtr);
        return BitmapFactory.decodeStream(stream, null, options);
    }

    protected static Bitmap createBitmap(int width, int height, int config, boolean hasAlpha) {
        return Bitmap.createBitmap(width, height, sConfigs[config]);
    }

    protected static boolean compressBitmap(Bitmap bitmap, int format, int quality, long ostreamPtr) {
        NativeOStream stream = new NativeOStream(ostreamPtr);
        return bitmap.compress(sFormats[format], quality, stream);
    }

    protected static String getCurrentThreadName() {
        return Thread.currentThread().getName();
    }

    public String getIntentDataPath() {
        Intent intent = getIntent();
        Uri data = intent.getData();
        if (data == null) {
            return null;
        }
        String path = data.getPath();
        if (path.startsWith("//")) {
          path = path.substring(1);
        }
        return path;
    }

    public String getIntentOutputUri() {
        Intent intent = getIntent();
        return intent.getStringExtra("org.panda3d.OUTPUT_URI");
    }

    public String getCacheDirString() {
        return getCacheDir().toString();
    }

    public void showToast(final String text, final int duration) {
        final PandaActivity activity = this;
        runOnUiThread(new Runnable() {
            public void run() {
                Toast toast = Toast.makeText(activity, text, duration);
                toast.show();
            }
        });
    }

    static {
        //System.loadLibrary("gnustl_shared");
        //System.loadLibrary("p3dtool");
        //System.loadLibrary("p3dtoolconfig");
        //System.loadLibrary("pandaexpress");
        //System.loadLibrary("panda");
        //System.loadLibrary("p3android");
        //System.loadLibrary("p3framework");
        System.loadLibrary("pandaegg");
        System.loadLibrary("pandagles");
    }
}
