/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file dcPackerInterface.h
 * @author drose
 * @date 2004-06-15
 */

#ifndef DCPACKERINTERFACE_H
#define DCPACKERINTERFACE_H

#include "dcbase.h"
#include "dcSubatomicType.h"
#include "vector_uchar.h"

class DCFile;
class DCField;
class DCSimpleParameter;
class DCSwitchParameter;
class DCClassParameter;
class DCArrayParameter;
class DCAtomicField;
class DCMolecularField;
class DCPackData;
class DCPackerCatalog;

BEGIN_PUBLISH
// This enumerated type is returned by get_pack_type() and represents the best
// choice for a subsequent call to pack_*() or unpack_*().
enum DCPackType {
  // This one should never be returned in a normal situation.
  PT_invalid,

  // These PackTypes are all fundamental types, and should be packed (or
  // unpacked) with the corresponding call to pack_double(), pack_int(), etc.
  // PT_blob is similar to PT_string, except that it contains arbitrary binary
  // data instead of just UTF-8 text.
  PT_double,
  PT_int,
  PT_uint,
  PT_int64,
  PT_uint64,
  PT_string,
  PT_blob,

  // The remaining PackTypes imply a need to call push() and pop(). They are
  // all variants on the same thing: a list of nested fields, but the PackType
  // provides a bit of a semantic context.
  PT_array,
  PT_field,
  PT_class,
  PT_switch,
};
END_PUBLISH

/**
 * This defines the internal interface for packing values into a DCField.  The
 * various different DC objects inherit from this.
 *
 * Normally these methods are called only by the DCPacker object; the user
 * wouldn't normally call these directly.
 */
class EXPCL_DIRECT_DCPARSER DCPackerInterface {
public:
  DCPackerInterface(const std::string &name = std::string());
  DCPackerInterface(const DCPackerInterface &copy);
  virtual ~DCPackerInterface();

PUBLISHED:
  INLINE const std::string &get_name() const;
  int find_seek_index(const std::string &name) const;

  virtual DCField *as_field();
  virtual const DCField *as_field() const;
  virtual DCSwitchParameter *as_switch_parameter();
  virtual const DCSwitchParameter *as_switch_parameter() const;
  virtual DCClassParameter *as_class_parameter();
  virtual const DCClassParameter *as_class_parameter() const;

  INLINE bool check_match(const DCPackerInterface *other) const;
  bool check_match(const std::string &description, DCFile *dcfile = nullptr) const;

public:
  virtual void set_name(const std::string &name);
  INLINE bool has_fixed_byte_size() const;
  INLINE size_t get_fixed_byte_size() const;
  INLINE bool has_fixed_structure() const;
  INLINE bool has_range_limits() const;
  INLINE size_t get_num_length_bytes() const;

  INLINE bool has_nested_fields() const;
  INLINE int get_num_nested_fields() const;
  virtual int calc_num_nested_fields(size_t length_bytes) const;
  virtual DCPackerInterface *get_nested_field(int n) const;

  virtual bool validate_num_nested_fields(int num_nested_fields) const;

  INLINE DCPackType get_pack_type() const;

  virtual void pack_double(DCPackData &pack_data, double value,
                           bool &pack_error, bool &range_error) const;
  virtual void pack_int(DCPackData &pack_data, int value,
                        bool &pack_error, bool &range_error) const;
  virtual void pack_uint(DCPackData &pack_data, unsigned int value,
                         bool &pack_error, bool &range_error) const;
  virtual void pack_int64(DCPackData &pack_data, int64_t value,
                          bool &pack_error, bool &range_error) const;
  virtual void pack_uint64(DCPackData &pack_data, uint64_t value,
                           bool &pack_error, bool &range_error) const;
  virtual void pack_string(DCPackData &pack_data, const std::string &value,
                           bool &pack_error, bool &range_error) const;
  virtual void pack_blob(DCPackData &pack_data, const vector_uchar &value,
                         bool &pack_error, bool &range_error) const;
  virtual bool pack_default_value(DCPackData &pack_data, bool &pack_error) const;

  virtual void unpack_double(const char *data, size_t length, size_t &p,
                             double &value, bool &pack_error, bool &range_error) const;
  virtual void unpack_int(const char *data, size_t length, size_t &p,
                          int &value, bool &pack_error, bool &range_error) const;
  virtual void unpack_uint(const char *data, size_t length, size_t &p,
                           unsigned int &value, bool &pack_error, bool &range_error) const;
  virtual void unpack_int64(const char *data, size_t length, size_t &p,
                            int64_t &value, bool &pack_error, bool &range_error) const;
  virtual void unpack_uint64(const char *data, size_t length, size_t &p,
                             uint64_t &value, bool &pack_error, bool &range_error) const;
  virtual void unpack_string(const char *data, size_t length, size_t &p,
                             std::string &value, bool &pack_error, bool &range_error) const;
  virtual void unpack_blob(const char *data, size_t length, size_t &p,
                           vector_uchar &value, bool &pack_error, bool &range_error) const;
  virtual bool unpack_validate(const char *data, size_t length, size_t &p,
                               bool &pack_error, bool &range_error) const;
  virtual bool unpack_skip(const char *data, size_t length, size_t &p,
                           bool &pack_error) const;

  // These are the low-level interfaces for packing and unpacking numbers from
  // a buffer.  You're responsible for making sure the buffer has enough room,
  // and for incrementing the pointer.
  INLINE static void do_pack_int8(char *buffer, int value);
  INLINE static void do_pack_int16(char *buffer, int value);
  INLINE static void do_pack_int32(char *buffer, int value);
  INLINE static void do_pack_int64(char *buffer, int64_t value);
  INLINE static void do_pack_uint8(char *buffer, unsigned int value);
  INLINE static void do_pack_uint16(char *buffer, unsigned int value);
  INLINE static void do_pack_uint32(char *buffer, unsigned int value);
  INLINE static void do_pack_uint64(char *buffer, uint64_t value);
  INLINE static void do_pack_float64(char *buffer, double value);

  INLINE static int do_unpack_int8(const char *buffer);
  INLINE static int do_unpack_int16(const char *buffer);
  INLINE static int do_unpack_int32(const char *buffer);
  INLINE static int64_t do_unpack_int64(const char *buffer);
  INLINE static unsigned int do_unpack_uint8(const char *buffer);
  INLINE static unsigned int do_unpack_uint16(const char *buffer);
  INLINE static unsigned int do_unpack_uint32(const char *buffer);
  INLINE static uint64_t do_unpack_uint64(const char *buffer);
  INLINE static double do_unpack_float64(const char *buffer);

  INLINE static void validate_int_limits(int value, int num_bits,
                                         bool &range_error);
  INLINE static void validate_int64_limits(int64_t value, int num_bits,
                                           bool &range_error);
  INLINE static void validate_uint_limits(unsigned int value, int num_bits,
                                          bool &range_error);
  INLINE static void validate_uint64_limits(uint64_t value, int num_bits,
                                            bool &range_error);

  const DCPackerCatalog *get_catalog() const;

protected:
  virtual bool do_check_match(const DCPackerInterface *other) const=0;

public:
  // These are declared public just so the derived classes can call them
  // easily.  They're not intended to be called directly.

  virtual bool do_check_match_simple_parameter(const DCSimpleParameter *other) const;
  virtual bool do_check_match_class_parameter(const DCClassParameter *other) const;
  virtual bool do_check_match_switch_parameter(const DCSwitchParameter *other) const;
  virtual bool do_check_match_array_parameter(const DCArrayParameter *other) const;
  virtual bool do_check_match_atomic_field(const DCAtomicField *other) const;
  virtual bool do_check_match_molecular_field(const DCMolecularField *other) const;

private:
  void make_catalog();

protected:
  std::string _name;
  bool _has_fixed_byte_size;
  size_t _fixed_byte_size;
  bool _has_fixed_structure;
  bool _has_range_limits;
  size_t _num_length_bytes;
  bool _has_nested_fields;
  int _num_nested_fields;
  DCPackType _pack_type;

private:
  DCPackerCatalog *_catalog;
};

#include "dcPackerInterface.I"

#endif
