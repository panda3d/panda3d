// Filename: imageBuffer.h
// Created by:  mike (09Jan97)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://www.panda3d.org/license.txt .
//
// To contact the maintainers of this program write to
// panda3d@yahoogroups.com .
//
////////////////////////////////////////////////////////////////////

#ifndef IMAGEBUFFER_H
#define IMAGEBUFFER_H

#include "pandabase.h"

#include "drawable.h"
#include "pointerToArray.h"
#include "typedef.h"
#include "filename.h"
#include "namable.h"

////////////////////////////////////////////////////////////////////
// Defines
////////////////////////////////////////////////////////////////////

class RenderBuffer;
class DisplayRegion;

////////////////////////////////////////////////////////////////////
//       Class : ImageBuffer
// Description :
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA ImageBuffer : public ReferenceCount,
                                public WritableConfigurable, public Namable {
PUBLISHED:
  ImageBuffer() { }
  virtual ~ImageBuffer() { }

public:
  virtual void config( void ) { WritableConfigurable::config(); }

  virtual void copy(GraphicsStateGuardianBase *, const DisplayRegion *)=0;
  virtual void copy(GraphicsStateGuardianBase *, const DisplayRegion *,
                    const RenderBuffer &rb)=0;

PUBLISHED:
  INLINE bool has_filename() const;
  INLINE const Filename &get_filename() const;
  INLINE bool has_alpha_filename() const;
  INLINE const Filename &get_alpha_filename() const;

  INLINE bool has_fullpath() const;
  INLINE const Filename &get_fullpath() const;
  INLINE bool has_alpha_fullpath() const;
  INLINE const Filename &get_alpha_fullpath() const;

public:
  INLINE void set_filename(const Filename &filename);
  INLINE void clear_filename();
  INLINE void set_alpha_filename(const Filename &alpha_filename);
  INLINE void clear_alpha_filename();

  INLINE void set_fullpath(const Filename &fullpath);
  INLINE void clear_fullpath();
  INLINE void set_alpha_fullpath(const Filename &alpha_fullpath);
  INLINE void clear_alpha_fullpath();

private:
  Filename _filename;
  Filename _alpha_filename;
  Filename _fullpath;
  Filename _alpha_fullpath;

public:
  //Abstract class, so no factory methods for Reading and Writing
  virtual void write_datagram(BamWriter* manager, Datagram &me);

protected:
  void fillin(DatagramIterator& scan, BamReader* manager);

public:

  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    ReferenceCount::init_type();
    WritableConfigurable::init_type();
    Namable::init_type();
    register_type(_type_handle, "ImageBuffer",
                  ReferenceCount::get_class_type(),
                  WritableConfigurable::get_class_type(),
                  Namable::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:

  static TypeHandle _type_handle;
};

#include "imageBuffer.I"

#endif


