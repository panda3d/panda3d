// Filename: p3dMultifileReader.h
// Created by:  drose (15Jun09)
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

#ifndef P3DMULTIFILEREADER_H
#define P3DMULTIFILEREADER_H

#include "p3d_plugin_common.h"
#include "p3dInstanceManager.h"  // for openssl
#include "p3dPackage.h"

////////////////////////////////////////////////////////////////////
//       Class : P3DMultifileReader
// Description : A way-simple implementation of Panda's multifile
//               reader.  See panda/src/express/multifile.cxx for a
//               full description of the binary format.  This
//               implementation doesn't support per-subfile
//               compression or encryption.
////////////////////////////////////////////////////////////////////
class P3DMultifileReader {
public:
  P3DMultifileReader();
  bool open_read(const string &pathname, const int &offset = 0);
  inline bool is_open() const;
  void close();

  bool extract_all(const string &to_dir, P3DPackage *package, 
                   P3DPackage::InstallStepThreaded *step);

  bool extract_one(ostream &out, const string &filename);

  class CertRecord {
  public:
    inline CertRecord(X509 *cert);
    inline CertRecord(const CertRecord &copy);
    inline ~CertRecord();
    X509 *_cert;
  };
  typedef vector<CertRecord> CertChain;
  int get_num_signatures() const;
  const CertChain &get_signature(int n) const;

private:
  class Subfile;

  bool read_header(const string &pathname);
  bool read_index();
  bool extract_subfile(ostream &out, const Subfile &s);

  void check_signatures();

  inline unsigned int read_uint16();
  inline unsigned int read_uint32();

  enum SubfileFlags {
    // If any of these bits are set, we can't read the subfile.
    SF_ignore         = 0x003f,

    // These bits are also relevant, and specifically so.
    SF_compressed     = 0x0008,
    SF_encrypted      = 0x0010,
    SF_signature      = 0x0020,
  };

  class Subfile {
  public:
    inline size_t get_last_byte_pos() const;

    string _filename;
    size_t _index_start;
    size_t _index_length;
    size_t _data_start;
    size_t _data_length;
    size_t _timestamp;
  };

  ifstream _in;
  bool _is_open;
  int _read_offset;

  typedef vector<Subfile> Subfiles;
  Subfiles _subfiles;
  Subfiles _cert_special;
  size_t _last_data_byte;

  typedef vector<CertChain> Certificates;
  Certificates _signatures;

  static const char _header[];
  static const size_t _header_size;
  static const int _current_major_ver;
  static const int _current_minor_ver;
};

#include "p3dMultifileReader.I"

#endif
