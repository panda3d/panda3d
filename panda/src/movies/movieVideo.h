/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file movieVideo.h
 * @author jyelon
 * @date 2007-07-02
 */

#ifndef MOVIEVIDEO_H
#define MOVIEVIDEO_H

#include "pandabase.h"
#include "namable.h"
#include "pointerTo.h"
#include "typedWritableReferenceCount.h"
#include "subfileInfo.h"
#include "filename.h"

class MovieVideoCursor;
class FactoryParams;
class BamWriter;
class BamReader;

/**
 * A MovieVideo is actually any source that provides a sequence of video
 * frames.  That could include an AVI file, a digital camera, or an internet
 * TV station.
 *
 * The difference between a MovieVideo and a MovieVideoCursor is like the
 * difference between a filename and a file handle.  The MovieVideo just
 * indicates a particular movie.  The MovieVideoCursor is what allows access.
 */
class EXPCL_PANDA_MOVIES MovieVideo : public TypedWritableReferenceCount, public Namable {
PUBLISHED:
  MovieVideo(const std::string &name = "Blank Video");
  virtual ~MovieVideo();
  virtual PT(MovieVideoCursor) open();
  static PT(MovieVideo) get(const Filename &name);

  INLINE const Filename &get_filename() const;
  INLINE const SubfileInfo &get_subfile_info() const;
  MAKE_PROPERTY(filename, get_filename);
  MAKE_PROPERTY(subfile_info, get_subfile_info);

protected:
  Filename _filename;
  SubfileInfo _subfile_info;

public:
  virtual void write_datagram(BamWriter *manager, Datagram &dg);

protected:
  void fillin(DatagramIterator &scan, BamReader *manager);

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    TypedWritableReferenceCount::init_type();
    register_type(_type_handle, "MovieVideo",
                  TypedWritableReferenceCount::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};

#include "movieVideo.I"

#endif
