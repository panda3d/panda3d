// Filename: iffInputFile.h
// Created by:  drose (24Apr01)
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

#ifndef IFFINPUTFILE_H
#define IFFINPUTFILE_H

#include "pandatoolbase.h"

#include "iffId.h"
#include "iffChunk.h"

#include "typedObject.h"
#include "pointerTo.h"

class Datagram;

////////////////////////////////////////////////////////////////////
//       Class : IffInputFile
// Description : A wrapper around an istream used for reading an IFF
//               file.
////////////////////////////////////////////////////////////////////
class IffInputFile : public TypedObject {
public:
  IffInputFile();
  virtual ~IffInputFile();

  bool open_read(Filename filename);
  void set_input(istream *input, bool owns_istream);

  INLINE void set_filename(const Filename &filename);
  INLINE const Filename &get_filename() const;

  INLINE bool is_eof() const;
  INLINE size_t get_bytes_read() const;

  INLINE void align();

  PN_int8 get_int8();
  PN_uint8 get_uint8();

  PN_int16 get_be_int16();
  PN_int32 get_be_int32();
  PN_uint16 get_be_uint16();
  PN_uint32 get_be_uint32();
  PN_stdfloat get_be_float32();

  string get_string();

  IffId get_id();

  PT(IffChunk) get_chunk();
  PT(IffChunk) get_subchunk(IffChunk *context);

  bool read_byte(char &byte);
  bool read_bytes(Datagram &datagram, int length);
  bool skip_bytes(int length);

protected:
  virtual IffChunk *make_new_chunk(IffId id);

  istream *_input;
  Filename _filename;
  bool _owns_istream;
  bool _eof;
  bool _unexpected_eof;
  size_t _bytes_read;

public:
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    TypedObject::init_type();
    register_type(_type_handle, "IffInputFile",
                  TypedObject::get_class_type());
  }

private:
  static TypeHandle _type_handle;

  friend class IffChunk;
};

#include "iffInputFile.I"

#endif


