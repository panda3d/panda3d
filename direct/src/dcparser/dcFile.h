// Filename: dcFile.h
// Created by:  drose (05Oct00)
// 
////////////////////////////////////////////////////////////////////

#ifndef DCFILE_H
#define DCFILE_H

#include "dcbase.h"
#include "dcClass.h"

#include <vector>
#include <map>

class HashGenerator;

////////////////////////////////////////////////////////////////////
// 	 Class : DCFile
// Description : Represents the complete list of Distributed Class
//               descriptions as read from a .dc file.
////////////////////////////////////////////////////////////////////
class EXPCL_DIRECT DCFile {
PUBLISHED:
  DCFile();
  ~DCFile();

  bool read(Filename filename);
  bool read(istream &in, const string &filename = string());

  bool write(Filename filename) const;
  bool write(ostream &out, const string &filename = string()) const;

  int get_num_classes();
  DCClass *get_class(int n);

  DCClass *get_class_by_name(const string &name);

  unsigned long get_hash() const;

public:
  void generate_hash(HashGenerator &hash) const;
  bool add_class(DCClass *dclass);

public:
  // This vector is the primary interface to the distributed classes
  // read from the file.
  typedef vector<DCClass *> Classes;
  Classes _classes;

public:
  // This map is built up during parsing for the convenience of the parser.
  typedef map<string, DCClass *> ClassesByName;
  ClassesByName _classes_by_name;
};

#endif


