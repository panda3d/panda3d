/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file PythonActivity.java
 * @author rdb
 * @date 2018-02-04
 */

package org.panda3d.android;

import org.panda3d.android.PandaActivity;

import android.content.Intent;
import java.util.ArrayList;
import java.util.Collections;
import java.util.Iterator;
import java.util.List;

/**
 * Extends PandaActivity with some things that are useful in a Python
 * application.
 */
public class PythonActivity extends PandaActivity {
    // This is required by plyer.
    public static PythonActivity mActivity;

    public PythonActivity() {
        mActivity = this;
    }

    // Helper code to further support plyer.
    public interface ActivityResultListener {
        void onActivityResult(int requestCode, int resultCode, Intent data);
    }

    private List<ActivityResultListener> activityResultListeners = null;

    public void registerActivityResultListener(ActivityResultListener listener) {
        if (this.activityResultListeners == null) {
            this.activityResultListeners = Collections.synchronizedList(new ArrayList<ActivityResultListener>());
        }
        this.activityResultListeners.add(listener);
    }

    public void unregisterActivityResultListener(ActivityResultListener listener) {
        if (this.activityResultListeners == null) {
            return;
        }
        this.activityResultListeners.remove(listener);
    }

    @Override
    protected void onActivityResult(int requestCode, int resultCode, Intent intent) {
        if (this.activityResultListeners == null) {
            return;
        }
        this.onResume();
        synchronized (this.activityResultListeners) {
            Iterator<ActivityResultListener> iterator = this.activityResultListeners.iterator();
            while (iterator.hasNext()) {
                iterator.next().onActivityResult(requestCode, resultCode, intent);
            }
        }
    }
}
