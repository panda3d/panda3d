// Filename: pnmFileTypeRegistry.h
// Created by:  drose (15Jun00)
// 
////////////////////////////////////////////////////////////////////

#ifndef PNMFILETYPEREGISTRY_H
#define PNMFILETYPEREGISTRY_H

#include <pandabase.h>

#include <typeHandle.h>

class PNMFileType;

////////////////////////////////////////////////////////////////////
// 	 Class : PNMFileTypeRegistry
// Description : This class maintains the set of all known
//               PNMFileTypes in the universe.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA PNMFileTypeRegistry {
protected:
  PNMFileTypeRegistry();

public:
  ~PNMFileTypeRegistry();

  static PNMFileTypeRegistry *get_ptr();

  int get_num_types() const;
  PNMFileType *get_type(int n) const;

  PNMFileType *get_type_from_extension(const string &filename) const;
  PNMFileType *get_type_from_magic_number(const string &magic_number) const;
  PNMFileType *get_type_by_handle(TypeHandle handle) const;

  void write_types(ostream &out, int indent_level = 0) const;

  void register_type(PNMFileType *type);

private:
  void sort_preferences();

  typedef vector<PNMFileType *> Types;
  Types _types;

  typedef map<string, Types> Extensions;
  Extensions _extensions;

  typedef map<TypeHandle, PNMFileType *> Handles;
  Handles _handles;

  bool _requires_sort;

  static PNMFileTypeRegistry *_global_ptr;
};

#endif

