// Filename: patchfile.h
// Created by:  mike (09Jan97)
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

////////////////////////////////////////////////////////////////////
//       Class : Patchfile 
// Description :
////////////////////////////////////////////////////////////////////
class EXPCL_PANDAEXPRESS Patchfile {
public:
  Patchfile(void);
  Patchfile(PT(Buffer) buffer);
  ~Patchfile(void);

  bool build(Filename &file_a, Filename &file_b);
  bool apply(Filename &patch, Filename &file);

  int find_longest_sequence(Filename &infile, int &pos, int &len) const;

  void reset(void);

private:
  void init(PT(Buffer) buffer);
  INLINE void write_header(ofstream &write_stream, const string &name); 
  INLINE void write_entry(ofstream &write_stream, int pos, int n, int m);
  int find_next_difference(const char *buf_a, int size_a, 
			   const char *buf_b, int size_b);
  int find_match(const char *win, int win_len, const char *buf,
				  int buf_len);
  bool find_next_match(const char *buf_a, int size_a, int &pos_a,
		       const char *buf_b, int size_b, int &pos_b);
  bool find_next_zone_match(const char *buf_a, int size_a, int &pos_a,
			    const char *buf_b, int size_b, int &pos_b);
  bool find_next_z_match(const char *buf_a, int size_a, int &pos_a,
			 const char *buf_b, int size_b, int &pos_b);
  bool is_match(const char *buf_a, const char *buf_b, 
	       				int size) const;
 
  class Entry {
  public:
    Entry(void);

  public:
    int _pos;
    int _n;
    int _m;
    char *_buffer;
    int _len;
  };

private:
  Datagram _datagram;
  int _win_len;
  int _zone_len;
  int _increment;
  Filename _name;
  int _min_sample_size;
  PT(Buffer) _buffer;
  int _header_length_length;
  Entry *_current_entry;
  Filename _temp_file_name;
 
  static PN_uint32 _magic_number;
};

#include "patchfile.I"

#endif
