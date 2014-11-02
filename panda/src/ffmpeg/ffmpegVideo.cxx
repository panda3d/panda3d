// Filename: ffmpegVideo.cxx
// Created by: jyelon (01Aug2007)
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

#include "ffmpegVideo.h"
#include "ffmpegVideoCursor.h"
#include "config_ffmpeg.h"
#include "bamReader.h"
#include "dcast.h"

TypeHandle FfmpegVideo::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: FfmpegVideo::Constructor
//       Access: Public
//  Description: Constructs an ffmpeg video that reads its contents
//               from the indicate filename, which may be a file in
//               the VFS.
////////////////////////////////////////////////////////////////////
FfmpegVideo::
FfmpegVideo(const Filename &name) :
  MovieVideo(name)
{
  _filename = name;
}

////////////////////////////////////////////////////////////////////
//     Function: FfmpegVideo::Constructor
//       Access: Public
//  Description: Constructs an ffmpeg video that reads its contents
//               from the indicated subfile information.  This is
//               normally used for low-level purposes only; you would
//               normally use the constructor that takes a filename.
////////////////////////////////////////////////////////////////////
FfmpegVideo::
FfmpegVideo(const SubfileInfo &info) :
  MovieVideo(info.get_filename())
{
  _filename = info.get_filename();
  _subfile_info = info;
}

////////////////////////////////////////////////////////////////////
//     Function: FfmpegVideo::Destructor
//       Access: Public
//  Description: xxx
////////////////////////////////////////////////////////////////////
FfmpegVideo::
~FfmpegVideo() {
}

////////////////////////////////////////////////////////////////////
//     Function: FfmpegVideo::open
//       Access: Published, Virtual
//  Description: Open this video, returning a MovieVideoCursor.
////////////////////////////////////////////////////////////////////
PT(MovieVideoCursor) FfmpegVideo::
open() {
  PT(FfmpegVideoCursor) result = new FfmpegVideoCursor(this);
  if (result->_format_ctx == 0) {
    ffmpeg_cat.error() << "Could not open " << _filename << "\n";
    return NULL;
  } else {
    return result.p();
  }
}

////////////////////////////////////////////////////////////////////
//     Function: FfmpegVideo::make
//       Access: Published, Static
//  Description: Obtains a MovieVideo that references a file.
////////////////////////////////////////////////////////////////////
PT(MovieVideo) FfmpegVideo::
make(const Filename &name) {
  return DCAST(MovieVideo, new FfmpegVideo(name));
}

////////////////////////////////////////////////////////////////////
//     Function: FfmpegVideo::register_with_read_factory
//       Access: Public, Static
//  Description: Tells the BamReader how to create objects of type
//               FfmpegVideo.
////////////////////////////////////////////////////////////////////
void FfmpegVideo::
register_with_read_factory() {
  BamReader::get_factory()->register_factory(get_class_type(), make_from_bam);
}

////////////////////////////////////////////////////////////////////
//     Function: FfmpegVideo::write_datagram
//       Access: Public, Virtual
//  Description: Writes the contents of this object to the datagram
//               for shipping out to a Bam file.
////////////////////////////////////////////////////////////////////
void FfmpegVideo::
write_datagram(BamWriter *manager, Datagram &dg) {
  MovieVideo::write_datagram(manager, dg);
}

////////////////////////////////////////////////////////////////////
//     Function: FfmpegVideo::make_from_bam
//       Access: Protected, Static
//  Description: This function is called by the BamReader's factory
//               when a new object of type FfmpegVideo is encountered
//               in the Bam file.  It should create the FfmpegVideo
//               and extract its information from the file.
////////////////////////////////////////////////////////////////////
TypedWritable *FfmpegVideo::
make_from_bam(const FactoryParams &params) {
  FfmpegVideo *video = new FfmpegVideo("");
  DatagramIterator scan;
  BamReader *manager;

  parse_params(params, scan, manager);
  video->fillin(scan, manager);

  return video;
}

////////////////////////////////////////////////////////////////////
//     Function: FfmpegVideo::fillin
//       Access: Protected
//  Description: This internal function is called by make_from_bam to
//               read in all of the relevant data from the BamFile for
//               the new FfmpegVideo.
////////////////////////////////////////////////////////////////////
void FfmpegVideo::
fillin(DatagramIterator &scan, BamReader *manager) {
  MovieVideo::fillin(scan, manager);
}
