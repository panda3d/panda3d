// Filename: config_chan.cxx
// Created by:  drose (28Feb00)
// 
////////////////////////////////////////////////////////////////////

#include "config_chan.h"
#include "animBundle.h"
#include "animBundleNode.h"
#include "animChannelBase.h"
#include "animChannelMatrixXfmTable.h"
#include "animChannelScalarTable.h"
#include "animControl.h"
#include "animGroup.h"
#include "movingPartBase.h"
#include "movingPartMatrix.h"
#include "movingPartScalar.h"
#include "partBundle.h"
#include "partBundleNode.h"
#include "partGroup.h"

#include <dconfig.h>
#include <luse.h>

Configure(config_chan);
NotifyCategoryDef(chan, "");

// This is normally set true to quantize animation channels to 16-bit
// integer values when writing to a Bam file; a cheesy way to
// hopefully achieve greater compression ratios.
const bool quantize_bam_channels = config_chan.GetBool("quantize-bam-channels", true);

ConfigureFn(config_chan) {
  AnimBundle::init_type();
  AnimBundleNode::init_type();
  AnimChannelBase::init_type();
  AnimChannelMatrixXfmTable::init_type();
  AnimChannelScalarTable::init_type();
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
  AnimChannelScalarTable::register_with_read_factory();
}




