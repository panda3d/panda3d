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
}
