// Filename: loaderFileTypeRegistry.h
// Created by:  drose (20Jun00)
// 
////////////////////////////////////////////////////////////////////

#ifndef LOADERFILETYPEREGISTRY_H
#define LOADERFILETYPEREGISTRY_H

#include <pandabase.h>

#include <vector>
#include <map>

class LoaderFileType;
class Filename;

////////////////////////////////////////////////////////////////////
//       Class : LoaderFileTypeRegistry
// Description : This class maintains the set of all known
//               LoaderFileTypes in the universe.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA LoaderFileTypeRegistry {
protected:
  LoaderFileTypeRegistry();

public:
  ~LoaderFileTypeRegistry();

  static LoaderFileTypeRegistry *get_ptr();

  int get_num_types() const;
  LoaderFileType *get_type(int n) const;

  LoaderFileType *get_type_from_extension(const string &extension) const;

  void write_types(ostream &out, int indent_level = 0) const;

  void register_type(LoaderFileType *type);

private:
  typedef vector<LoaderFileType *> Types;
  Types _types;

  typedef map<string, LoaderFileType *> Extensions;
  Extensions _extensions;

  static LoaderFileTypeRegistry *_global_ptr;
};

#endif

