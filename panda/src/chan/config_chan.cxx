// Filename: config_chan.cxx
// Created by:  drose (28Feb00)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001 - 2004, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://etc.cmu.edu/panda3d/docs/license/ .
//
// To contact the maintainers of this program write to
// panda3d-general@lists.sourceforge.net .
//
////////////////////////////////////////////////////////////////////


#include "config_chan.h"
#include "animBundle.h"
#include "animBundleNode.h"
#include "animChannelBase.h"
#include "animChannelMatrixXfmTable.h"
#include "animChannelMatrixDynamic.h"
#include "animChannelScalarTable.h"
#include "animChannelScalarDynamic.h"
#include "animControl.h"
#include "animGroup.h"
#include "movingPartBase.h"
#include "movingPartMatrix.h"
#include "movingPartScalar.h"
#include "partBundle.h"
#include "partBundleNode.h"
#include "partGroup.h"

#include "luse.h"
#include "dconfig.h"

Configure(config_chan);
NotifyCategoryDef(chan, "");

ConfigVariableBool compress_channels
("compress-channels", false,
 PRC_DESC("Set this true to enable lossy compression of animation channels "
          "when writing to the bam file.  This serves to reduce the size of "
          "the bam file only; it does reduce the memory footprint of the "
          "channels when the bam file is loaded."));

// There are some special values above 100 which are generally only
// useful for debugging (specifically, to research at what point a
// particular animation artifact is being introduced):
//
//   101  Output numerically lossless data.  The output is not run
//        through the FFTCompressor.  This can be used to check
//        whether a particular artifact is due to the FFT conversion
//        or not.  However, joint angles (HPR) are still converted to
//        quaternions and normalized, discarding the fourth
//        (redundant) component.
//
//   102  As above, but the fourth quaternion component is preserved.
//
//   103  Quaternions are not used; instead, the HPR values are written
//        directly.  All output is now completely lossless; if some
//        artifacts are being introduced at this point, check your
//        sanity.
//
ConfigVariableInt compress_chan_quality
("compress-chan-quality", 95,
 PRC_DESC("The quality level is an integer number that generally ranges "
          "between 0 and 100, where smaller numbers indicate greater "
          "compression at the cost of quality, and larger numbers indicate "
          "higher quality but less compression.  Generally, 95 is the highest "
          "useful value; values between 95 and 100 produce substantially "
          "larger, but not substantially better, output files.  This is akin "
          "to the JPEG compression level."));

ConfigVariableBool read_compressed_channels
("read-compressed-channels", true,
PRC_DESC("Set this false to disable reading of compressed animation channels, "
         "even if the decompression code is available.  The only reason you "
         "might want to do this would be to speed load time when you don't "
         "care about what the animation looks like."));


ConfigureFn(config_chan) {
  AnimBundle::init_type();
  AnimBundleNode::init_type();
  AnimChannelBase::init_type();
  AnimChannelMatrixXfmTable::init_type();
  AnimChannelMatrixDynamic::init_type();
  AnimChannelScalarTable::init_type();
  AnimChannelScalarDynamic::init_type();
  AnimControl::init_type();
  AnimGroup::init_type();
  MovingPartBase::init_type();
  MovingPartMatrix::init_type();
  MovingPartScalar::init_type();
  PartBundle::init_type();
  PartBundleNode::init_type();
  PartGroup::init_type();

  // This isn't defined in this package, but it *is* essential that it
  // be initialized.  We have to do it explicitly here since template
  // statics don't necessarily resolve very well across dynamic
  // libraries.
  LMatrix4f::init_type();

  //Registration of writeable object's creation
  //functions with BamReader's factory
  PartGroup::register_with_read_factory();
  PartBundle::register_with_read_factory();
  MovingPartMatrix::register_with_read_factory();
  MovingPartScalar::register_with_read_factory();

  AnimGroup::register_with_read_factory();
  AnimBundle::register_with_read_factory();
  AnimBundleNode::register_with_read_factory();
  AnimChannelMatrixXfmTable::register_with_read_factory();
  AnimChannelMatrixDynamic::register_with_read_factory();
  AnimChannelScalarTable::register_with_read_factory();
  AnimChannelScalarDynamic::register_with_read_factory();
}




