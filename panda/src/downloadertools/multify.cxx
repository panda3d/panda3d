// Filename: multify.cxx
// Created by:  
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001 - 2004, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://etc.cmu.edu/panda3d/docs/license/ .
//
// To contact the maintainers of this program write to
// panda3d-general@lists.sourceforge.net .
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
#include "pset.h"
#include <stdio.h>


bool create = false;           // -c
bool append = false;           // -r
bool update = false;           // -u
bool list = false;             // -t
bool extract = false;          // -x
bool verbose = false;          // -v
bool compress = false;         // -z
int default_compression_level = 6;
Filename multifile_name;       // -f
bool got_multifile_name = false;
bool to_stdout = false;        // -O
bool encryption_flag = false;  // -e
string password;               // -p
bool got_password = false;
Filename chdir_to;             // -C
bool got_chdir_to = false;
size_t scale_factor = 0;       // -F
pset<string> dont_compress;    // -Z

// Default extensions not to compress.  May be overridden with -Z.
string dont_compress_str = "jpg,mp3";

void 
usage() {
  cerr <<
    "Usage: multify -[c|r|u|t|x] -f <multifile_name> [options] <subfile_name> ...\n";
}

void 
help() {
  usage();
  cerr << "\n"
    "multify is used to store and extract files from a Panda Multifile.\n"
    "This is similar to a tar or zip file in that it is an archive file that\n"
    "contains a number of subfiles that may later be extracted.\n\n"

    "Panda's VirtualFileSystem is capable of mounting Multifiles for direct\n"
    "access to the subfiles contained within without having to extract them\n"
    "out to independent files first.\n\n"

    "The command-line options for multify are designed to be similar to those\n"
    "for tar, the traditional Unix archiver utility.\n\n"

    "Options:\n\n"
    
    "  You must specify exactly one of the following command switches:\n\n"

    "  -c\n"
    "      Create a new Multifile archive.  Subfiles named on the command line\n"
    "      will be added to the new Multifile.  If the Multifile already exists,\n"
    "      it is first removed.\n\n"

    "  -r\n"
    "      Rewrite an existing Multifile archive.  Subfiles named on the command\n"
    "      line will be added to the Multifile or will replace subfiles within\n"
    "      the Multifile with the same name.  The Multifile will be repacked\n"
    "      after completion, even if no Subfiles were added.\n\n"

    "  -u\n"
    "      Update an existing Multifile archive.  This is similar to -r, except\n"
    "      that files are compared byte-for-byte with their corresponding files\n"
    "      in the archive first.  If they have not changed, the multifile is not\n"
    "      modified (other than to repack it if necessary).\n\n"

    "  -t\n"
    "      List the contents of an existing Multifile.  With -v, this shows\n"
    "      the size of each Subfile and its compression ratio, if compressed.\n\n"

    "  -x\n"
    "      Extract the contents of an existing Multifile.  The Subfiles named on\n"
    "      the command line, or all Subfiles if nothing is named, are extracted\n"
    "      into the current directory or into whichever directory is specified\n"
    "      with -C.\n\n\n"

    
    "  You must always specify the following switch:\n\n"

    "  -f <multifile_name>\n"
    "      Names the Multifile that will be operated on.\n\n\n"

    "  The remaining switches are optional:\n\n"

    "  -v\n"
    "      Run verbosely.  In -c, -r, or -x mode, list each file as it is\n"
    "      written or extracted.  In -t mode, list more information about each\n"
    "      file.\n\n"

    "  -z\n"
    "      Compress subfiles as they are written to the Multifile.  Unlike tar\n"
    "      (but like zip), subfiles are compressed individually, instead of the\n"
    "      entire archive being compressed as one stream.  It is not necessary\n"
    "      to specify -z when extracting compressed subfiles; they will always be\n"
    "      decompressed automatically.  Also see -Z, which restricts which\n"
    "      subfiles will be compressed based on the filename extension.\n\n"

    "  -e\n"
    "      Encrypt subfiles as they are written to the Multifile using the password\n"
    "      specified with -p, below.  Subfiles are encrypted individually, rather\n"
    "      than encrypting the entire multifile, and different subfiles may be\n"
    "      encrypted using different passwords (although this requires running\n"
    "      multify multiple times).  It is not possible to encrypt the multifile's\n"
    "      table of contents using this interface, but see the pencrypt program to\n"
    "      encrypt the entire multifile after it has been generated).\n\n"


    "  -p \"password\"\n"
    "      Specifies the password to encrypt or decrypt subfiles.  If this is not\n"
    "      specified, and passwords are required, the user will be prompted from\n"
    "      standard input.\n\n"

    "  -F <scale_factor>\n"
    "      Specify a Multifile scale factor.  This is only necessary to support\n"
    "      Multifiles that will exceed 4GB in size.  The default scale factor is\n"
    "      1, which should be sufficient for almost any application, but the total\n"
    "      size of the Multifile will be limited to 4GB * scale_factor.  The size\n"
    "      of individual subfiles may not exceed 4GB in any case.\n\n"

    "  -C <extract_dir>\n"

    "      With -x, change to the named directory before extracting files;\n"
    "      that is, extract subfiles into the named directory.\n\n"

    "  -O\n"
    "      With -x, extract subfiles to standard output instead of to disk.\n\n"
    "  -Z <extension_list>\n"
    "      Specify a comma-separated list of filename extensions that represent\n"
    "      files that are not to be compressed.  The default if this is omitted is\n"
    "      \"" << dont_compress_str << "\".  Specify -Z \"\" (be sure to include the space) to allow\n"
    "      all files to be compressed.\n\n"

    "  -1 .. -9\n"
    "      Specify the compression level when -z is in effect.  Larger numbers\n"
    "      generate slightly smaller files, but compression takes longer.  The\n"
    "      default is -" << default_compression_level << ".\n\n";
}

const string &
get_password() {
  if (!got_password) {
    cerr << "Enter password: ";
    getline(cin, password);
    got_password = true;
  }

  return password;
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

int
get_compression_level(const Filename &subfile_name) {
  // Returns the appropriate compression level for the named file.
  if (!compress) {
    // Don't compress anything.
    return 0;
  }

  string ext = subfile_name.get_extension();
  if (dont_compress.find(ext) != dont_compress.end()) {
    // This extension is listed on the -Z parameter list; don't
    // compress it.
    return 0;
  }

  // Go ahead and compress this file.
  return default_compression_level;
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
      if (!add_directory(multifile, subfile_name)) {
        okflag = false;
      }

    } else if (!subfile_name.exists()) {
      cerr << "Not found: " << subfile_name << "\n";
      okflag = false;

    } else {
      string new_subfile_name;
      if (update) {
        new_subfile_name = multifile.update_subfile
          (subfile_name, subfile_name, get_compression_level(subfile_name));
      } else {
        new_subfile_name = multifile.add_subfile
          (subfile_name, subfile_name, get_compression_level(subfile_name));
      }
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
  if (append || update) {
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

  if (encryption_flag) {
    multifile.set_encryption_flag(true);
    multifile.set_encryption_password(get_password());
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
      string new_subfile_name;
      if (update) {
        new_subfile_name = multifile.update_subfile
          (subfile_name, subfile_name, get_compression_level(subfile_name));
      } else {
        new_subfile_name = multifile.add_subfile
          (subfile_name, subfile_name, get_compression_level(subfile_name));
      }
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

  // First, check to see whether any of the named subfiles have been
  // encrypted.  If any have, we may need to prompt the user to enter
  // a password before we can extract them.
  int i;
  bool any_encrypted = false;
  for (i = 0; i < num_subfiles && !any_encrypted; i++) {
    string subfile_name = multifile.get_subfile_name(i);
    if (is_named(subfile_name, argc, argv)) {
      if (multifile.is_subfile_encrypted(i)) {
        any_encrypted = true;
      }
    }
  }

  if (any_encrypted) {
    multifile.set_encryption_password(get_password());
  }

  // Now walk back through the list and this time do the extraction.
  for (i = 0; i < num_subfiles; i++) {
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
        char encrypted_symbol = ' ';
        if (multifile.is_subfile_encrypted(i)) {
          encrypted_symbol = 'e';
        }
        if (multifile.is_subfile_compressed(i)) {
          size_t orig_length = multifile.get_subfile_length(i);
          size_t internal_length = multifile.get_subfile_internal_length(i);
          double ratio = 1.0;
          if (orig_length != 0) {
            ratio = (double)internal_length / (double)orig_length;
          }
          if (ratio > 1.0) {
            printf("%12d worse %c %s\n",
                   multifile.get_subfile_length(i),
                   encrypted_symbol,
                   subfile_name.c_str());
          } else {
            printf("%12d  %3.0f%% %c %s\n",
                   multifile.get_subfile_length(i),
                   100.0 - ratio * 100.0, encrypted_symbol,
                   subfile_name.c_str());
          }
        } else {
          printf("%12d       %c %s\n", 
                 multifile.get_subfile_length(i),
                 encrypted_symbol, subfile_name.c_str());
        }
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

void
tokenize_extensions(const string &str, pset<string> &extensions) {
  size_t p = 0;
  while (p < str.length()) {
    size_t q = str.find_first_of(",", p);
    if (q == string::npos) {
      extensions.insert(str.substr(p));
      return;
    }
    extensions.insert(str.substr(p, q - p));
    p = q + 1;
  }
  extensions.insert(string());
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
  static const char *optflags = "crutxvz123456789Z:f:OC:ep:F:h";
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
    case 'u':
      update = true;
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
    case 'z':
      compress = true;
      break;
    case '1':
      default_compression_level = 1;
      compress = true;
      break;
    case '2':
      default_compression_level = 2;
      compress = true;
      break;
    case '3':
      default_compression_level = 3;
      compress = true;
      break;
    case '4':
      default_compression_level = 4;
      compress = true;
      break;
    case '5':
      default_compression_level = 5;
      compress = true;
      break;
    case '6':
      default_compression_level = 6;
      compress = true;
      break;
    case '7':
      default_compression_level = 7;
      compress = true;
      break;
    case '8':
      default_compression_level = 8;
      compress = true;
      break;
    case '9':
      default_compression_level = 9;
      compress = true;
      break;
    case 'Z':
      dont_compress_str = optarg;
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
    case 'e':
      encryption_flag = true;
      break;
    case 'p':
      password = optarg;
      got_password = true;
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
  if ((create?1:0) + (append?1:0) + (update?1:0) + (list?1:0) + (extract?1:0) != 1) {
    cerr << "Exactly one of -c, -r, -u, -t, -x must be specified.\n";
    usage();
    return 1;
  }

  if (!got_multifile_name) {
    cerr << "Multifile name not specified.\n";
    usage();
    return 1;
  }

  // Split out the extensions named by -Z into different words.
  tokenize_extensions(dont_compress_str, dont_compress);

  bool okflag = true;
  if (create || append || update) {
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
