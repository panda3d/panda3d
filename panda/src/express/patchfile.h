// Filename: patchfile.h
// Created by:  mike, darren (09Jan97)
//
////////////////////////////////////////////////////////////////////
//
#ifndef PATCHFILE_H
#define PATCHFILE_H
//
////////////////////////////////////////////////////////////////////
// Includes
////////////////////////////////////////////////////////////////////
#include <pandabase.h>

#include "typedef.h"
#include <notify.h>
#include <filename.h>
#include <list>
#include "datagram.h"
#include "datagramIterator.h"
#include "buffer.h"
#include "pointerTo.h"

#include <algorithm>

////////////////////////////////////////////////////////////////////
//       Class : Patchfile
// Description :
////////////////////////////////////////////////////////////////////
class EXPCL_PANDAEXPRESS Patchfile {
public:
  Patchfile(void);
  Patchfile(PT(Buffer) buffer);
  ~Patchfile(void);

  bool build(Filename &file_orig, Filename &file_new);
  bool apply(Filename &patch, Filename &file);

  int find_longest_sequence(Filename &infile, int &pos, int &len) const;

  void reset(void);

private:
  void init(PT(Buffer) buffer);

private:
  // stuff for the build operation
  INLINE void write_header(ofstream &write_stream, const string &name);
  void build_hash_link_tables(const char *buffer_orig, PN_uint32 length_orig,
    PN_uint32 *hash_table, PN_uint32 *link_table);
  PN_uint16 calc_hash(const char *buffer);
  void find_longest_match(PN_uint32 new_pos, PN_uint32 &copy_offset, PN_uint32 &copy_length,
    PN_uint32 *hash_table, PN_uint32 *link_table, const char* buffer_orig,
    PN_uint32 length_orig, const char* buffer_new, PN_uint32 length_new);
  PN_uint32 calc_match_length(const char* buf1, const char* buf2, PN_uint32 max_length);

  void emit_ADD(ofstream &write_stream, PN_uint32 length, const char* buffer);
  void emit_COPY(ofstream &write_stream, PN_uint32 length, PN_uint32 offset);

  static const PN_uint32 _HASHTABLESIZE;
  static const PN_uint32 _footprint_length;
  static const PN_uint32 _NULL_VALUE;

protected:
  PT(Buffer) _buffer; // this is the work buffer for apply -- used to prevent virtual memory swapping
  Filename _temp_file_name; // this is the temp file
  Datagram _datagram; // used to eliminate endian problems

  static const PN_uint32 _magic_number;
};

#include "patchfile.I"

#endif
