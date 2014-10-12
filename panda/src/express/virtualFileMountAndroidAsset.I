// Filename: virtualFileMountAndroidAsset.cxx
// Created by:  rdb (21Jan13)
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


////////////////////////////////////////////////////////////////////
//     Function: VirtualFileMountAndroidAsset::Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
VirtualFileMountAndroidAsset::
VirtualFileMountAndroidAsset(AAssetManager *mgr, const string &apk_path) :
  _asset_mgr(mgr), _apk_path(apk_path)
{
}

////////////////////////////////////////////////////////////////////
//     Function: VirtualFileMountAndroidAsset::AssetStream::Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE VirtualFileMountAndroidAsset::AssetStream::
AssetStream(AAsset *asset) :
  istream(new VirtualFileMountAndroidAsset::AssetStreamBuf(asset)) {
}
