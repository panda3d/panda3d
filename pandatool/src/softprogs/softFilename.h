// Filename: softFilename.h
// Created by:  drose (10Nov00)
// 
////////////////////////////////////////////////////////////////////

#ifndef SOFTFILENAME_H
#define SOFTFILENAME_H

#include <pandatoolbase.h>

////////////////////////////////////////////////////////////////////
// 	 Class : SoftFilename
// Description : This encapsulates a SoftImage versioned filename, of
//               the form base.v-v.ext: it consists of a base, a major
//               and minor version number, and an optional extension.
////////////////////////////////////////////////////////////////////
class SoftFilename {
public:
  SoftFilename(const string &filename);
  SoftFilename(const SoftFilename &copy);
  void operator = (const SoftFilename &copy);

  const string &get_filename() const;
  bool has_version() const;

  string get_1_0_filename() const;

  const string &get_base() const;
  int get_major() const;
  int get_minor() const;
  const string &get_extension() const;
  string get_non_extension() const;

  bool is_1_0() const;

  bool is_same_file(const SoftFilename &other) const;
  bool operator < (const SoftFilename &other) const;

private:
  string _filename;
  bool _has_version;
  string _base;
  int _major;
  int _minor;
  string _ext;
};

#endif
