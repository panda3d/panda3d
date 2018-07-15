/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file virtualFileMountAndroidAsset.I
 * @author rdb
 * @date 2013-01-21
 */

/**
 *
 */
VirtualFileMountAndroidAsset::
VirtualFileMountAndroidAsset(AAssetManager *mgr, const std::string &apk_path) :
  _asset_mgr(mgr), _apk_path(apk_path)
{
}

/**
 *
 */
INLINE VirtualFileMountAndroidAsset::AssetStream::
AssetStream(AAsset *asset) :
  std::istream(new VirtualFileMountAndroidAsset::AssetStreamBuf(asset)) {
}
