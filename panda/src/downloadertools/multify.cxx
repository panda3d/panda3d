// Filename: multify.cxx
// Created by:  
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://www.panda3d.org/license.txt .
//
// To contact the maintainers of this program write to
// panda3d@yahoogroups.com .
//
////////////////////////////////////////////////////////////////////

#include "pandabase.h"
#ifndef HAVE_GETOPT
  #include "gnu_getopt.h"
#else
  #include <getopt.h>
#endif
#include "multifile.h"
#include "filename.h"
#include <stdio.h>


bool create = false;      // -c
bool append = false;      // -r
bool list = false;        // -t
bool extract = false;     // -x
bool verbose = false;     // -v
Filename multifile_name;  // -f
bool got_multifile_name = false;
bool to_stdout = false;   // -O
Filename chdir_to;        // -C
bool got_chdir_to = false;
size_t scale_factor = 0;  // -F

void 
usage() {
  cerr << "Usage: multify -[c|r|t|x] -f <multifile_name> [options] <subfile_name> ...\n";
}

void 
help() {
  cerr << "multify is used to store and extract files from a Panda Multifile.\n"
       << "This is similar to a tar or zip file in that it is an archive file that\n"
       << "contains a number of subfiles that may later be extracted.\n\n"

       << "The command-line options for multify are designed to be similar to those\n"
       << "for tar, the traditional Unix archiver utility.\n\n";
  usage();
}

bool
is_named(const string &subfile_name, int argc, char *argv[]) {
  // Returns true if the indicated subfile appears on the list of
  // files named on the command line.
  if (argc < 2) {
    // No named files; everything is listed.
    return true;
  }

  for (int i = 1; i < argc; i++) {
    if (subfile_name == argv[i]) {
      return true;
    }
  }

  return false;
}

bool
add_directory(Multifile &multifile, const Filename &directory_name) {
  vector_string files;
  if (!directory_name.scan_directory(files)) {
    cerr << "Unable to scan directory " << directory_name << "\n";
    return false;
  }

  bool okflag = true;

  vector_string::const_iterator fi;
  for (fi = files.begin(); fi != files.end(); ++fi) {
    Filename subfile_name(directory_name, (*fi));
    if (subfile_name.is_directory()) {
      okflag = add_directory(multifile, subfile_name);

    } else if (!subfile_name.exists()) {
      cerr << "Not found: " << subfile_name << "\n";
      okflag = false;

    } else {
      string new_subfile_name =
        multifile.add_subfile(subfile_name, subfile_name);
      if (new_subfile_name.empty()) {
        cerr << "Unable to add " << subfile_name << ".\n";
        okflag = false;
      } else {
        if (verbose) {
          cout << new_subfile_name << "\n";
        }
      }
    }
  }

  return okflag;
}

bool
add_files(int argc, char *argv[]) {
  Multifile multifile;
  if (append) {
    if (!multifile.open_read_write(multifile_name)) {
      cerr << "Unable to open " << multifile_name << " for updating.\n";
      return false;
    }
  } else {
    if (!multifile.open_write(multifile_name)) {
      cerr << "Unable to open " << multifile_name << " for writing.\n";
      return false;
    }
  }

  if (scale_factor != 0 && scale_factor != multifile.get_scale_factor()) {
    cerr << "Setting scale factor to " << scale_factor << "\n";
    multifile.set_scale_factor(scale_factor);
  }

  bool okflag = true;
  for (int i = 1; i < argc; i++) {
    Filename subfile_name = Filename::from_os_specific(argv[i]);
    if (subfile_name.is_directory()) {
      if (!add_directory(multifile, subfile_name)) {
        okflag = false;
      }

    } else if (!subfile_name.exists()) {
      cerr << "Not found: " << subfile_name << "\n";
      okflag = false;

    } else {
      string new_subfile_name =
        multifile.add_subfile(subfile_name, subfile_name);
      if (new_subfile_name.empty()) {
        cerr << "Unable to add " << subfile_name << ".\n";
        okflag = false;
      } else {
        if (verbose) {
          cout << new_subfile_name << "\n";
        }
      }
    }
  }

  if (multifile.needs_repack()) {
    if (!multifile.repack()) {
      cerr << "Failed to write " << multifile_name << ".\n";
      okflag = false;
    }
  } else {
    if (!multifile.flush()) {
      cerr << "Failed to write " << multifile_name << ".\n";
      okflag = false;
    }
  }

  return okflag;
}

bool
extract_files(int argc, char *argv[]) {
  if (!multifile_name.exists()) {
    cerr << multifile_name << " not found.\n";
    return false;
  }
  Multifile multifile;
  if (!multifile.open_read(multifile_name)) {
    cerr << "Unable to open " << multifile_name << " for reading.\n";
    return false;
  }

  int num_subfiles = multifile.get_num_subfiles();

  for (int i = 0; i < num_subfiles; i++) {
    string subfile_name = multifile.get_subfile_name(i);
    if (is_named(subfile_name, argc, argv)) {
      Filename filename = subfile_name;
      if (got_chdir_to) {
        filename = Filename(chdir_to, subfile_name);
      }
      if (to_stdout) {
        if (verbose) {
          cerr << filename << "\n";
        }
        multifile.extract_subfile_to(i, cout);
      } else {
        if (verbose) {
          cout << filename << "\n";
        }
        multifile.extract_subfile(i, filename);
      }
    }
  }

  return true;
}

bool
list_files(int argc, char *argv[]) {
  if (!multifile_name.exists()) {
    cerr << multifile_name << " not found.\n";
    return false;
  }
  Multifile multifile;
  if (!multifile.open_read(multifile_name)) {
    cerr << "Unable to open " << multifile_name << " for reading.\n";
    return false;
  }

  int num_subfiles = multifile.get_num_subfiles();
  
  if (verbose) {
    cout << num_subfiles << " subfiles:\n" << flush;
    for (int i = 0; i < num_subfiles; i++) {
      string subfile_name = multifile.get_subfile_name(i);
      if (is_named(subfile_name, argc, argv)) {
        printf("%12d  %s\n", 
               multifile.get_subfile_length(i),
               subfile_name.c_str());
      }
    }
    fflush(stdout);
    if (multifile.get_scale_factor() != 1) {
      cout << "Scale factor is " << multifile.get_scale_factor() << "\n";
    }
    if (multifile.needs_repack()) {
      cout << "Multifile needs to be repacked.\n";
    }
  } else {
    for (int i = 0; i < num_subfiles; i++) {
      string subfile_name = multifile.get_subfile_name(i);
      if (is_named(subfile_name, argc, argv)) {
        cout << subfile_name << "\n";
      }
    }
  }

  return true;
}

int
main(int argc, char *argv[]) {
  if (argc < 2) {
    usage();
    return 1;
  }

  // To emulate tar, we assume an implicit hyphen in front of the
  // first argument if there is not one already.
  if (argc >= 2) {
    if (*argv[1] != '-' && *argv[1] != '\0') {
      char *new_arg = new char[strlen(argv[1]) + 2];
      new_arg[0] = '-';
      strcpy(new_arg + 1, argv[1]);
      argv[1] = new_arg;
    }
  }

  extern char *optarg;
  extern int optind;
  static const char *optflags = "crtxvf:OC:F:h";
  int flag = getopt(argc, argv, optflags);
  Filename rel_path;
  while (flag != EOF) {
    switch (flag) {
    case 'c':
      create = true;
      break;
    case 'r':
      append = true;
      break;
    case 't':
      list = true;
      break;
    case 'x':
      extract = true;
      break;
    case 'v':
      verbose = true;
      break;
    case 'f':
      multifile_name = Filename::from_os_specific(optarg);
      got_multifile_name = true;
      break;
    case 'C':
      chdir_to = Filename::from_os_specific(optarg);
      got_chdir_to = true;
      break;
    case 'O':
      to_stdout = true;
      break;
    case 'F':
      {
        char *endptr;
        scale_factor = strtol(optarg, &endptr, 10);
        if (*endptr != '\0') {
          cerr << "Invalid integer: " << optarg << "\n";
          usage();
          return 1;
        }
        if (scale_factor == 0) {
          cerr << "Scale factor must be nonzero.\n";
          usage();
          return 1;
        }
      }
      break;

    case 'h':
      help();
      return 1;
    case '?':
      usage();
      return 1;
    default:
      cerr << "Unhandled switch: " << flag << endl;
      break;
    }
    flag = getopt(argc, argv, optflags);
  }
  argc -= (optind - 1);
  argv += (optind - 1);

  // We should have exactly one of these options.
  if ((create?1:0) + (append?1:0) + (list?1:0) + (extract?1:0) != 1) {
    cerr << "Exactly one of -c, -r, -t, -x must be specified.\n";
    usage();
    return 1;
  }

  if (!got_multifile_name) {
    if (argc <= 1) {
      usage();
      return 1;
    }

    // For now, we allow -f to be omitted, and use the first argument
    // as the archive name, for backward compatibility.  Later we will
    // remove this.
    multifile_name = Filename::from_os_specific(argv[1]);
    cerr << "Warning: using " << multifile_name
         << " as archive name.  Use -f in the future to specify this.\n";
    argc--;
    argv++;
  }

  bool okflag = true;
  if (create || append) {
    okflag = add_files(argc, argv);
  } else if (extract) {
    okflag = extract_files(argc, argv);
  } else { // list
    okflag = list_files(argc, argv);
  }

  if (okflag) {
    return 0;
  } else {
    return 1;
  }
}
