// Filename: findlatest.C
// Created by:  drose (28Aug97)
// 
////////////////////////////////////////////////////////////////////

#include <dtoolbase.h>

#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <ctype.h>

static const int max_vnums = 10;

#ifndef True
static const int True = 1;
static const int False = 0;
#endif


// The VersionNumber class defines the sequence of integers that makes
// up a SoftImage file version number.  One file's version is greater
// than another file's if the first integer from the left that differs
// is greater.
class VersionNumber {
public:
  VersionNumber();
  VersionNumber(const VersionNumber &copy);

  VersionNumber &operator = (const VersionNumber &copy);
  int operator < (const VersionNumber &copy);

  void Extract(const char *filename);

  void Output(ostream &out) const;

  int num_vnums;
  int vnums[max_vnums];
};

inline ostream &operator << (ostream &out, const VersionNumber &v) {
  v.Output(out);
  return out;
}

VersionNumber::
VersionNumber() {
  num_vnums = 0;
}

VersionNumber::
VersionNumber(const VersionNumber &copy) {
  (*this) = copy;
}

VersionNumber &VersionNumber::
operator = (const VersionNumber &copy) {
  num_vnums = copy.num_vnums;
  assert(num_vnums < max_vnums);
  for (int i = 0; i < num_vnums; i++) {
    vnums[i] = copy.vnums[i];
  }
  return *this;
}

int VersionNumber::
operator < (const VersionNumber &copy) {
  for (int i = 0; i < num_vnums && i < copy.num_vnums; i++) {
    if (vnums[i] < copy.vnums[i]) {
      return True;
    } else if (vnums[i] > copy.vnums[i]) {
      return False;
    }
  }
  return (num_vnums < copy.num_vnums);
}

// Extract the version number from a filename.
void VersionNumber::
Extract(const char *filename) {
  const char *basename = strchr(filename, '/');
  basename = (basename == NULL) ? filename : basename + 1;

  // Now basename is the beginning of the file, sans directory.
  // Find the extension.
  const char *ext = strrchr(basename, '.');
  if (ext == NULL) {
    ext = basename+strlen(basename);
  }

  // Now walk back from the extension, until we find something that's
  // not a number.
  int temp_vnums[max_vnums];
  int v = 0;

  const char *d = ext-1;
  while (v < max_vnums && d >= basename && isdigit(*d)) {
    while (d >= basename && isdigit(*d)) {
      d--;
    }
    temp_vnums[v++] = atoi(d+1);
    if (*d=='-' || *d=='.') {
      d--;
    }
  }
  
  // temp_vnums now has our version numbers in right-to-left order.
  // Reverse them back into vnums.
  for (int i = 0; i < v; i++) {
    vnums[i] = temp_vnums[v-1-i];
  }
  num_vnums = v;
}

void VersionNumber::
Output(ostream &out) const {
  out << "(";
  for (int i = 0; i < num_vnums; i++) {
    out << " " << vnums[i];
  }
  out << " )";
}


void
usage() {
  cerr << 
    "Usage:\n\n"

    "  findlatest dir file1 file2 file3 ... fileN\n\n"

    "Finds the filename with the greatest version number, and echoes dir/file.\n"
    "This is intended to identify the most recent version of a file in a directory.\n"
    "The files are assumed to be of the form basename[vvv].extension, where\n"
    "basename and extension are any arbitrary string, and vvv is any string\n"
    "of integers delimited by '.' or '-'.\n"
    "\n";
}

int
main(int argc, char *argv[]) {
  if (argc<2) {
    usage();
    exit(0);
  }

  if (argc<3) {
    // No files.
    exit(0);
  }

  VersionNumber max_version;
  int max_i = 0;

  VersionNumber v;
  for (int i = 2; i < argc; i++) {
    v.Extract(argv[i]);

    if (max_version < v) {
      max_version = v;
      max_i = i;
    }
  }

  if (max_i > 0) {
    cout << argv[1] << '/' << argv[max_i] << "\n";
  }

  return (0);
}

