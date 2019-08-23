/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file dcPacker.h
 * @author drose
 * @date 2004-06-15
 */

#ifndef DCPACKER_H
#define DCPACKER_H

#include "dcbase.h"
#include "dcPackerInterface.h"
#include "dcSubatomicType.h"
#include "dcPackData.h"
#include "dcPackerCatalog.h"

#ifdef WITHIN_PANDA
#include "extension.h"
#endif

class DCClass;
class DCSwitchParameter;

/**
 * This class can be used for packing a series of numeric and string data into
 * a binary stream, according to the DC specification.
 *
 * See also direct/src/doc/dcPacker.txt for a more complete description and
 * examples of using this class.
 */
class EXPCL_DIRECT_DCPARSER DCPacker {
PUBLISHED:
  DCPacker();
  ~DCPacker();

  INLINE void clear_data();

  void begin_pack(const DCPackerInterface *root);
  bool end_pack();

  void set_unpack_data(const vector_uchar &data);
public:
  void set_unpack_data(const char *unpack_data, size_t unpack_length,
                       bool owns_unpack_data);

PUBLISHED:
  void begin_unpack(const DCPackerInterface *root);
  bool end_unpack();

  void begin_repack(const DCPackerInterface *root);
  bool end_repack();

  bool seek(const std::string &field_name);
  bool seek(int seek_index);

  INLINE bool has_nested_fields() const;
  INLINE int get_num_nested_fields() const;
  INLINE bool more_nested_fields() const;

  INLINE const DCPackerInterface *get_current_parent() const;
  INLINE const DCPackerInterface *get_current_field() const;
  INLINE const DCSwitchParameter *get_last_switch() const;
  INLINE DCPackType get_pack_type() const;
  INLINE std::string get_current_field_name() const;

  void push();
  void pop();

  INLINE void pack_double(double value);
  INLINE void pack_int(int value);
  INLINE void pack_uint(unsigned int value);
  INLINE void pack_int64(int64_t value);
  INLINE void pack_uint64(uint64_t value);
  INLINE void pack_string(const std::string &value);
  INLINE void pack_blob(const vector_uchar &value);
  INLINE void pack_literal_value(const vector_uchar &value);
  void pack_default_value();

  INLINE double unpack_double();
  INLINE int unpack_int();
  INLINE unsigned int unpack_uint();
  INLINE int64_t unpack_int64();
  INLINE uint64_t unpack_uint64();
  INLINE std::string unpack_string();
  INLINE vector_uchar unpack_blob();
  INLINE vector_uchar unpack_literal_value();
  void unpack_validate();
  void unpack_skip();

public:
  // The following are variants on the above unpack() calls that pass the
  // result back by reference instead of as a return value.
  INLINE void unpack_double(double &value);
  INLINE void unpack_int(int &value);
  INLINE void unpack_uint(unsigned int &value);
  INLINE void unpack_int64(int64_t &value);
  INLINE void unpack_uint64(uint64_t &value);
  INLINE void unpack_string(std::string &value);
  INLINE void unpack_blob(vector_uchar &value);
  INLINE void unpack_literal_value(vector_uchar &value);

PUBLISHED:

  EXTENSION(void pack_object(PyObject *object));
  EXTENSION(PyObject *unpack_object());

  bool parse_and_pack(const std::string &formatted_object);
  bool parse_and_pack(std::istream &in);
  std::string unpack_and_format(bool show_field_names = true);
  void unpack_and_format(std::ostream &out, bool show_field_names = true);

  INLINE bool had_parse_error() const;
  INLINE bool had_pack_error() const;
  INLINE bool had_range_error() const;
  INLINE bool had_error() const;
  INLINE size_t get_num_unpacked_bytes() const;

  INLINE size_t get_length() const;
  INLINE std::string get_string() const;
  INLINE vector_uchar get_bytes() const;
  INLINE size_t get_unpack_length() const;
  INLINE std::string get_unpack_string() const;
public:
  INLINE void get_string(std::string &data) const;
  INLINE const char *get_data() const;
  INLINE char *take_data();

  INLINE void append_data(const unsigned char *buffer, size_t size);
  INLINE char *get_write_pointer(size_t size);

  INLINE const char *get_unpack_data() const;

PUBLISHED:
  INLINE static int get_num_stack_elements_ever_allocated();

  // The following methods are used only for packing (or unpacking) raw data
  // into the buffer between packing sessions (e.g.  between calls to
  // end_pack() and the next begin_pack()).

  INLINE void raw_pack_int8(int value);
  INLINE void raw_pack_int16(int value);
  INLINE void raw_pack_int32(int value);
  INLINE void raw_pack_int64(int64_t value);
  INLINE void raw_pack_uint8(unsigned int value);
  INLINE void raw_pack_uint16(unsigned int value);
  INLINE void raw_pack_uint32(unsigned int value);
  INLINE void raw_pack_uint64(uint64_t value);
  INLINE void raw_pack_float64(double value);
  INLINE void raw_pack_string(const std::string &value);
  INLINE void raw_pack_blob(const vector_uchar &value);

// this is a hack to allw me to get in and out of 32bit Mode Faster need to
// agree with channel_type in dcbase.h
#define RAW_PACK_CHANNEL(in)  raw_pack_uint64(in)
#define RAW_UNPACK_CHANNEL()  raw_unpack_uint64()


  INLINE int raw_unpack_int8();
  INLINE int raw_unpack_int16();
  INLINE int raw_unpack_int32();
  INLINE int64_t raw_unpack_int64();
  INLINE unsigned int raw_unpack_uint8();
  INLINE unsigned int raw_unpack_uint16();
  INLINE unsigned int raw_unpack_uint32();
  INLINE uint64_t raw_unpack_uint64();
  INLINE double raw_unpack_float64();
  INLINE std::string raw_unpack_string();
  INLINE vector_uchar raw_unpack_blob();

public:
  INLINE void raw_unpack_int8(int &value);
  INLINE void raw_unpack_int16(int &value);
  INLINE void raw_unpack_int32(int &value);
  INLINE void raw_unpack_int64(int64_t &value);
  INLINE void raw_unpack_uint8(unsigned int &value);
  INLINE void raw_unpack_uint16(unsigned int &value);
  INLINE void raw_unpack_uint32(unsigned int &value);
  INLINE void raw_unpack_uint64(uint64_t &value);
  INLINE void raw_unpack_float64(double &value);
  INLINE void raw_unpack_string(std::string &value);
  INLINE void raw_unpack_blob(vector_uchar &value);

public:
  static void enquote_string(std::ostream &out, char quote_mark, const std::string &str);
  static void output_hex_string(std::ostream &out, const vector_uchar &str);

private:
  INLINE void advance();
  void handle_switch(const DCSwitchParameter *switch_parameter);
  void clear();
  void clear_stack();

  EXTENSION(void pack_class_object(const DCClass *dclass, PyObject *object));
  EXTENSION(PyObject *unpack_class_object(const DCClass *dclass));
  EXTENSION(void set_class_element(PyObject *class_def, PyObject *&object,
                                   const DCField *field));
  EXTENSION(void get_class_element(const DCClass *dclass, PyObject *object,
                                   const DCField *field));

private:
  enum Mode {
    M_idle,
    M_pack,
    M_unpack,
    M_repack,
  };
  Mode _mode;

  DCPackData _pack_data;
  const char *_unpack_data;
  size_t _unpack_length;
  bool _owns_unpack_data;
  size_t _unpack_p;

  const DCPackerInterface *_root;
  const DCPackerCatalog *_catalog;
  const DCPackerCatalog::LiveCatalog *_live_catalog;

  class EXPCL_DIRECT_DCPARSER StackElement {
  public:
    // As an optimization, we implement operator new and delete here to
    // minimize allocation overhead during push() and pop().
    INLINE void *operator new(size_t size);
    INLINE void operator delete(void *ptr);

    const DCPackerInterface *_current_parent;
    int _current_field_index;
    size_t _push_marker;
    size_t _pop_marker;
    StackElement *_next;

    static StackElement *_deleted_chain;
    static int _num_ever_allocated;
  };
  StackElement *_stack;

  const DCPackerInterface *_current_field;
  const DCPackerInterface *_current_parent;
  int _current_field_index;

  // _push_marker marks the beginning of the push record (so we can go back
  // and write in the length later, or figure out the switch parameter).
  size_t _push_marker;
  // _pop_marker is used in unpack mode with certain data structures (like
  // dynamic arrays) to mark the end of the push record (so we know when we've
  // reached the end).  It is zero when it is not in use.
  size_t _pop_marker;
  int _num_nested_fields;
  const DCSwitchParameter *_last_switch;

  bool _parse_error;
  bool _pack_error;
  bool _range_error;

#ifdef WITHIN_PANDA
  friend class Extension<DCPacker>;
#endif
};

#include "dcPacker.I"

#endif
