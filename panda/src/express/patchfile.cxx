// Filename: patchfile.cxx
// Created by:  mike (09Jan97)
//
////////////////////////////////////////////////////////////////////
// Copyright (C) 1992,93,94,95,96,97,98  
// Walt Disney Imagineering, Inc.
//
// These  coded  instructions,  statements,  data   structures   and
// computer  programs contain unpublished proprietary information of
// Walt Disney Imagineering and are protected by  Federal  copyright
// law.  They may  not be  disclosed to third  parties  or copied or
// duplicated in any form, in whole or in part,  without  the  prior
// written consent of Walt Disney Imagineering Inc.
////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////
// Includes
////////////////////////////////////////////////////////////////////
#include "patchfile.h"
#include "config_express.h"

#include <stdio.h> // for tempnam

////////////////////////////////////////////////////////////////////
// Defines
////////////////////////////////////////////////////////////////////
PN_uint32 Patchfile::_magic_number = 0xfeebfaab;

////////////////////////////////////////////////////////////////////
//     Function: Patchfile::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
Patchfile::
Patchfile(void) {
  PT(Buffer) buffer = new Buffer(patchfile_buffer_size);
  init(buffer);
}

////////////////////////////////////////////////////////////////////
//     Function: Patchfile::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
Patchfile::
Patchfile(PT(Buffer) buffer) {
  init(buffer);
}

////////////////////////////////////////////////////////////////////
//     Function: Patchfile::init
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
void Patchfile::
init(PT(Buffer) buffer) {
  nassertv(!buffer.is_null());
  _win_len = patchfile_window_size;
  _zone_len = patchfile_zone_size;
  _increment = patchfile_increment_size;
  _name.set_binary();
  _buffer = buffer;
  _header_length_length = sizeof(PN_uint32) + sizeof(PN_int32);
  char *temp_name = tempnam(NULL, "pf");
  _temp_file_name = temp_name;
  _temp_file_name.set_binary();
  delete temp_name;
  reset();
}

////////////////////////////////////////////////////////////////////
//     Function: Patchfile::Destructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
Patchfile::
~Patchfile(void) {
  _temp_file_name.unlink();
}

////////////////////////////////////////////////////////////////////
//     Function: Patchfile::find_next_difference
//       Access: Private
//  Description: Finds the first byte that differs between buf_a and
//               buf_b.  Returns -1 if buf_a == buf_b.
////////////////////////////////////////////////////////////////////
int Patchfile::
find_next_difference(const char *buf_a, int size_a, 
		     const char *buf_b, int size_b) {
  int i;
  for (i = 0; i < size_a && i < size_b; i++) {
    if (buf_a[i] != buf_b[i])
      return i;
  }

  // buf_a == buf_b
  if (size_a == size_b)
    return -1;

  return i + 1;
}

////////////////////////////////////////////////////////////////////
//     Function: Patchfile::is_match
//       Access: Private
//  Description: 
////////////////////////////////////////////////////////////////////
bool Patchfile::
is_match(const char *buf_a, const char *buf_b, int size) const {
  if (size < _win_len)
    return false;

  for (int i = 0; i < size; i++) {
    if (buf_a[i] != buf_b[i])
      return false;
  }
  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: Patchfile::find_match
//       Access: Private
//  Description: Run a window along a buffer in specified increments
//		 until a match is found.
//		 Returns -1 on failure. 
////////////////////////////////////////////////////////////////////
int Patchfile::
find_match(const char *win, int win_len, const char *buf, int buf_len) {
  if (win_len < _win_len)
    return -1;

  char *bufptr = (char *)buf;
  int bytes_left = buf_len;
  int sample_len;
  for (int i = 0; i < buf_len; i++) {
    sample_len = (win_len <= bytes_left) ? win_len : bytes_left;
    if (is_match(win, bufptr, sample_len) == true) {
      return i;
    }
    bufptr++;
    bytes_left--;
  }

  return -1;
}

////////////////////////////////////////////////////////////////////
//     Function: Patchfile::find_next_zone_match
//       Access: Private
//  Description: Advance a window down the entire length of a buffer
//               looking for a complete match.  If one is not found,
//               advance the window by _win_len and try again until
//               a match is found or we reach the end of the buffer.
//               Returns false on failure.
////////////////////////////////////////////////////////////////////
bool Patchfile::
find_next_zone_match(const char *buf_a, int size_a, int &pos_a,
		     const char *buf_b, int size_b, int &pos_b) {
  // Handle boundary conditions 
  if (size_a == 0 || size_b == 0) {
    express_cat.debug()
      << "Patchfile::find_next_zone_match() - size a = " << size_a 
      << " size b = " << size_b << endl;
    return false;
  }

  char *winptr = (char *)buf_a;
  int bytes_left = size_a;
  int win_len, pos;
  int i, j, k;
  pos_a = 0;
  for (i = 0; i < size_a; i += _increment) {
    win_len = (_win_len <= bytes_left) ? _win_len : bytes_left;    
    pos = find_match(winptr, win_len, buf_b, size_b);
    if (pos != -1) {
cerr << "found a match at: " << pos << endl;
      // Back up the window in case we missed part of an earlier match
      if (pos_a >= win_len) {
        int prev_pos_a = pos_a - _increment;
        for (j = pos_a, k = pos;
	     j > prev_pos_a && k > 0; 
  	     j--, k--) {
	  if (buf_a[j] != buf_b[k]) 
	    break;
        }
        pos_a = j+1;
        pos_b = k+1;
      } else
	pos_b = pos;
      return true;
    }
    pos_a += _increment;
    winptr += _increment;
    bytes_left -= _increment;
  }

  return false;
}

////////////////////////////////////////////////////////////////////
//     Function: Patchfile::find_next_z_match
//       Access: Public
//  Description: Starts with a zone in stream b and searches for
//		 a match.  If a match is not found the zone is 
//		 shifted down and we try again.
////////////////////////////////////////////////////////////////////
bool Patchfile::
find_next_z_match(const char *buf_a, int size_a, int &pos_a,
		  const char *buf_b, int size_b, int &pos_b) {
  // Handle boundary conditions
  if (size_a == 0 || size_b == 0) {
    express_cat.debug()
      << "Patchfile::find_next_z_match() - size a = " << size_a
      << " size b = " << size_b << endl;
    return false;
  }

  char *bufbptr = (char *)buf_b;
  int sizeb;
  int bytes_left = size_b;
  int starting_pos = 0;
  for (int i = 0; i < size_b; i += _zone_len) {
    sizeb = (_zone_len <= bytes_left) ? _zone_len : bytes_left;
    if (find_next_zone_match(buf_a, size_a, pos_a, bufbptr, sizeb, pos_b)
	== true) {
      pos_b += starting_pos; 
      return true;
    }
    bytes_left -= sizeb;
    bufbptr += sizeb;
    starting_pos += sizeb;
  }

  return false;
}

////////////////////////////////////////////////////////////////////
//     Function: Patchfile::find_next_match
//       Access: Public
//  Description: Starts with a zone in stream a and searches for a
//		 match.  If a match is not found, expands the zone
//		 in stream a and starts again.
////////////////////////////////////////////////////////////////////
bool Patchfile::
find_next_match(const char *buf_a, int size_a, int &pos_a,
                const char *buf_b, int size_b, int &pos_b) {
  // Handle boundary conditions
  if (size_a == 0 || size_b == 0) {
    express_cat.debug()
      << "Patchfile::find_next_match() - size a = " << size_a
      << " size b = " << size_b << endl;
    return false;
  }

  int sizea;
  int bytes_left = size_a;
  for (int i = 0; i < size_a; i += _zone_len) {
    sizea = (_zone_len <= bytes_left) ? _zone_len : bytes_left;
    if (find_next_z_match(buf_a, sizea, pos_a, buf_b, size_b, pos_b)
        == true) {
      return true;
    }
    bytes_left -= sizea;
  }

  return false;
}

////////////////////////////////////////////////////////////////////
//     Function: Patchfile::build
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
bool Patchfile::
build(Filename &file_a, Filename &file_b) {

  if (express_cat.is_debug()) {
    int pos = 0;
    int len = 0;
    int seq = find_longest_sequence(file_a, pos, len);
    express_cat.debug()
      << "File: " << file_a << " longest sequence = " << seq << " at "
      << "position: " << pos << " of " << len << endl;
    pos = 0;
    len = 0;
    seq = find_longest_sequence(file_b, pos, len);
    express_cat.debug()
      << "File: " << file_b << " longest sequence = " << seq << " at "
      << "position: " << pos << " of " << len << endl;
  }

  // Open the file for read
  ifstream stream_a;
  file_a.set_binary();
  if (!file_a.open_read(stream_a)) {
    express_cat.error()
      << "Patchfile::build() - Failed to open file: " << file_a << endl;
    return false;
  }

  // Open the file for read
  ifstream stream_b;
  file_b.set_binary();
  if (!file_b.open_read(stream_b)) {
    express_cat.error()
      << "Patchfile::build() - Failed to open file: " << file_b << endl;
    return false;
  }

  // Open patch file for write
  ofstream write_stream;
  _name = file_a.get_fullpath() + ".pch";
  if (!_name.open_write(write_stream)) {
    express_cat.error()
      << "Patchfile::build() - Failed to open file: " << _name << endl;
    return false;
  }

  // Determine file length
  stream_a.seekg(0, ios::end);
  int length_a = stream_a.tellg();
  char *buffer_a = new char[length_a]; 
  stream_a.seekg(0, ios::beg);
  stream_a.read(buffer_a, length_a);

  // Determine file length
  stream_b.seekg(0, ios::end);
  int length_b = stream_b.tellg();
  char *buffer_b = new char[length_b];
  stream_b.seekg(0, ios::beg);
  stream_b.read(buffer_b, length_b);

  char *next_a = buffer_a;
  int remaining_a = length_a;
  char *next_b = buffer_b;
  int remaining_b = length_b;

  // Strip the v# out of the filename
  // Save the original extension
  string ext = file_a.get_extension();
  // Strip out the extension
  Filename tfile = file_a.get_basename_wo_extension();
  // Now strip out the .v#
  string fname = tfile.get_basename_wo_extension();
  fname += ".";
  fname += ext;
  write_header(write_stream, fname);

  bool done = false;
  int starting_pos = 0;
  int pos_a, pos_b;
cerr << "remaining_a: " << remaining_a << " remaining_b: " << remaining_b << endl;
  while (done == false) {
    int diff_pos = find_next_difference(next_a, remaining_a,
					next_b, remaining_b);
    if (diff_pos == -1)
      done = true;
    else if (diff_pos >= remaining_a || diff_pos >= remaining_b) {
cerr << "madeit" << endl;
      done = true;
    } else {
cerr << "found a diff at: " << starting_pos + diff_pos << endl;
      starting_pos += diff_pos;
      next_a += diff_pos;
      remaining_a -= diff_pos;
      next_b += diff_pos;
      remaining_b -= diff_pos;
      bool match = find_next_match(next_a, remaining_a, pos_a,
				   next_b, remaining_b, pos_b);  
      if (match == false) {
 	// Files differ from here on out - replace remaining_a bytes
	// with remaining_b bytes
	write_entry(write_stream, starting_pos, remaining_a, remaining_b);
	if (remaining_b > 0)
	  write_stream.write(next_b, remaining_b);
	done = true;
      } else {
	// Write an entry and start again		
	write_entry(write_stream, starting_pos, pos_a, pos_b); 
	if (pos_b > 0)
	  write_stream.write(next_b, pos_b);
	next_a += pos_a;
	remaining_a -= pos_a;
 	next_b += pos_b;
	remaining_b -= pos_b;
	starting_pos += pos_a;
      }	
    }
  } 

  stream_a.close();
  stream_b.close();
  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: Patchfile::apply
//       Access: Public
//  Description: Apply the patch to the file (original file and
//		 patch are destroyed in the process).
////////////////////////////////////////////////////////////////////
bool Patchfile::
apply(Filename &patch, Filename &file) {
  // Open the patch file for read
  ifstream patch_stream;
  patch.set_binary();
  if (!patch.open_read(patch_stream)) {
    express_cat.error()
      << "Patchfile::apply() - Failed to open file: " << patch << endl;
    return false;
  }

  // Open the file for read
  ifstream file_stream;
  file.set_binary();
  if (!file.open_read(file_stream)) {
    express_cat.error()
      << "Patchfile::apply() - Failed to open file: " << file << endl;
    return false;
  }

  // Determine the size of the orig file
  file_stream.seekg(0, ios::end);
  int orig_file_length = file_stream.tellg();
  file_stream.seekg(0, ios::beg);

#if 0
  // Open the file for write 
  ofstream write_stream;
  if (!_temp_file_name.open_write(write_stream)) {
    express_cat.error()
      << "Patchfile::apply() - Failed to open file: " << _temp_file_name 
      << endl;
    return false;
  }
#endif

  // Open the file for write
  ofstream write_stream;
  Filename mofile = "patcher_temp_file";
  mofile.set_binary();
  if (!mofile.open_write(write_stream)) {
    express_cat.error()
      << "Patchfile::apply() - Failed to open file: " << mofile << endl;
    return false;
  }

  // Determine the size of the patch file
  patch_stream.seekg(0, ios::end);
  int patch_file_length = patch_stream.tellg();
  patch_stream.seekg(0, ios::beg);

  // Make sure the patch file is valid
  nassertr(_buffer->get_length() >= _header_length_length, false);
  patch_stream.read(_buffer->_buffer, _header_length_length);
  _datagram.clear();
  _datagram.append_data(_buffer->_buffer, _header_length_length);
  DatagramIterator di(_datagram);
  uint magic_number = di.get_uint32();
  if (magic_number != _magic_number) {
    express_cat.error()
      << "Patchfile::apply() - invalid patch file: " << patch << endl;
    return false;
  }
  int name_length = di.get_int32();
  nassertr(_buffer->get_length() >= name_length, false);
  patch_stream.read(_buffer->_buffer, name_length);
  _datagram.clear();
  _datagram.append_data(_buffer->_buffer, name_length);
  DatagramIterator di2(_datagram);
  string name = di2.extract_bytes(name_length);
  if (name != file.get_basename()) {
    express_cat.error()
      << "Patchfile::apply() - patch intended for file: " << name
      << ", not file: " << file << endl;
    return false;
  }
  patch_file_length -= (_header_length_length + name_length);

  express_cat.debug()
    << "Patchfile::apply() - valid patchfile for file: " << name << endl;

  // Now patch the file using the given buffer
  int buflen = _buffer->get_length();
  int total_orig_bytes = 0;
  int total_patch_bytes = 0;
  int read_bytes;
  while (total_orig_bytes < orig_file_length) {
  
    // Read an entry
    Entry entry;
    nassertr(buflen >= entry._len, false); 
    patch_stream.read(_buffer->_buffer, entry._len);
    _datagram.clear();
    _datagram.append_data(_buffer->_buffer, entry._len);
    DatagramIterator di3(_datagram);
    entry._pos = di3.get_int32();
    entry._n = di3.get_int32();
    entry._m = di3.get_int32();

    express_cat.debug()
      << "Patchfile::apply() - read an entry: pos: " << entry._pos
      << " n: " << entry._n << " m: " << entry._m << endl;

    total_patch_bytes += entry._len;

    // Write original file until we hit a difference
    while (total_orig_bytes < entry._pos) { 
      int next_diff = entry._pos - total_orig_bytes;
      read_bytes = (next_diff < buflen) ? next_diff : buflen;
      file_stream.read(_buffer->_buffer, read_bytes); 
      total_orig_bytes += read_bytes;
      write_stream.write(_buffer->_buffer, read_bytes);
    }

    // Now skip the next "n" bytes of the orig file
    file_stream.seekg(total_orig_bytes + entry._n, ios::beg);
    total_orig_bytes += entry._n;

    // Now write the patch file until we're done with the current entry
    int patch_bytes = 0; 
    while (patch_bytes < entry._m) {
      int patch_end = entry._m - patch_bytes;
      read_bytes = (patch_end < buflen) ? patch_end : buflen;
      patch_stream.read(_buffer->_buffer, read_bytes);
      patch_bytes += read_bytes;
      write_stream.write(_buffer->_buffer, read_bytes);
    }

    total_patch_bytes += patch_bytes;

    // If the patch file is empty, write any remaining original file
    // bytes to the out file
    if (total_patch_bytes >= patch_file_length) {
      while (total_orig_bytes < orig_file_length) {
	int file_end = orig_file_length - total_orig_bytes;
	read_bytes = (file_end < buflen) ? file_end : buflen;
	file_stream.read(_buffer->_buffer, read_bytes);
	total_orig_bytes += read_bytes;
	write_stream.write(_buffer->_buffer, read_bytes);
      }
    }

  }

  patch_stream.close();
  file_stream.close();
  write_stream.close();
  patch.unlink();
  file.unlink();
  if (!mofile.rename_to(file)) {
    express_cat.error()
      << "Patchfile::apply() failed to rename temp file to: " << file
      << endl;
    return false;
  }
  
#if 0
  if (!_temp_file_name.rename_to(file)) {
    express_cat.error()
      << "Patchfile::apply() failed to rename temp file to: " << file
      << endl;
    return false;
  }
#endif
  
  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: Patchfile::reset
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
void Patchfile::
reset(void) {
  _datagram.clear();
  _current_entry = (Entry *)0L;
}

////////////////////////////////////////////////////////////////////
//     Function: Patchfile::find_longest_sequence
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
int Patchfile::
find_longest_sequence(Filename &infile, int &pos, int &len) const { 
  // Open the file for read
  ifstream read_stream;
  infile.set_binary();
  if (!infile.open_read(read_stream)) {
    express_cat.error()
      << "Patchfile::find_longest_sequence() - Failed to open file: " 
      << infile << endl;
    return 0;
  }

  // Determine file length
  read_stream.seekg(0, ios::end);
  len = read_stream.tellg();
  char *buffer = new char[len];
  read_stream.seekg(0, ios::beg);
  read_stream.read(buffer, len);

  pos = 0;
  char holder = 0;
  int seq_len;
  int longest_seq_len = 0;
  for (int i = 0; i < len; i++) {
    if (buffer[i] != holder) {
      holder = buffer[i];
      seq_len = 0;
    } else {
      if (++seq_len > longest_seq_len) {
	longest_seq_len = seq_len;
	pos = i;
      }
    }
  } 

  read_stream.close();

  return longest_seq_len;
}

////////////////////////////////////////////////////////////////////
//     Function: Patchfile::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
Patchfile::Entry::
Entry(void) {
  _len = 3 * sizeof(PN_int32);
  _buffer = (char *)0L;
}
