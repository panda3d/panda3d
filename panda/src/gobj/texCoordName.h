// Filename: texCoordName.h
// Created by:  drose (15Jul04)
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

#ifndef TEXCOORDNAME_H
#define TEXCOORDNAME_H

#include "pandabase.h"

#include "typedWritableReferenceCount.h"

class FactoryParams;

////////////////////////////////////////////////////////////////////
//       Class : TexCoordName
// Description : Associates a name with a set of texture coordinates,
//               for the purpose of storing within a Geom.  This is
//               used in conjunction with TextureStage to support
//               multitexturing.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA TexCoordName : public TypedWritableReferenceCount {
private:
  TexCoordName(const string &name);

PUBLISHED:
  virtual ~TexCoordName();

  static const TexCoordName *make(const string &name);
  INLINE const string &get_name() const;

  void output(ostream &out) const;

  INLINE static const TexCoordName *get_default();

private:
  string _name;

  typedef pmap<string, TexCoordName *> TexCoordsByName;
  static TexCoordsByName _texcoords_by_name;

  static CPT(TexCoordName) _default_name;
  
public:
  // Datagram stuff
  static void register_with_read_factory();
  virtual void write_datagram(BamWriter *manager, Datagram &me);

  virtual void finalize();

protected:
  static TypedWritable *make_from_bam(const FactoryParams &params);

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    TypedWritableReferenceCount::init_type();
    register_type(_type_handle, "TexCoordName",
                  TypedWritableReferenceCount::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};

INLINE ostream &operator << (ostream &out, const TexCoordName &tcn);

#include "texCoordName.I"

#endif


  
