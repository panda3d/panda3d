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
//
////////////////////////////////////////////////////////////////////
// Includes
////////////////////////////////////////////////////////////////////
#include <pandabase.h>

#include "drawable.h"
#include <pointerToArray.h>
#include <typedef.h>
#include <filename.h>
#include <namable.h>

////////////////////////////////////////////////////////////////////
// Defines
////////////////////////////////////////////////////////////////////

class RenderBuffer;
class DisplayRegion;

////////////////////////////////////////////////////////////////////
//       Class : ImageBuffer
// Description :
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA ImageBuffer : public dDrawable, public Namable
{
PUBLISHED:
  ImageBuffer( void ) : dDrawable() { }
  virtual ~ImageBuffer( void ) { }

  virtual bool read( const string& name ) = 0;
  virtual bool write( const string& name = "" ) const = 0;

public:
  virtual void config( void ) { WritableConfigurable::config(); }

  virtual void copy(GraphicsStateGuardianBase *, const DisplayRegion *)=0;
  virtual void copy(GraphicsStateGuardianBase *, const DisplayRegion *,
                    const RenderBuffer &rb)=0;
  virtual void draw(GraphicsStateGuardianBase *)=0;
  virtual void draw(GraphicsStateGuardianBase *, const DisplayRegion *)=0;
  virtual void draw(GraphicsStateGuardianBase *, const DisplayRegion *,
                    const RenderBuffer &rb)=0;

  INLINE void set_alpha_name(const string &alpha_name);
  INLINE void clear_alpha_name();
  INLINE bool has_alpha_name() const;
  INLINE const string &get_alpha_name() const;

private:
  string _alpha_name;

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
    dDrawable::init_type();
    Namable::init_type();
    register_type(_type_handle, "ImageBuffer",
                  dDrawable::get_class_type(),
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


