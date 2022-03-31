/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file dcparse.cxx
 * @author drose
 * @date 2000-10-05
 */

#include "dcbase.h"
#include "dcFile.h"
#include "dcClass.h"
#include "dcTypedef.h"
#include "memoryUsage.h"
#include "indent.h"
#include "panda_getopt.h"

using std::cerr;
using std::cout;

void
usage() {
  cerr <<
    "\n"
    "Usage:\n\n"
    "dcparse [options]  [file1 file2 ...]\n"
    "dcparse -h\n\n";
}

void
help() {
  usage();
  cerr <<
    "This program reads one or more DC files, which are used to describe the\n"
    "communication channels in the distributed class system.  By default,\n"
    "the file(s) are read and concatenated, and a single hash code is printed\n"
    "corresponding to the file's contents.\n\n"

    "Options:\n\n"

    "  -v Writes a complete parseable version of the file to standard\n"
    "     output instead of printing a hash code.\n\n"

    "  -b Writes a brief parseable version of the file instead of a full\n"
    "     version.  This is semantically the same as the output produced\n"
    "     the above -v option--reading it would produce exactly the same\n"
    "     results--but it is designed to be slightly obfuscated.  The\n"
    "     comments and parameter names are not included.\n\n"

    "  -c Write a list of class names, showing the inheritance hierarchy.\n"
    "     Some class names will be listed twice in the presence of multiple\n"
    "     inheritance.\n\n"

    "  -f Write a complete list of field names available for each class,\n"
    "     including all inherited fields.\n\n";
}

void
write_class_hierarchy(int indent_level, const DCFile &file,
                      const DCClass *this_dclass) {
  indent(cout, indent_level)
    << this_dclass->get_name() << "\n";

  int num_classes = file.get_num_classes();
  for (int i = 0; i < num_classes; ++i) {
    const DCClass *dclass = file.get_class(i);
    bool is_my_child = false;
    int num_parents = dclass->get_num_parents();
    for (int j = 0; j < num_parents && !is_my_child; ++j) {
      is_my_child = (dclass->get_parent(j) == this_dclass);
    }

    if (is_my_child) {
      write_class_hierarchy(indent_level + 2, file, dclass);
    }
  }
}

void
write_class_hierarchy(const DCFile &file) {
  int num_classes = file.get_num_classes();
  for (int i = 0; i < num_classes; ++i) {
    const DCClass *dclass = file.get_class(i);
    if (dclass->get_num_parents() == 0) {
      write_class_hierarchy(0, file, dclass);
      cout << "\n";
    }
  }
}

void
write_complete_field_list(const DCFile &file) {
  int num_classes = file.get_num_classes();
  for (int i = 0; i < num_classes; ++i) {
    const DCClass *dclass = file.get_class(i);
    cout << "\n" << dclass->get_name() << "\n";
    int num_inherited_fields = dclass->get_num_inherited_fields();
    for (int j = 0; j < num_inherited_fields; ++j) {
      const DCField *field = dclass->get_inherited_field(j);
      cout << "  ";
      if (field->get_class() != dclass) {
        cout << field->get_class()->get_name() << "::";
      }
      cout << field->get_name();
      if (field->as_atomic_field() != nullptr ||
          field->as_molecular_field() != nullptr) {
        // It's a "method".
        cout << "()";
      }
      field->output_keywords(cout);
      cout << "\n";
    }
  }
}

int
main(int argc, char *argv[]) {
  // extern char *optarg;
  extern int optind;
  const char *optstr = "bvcfh";

  bool dump_verbose = false;
  bool dump_brief = false;
  bool dump_classes = false;
  bool dump_fields = false;

  int flag = getopt(argc, argv, optstr);

  while (flag != EOF) {
    switch (flag) {
    case 'b':
      dump_brief = true;
      break;

    case 'v':
      dump_verbose = true;
      break;

    case 'c':
      dump_classes = true;
      break;

    case 'f':
      dump_fields = true;
      break;

    case 'h':
      help();
      exit(1);

    default:
      exit(1);
    }
    flag = getopt(argc, argv, optstr);
  }

  argc -= (optind-1);
  argv += (optind-1);

  if (argc < 2) {
    usage();
    exit(1);
  }

  DCFile file;
  for (int i = 1; i < argc; i++) {
    if (!file.read(argv[i])) {
      return (1);
    }
  }

  if (!file.all_objects_valid() && !dump_brief) {
    cerr << "File is incomplete.  The following objects are undefined:\n";

    int num_typedefs = file.get_num_typedefs();
    int i;
    for (i = 0; i < num_typedefs; i++) {
      DCTypedef *dtypedef = file.get_typedef(i);
      if (dtypedef->is_bogus_typedef()) {
        cerr << "  " << dtypedef->get_name() << "\n";
      }
    }

    int num_classes = file.get_num_classes();
    for (i = 0; i < num_classes; i++) {
      DCClass *dclass = file.get_class(i);
      if (dclass->is_bogus_class()) {
        cerr << "  " << dclass->get_name() << "\n";
      }
    }

    return 1;
  }

  if (dump_verbose || dump_brief) {
    if (!file.write(cout, dump_brief)) {
      return 1;
    }

  } else if (dump_classes) {
    write_class_hierarchy(file);

  } else if (dump_fields) {
    write_complete_field_list(file);

  } else {
    unsigned long hash = file.get_hash();
    cerr << "File hash is " << hash << " (signed " << (long)hash << ")\n";
  }

#ifdef DO_MEMORY_USAGE
  if (MemoryUsage::is_tracking()) {
    file.clear();
    MemoryUsage::show_current_types();
    for (int i = 1; i < argc; i++) {
      file.read(argv[i]);
    }
    file.clear();
    MemoryUsage::show_current_types();
  }
#endif

  return 0;
}
