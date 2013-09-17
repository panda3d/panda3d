// Filename: ppDependableFile.cxx
// Created by:  drose (15Oct00)
// 
////////////////////////////////////////////////////////////////////

#include "ppDependableFile.h"
#include "ppDirectory.h"
#include "ppDirectoryTree.h"
#include "filename.h"
#include "check_include.h"

#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

#include <assert.h>
#include <sys/stat.h>
#include <algorithm>
#include <iterator>

class SortDependableFilesByName {
public:
  bool operator () (PPDependableFile *a, PPDependableFile *b) const {
    return a->get_filename() < b->get_filename();
  }
};

////////////////////////////////////////////////////////////////////
//     Function: PPDependableFile::Ordering Operator
//       Access: Public
//  Description: We provide this function so we can sort the
//               dependency list into a consistent ordering, so that
//               the makefiles won't get randomly regenerated between
//               different sessions.
////////////////////////////////////////////////////////////////////
bool PPDependableFile::Dependency::
operator < (const Dependency &other) const {
  return _file->get_filename() < other._file->get_filename();
}

////////////////////////////////////////////////////////////////////
//     Function: PPDependableFile::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
PPDependableFile::
PPDependableFile(PPDirectory *directory, const string &filename) :
  _directory(directory),
  _filename(filename)
{
  _flags = 0;
  _mtime = 0;
}

////////////////////////////////////////////////////////////////////
//     Function: PPDependableFile::update_from_cache
//       Access: Public
//  Description: Called as the dependency cache file is being read,
//               this asks the file to update its information from the
//               cache file if appropriate.  This means comparing the
//               cached modification time against the file's actual
//               modification time, and storing the cached
//               dependencies if they match.
//
//               The return value is true if the cache is valid, false
//               if something appears to be wrong.
////////////////////////////////////////////////////////////////////
bool PPDependableFile::
update_from_cache(const vector<string> &words) {
  // Shouldn't call this once the file has actually been read.
  assert((_flags & F_updated) == 0);
  assert((_flags & F_updating) == 0);
  assert((_flags & F_from_cache) == 0);
  assert(words.size() >= 2);

  if (!exists()) {
    // The file doesn't even exist; clearly the cache is bad.
    _flags |= F_bad_cache;

  } else {
    // The second parameter is the cached modification time.
    time_t mtime = strtol(words[1].c_str(), (char **)NULL, 10);
    if (mtime == get_mtime()) {
      // The modification matches; preserve the cache information.
      PPDirectoryTree *tree = _directory->get_tree();

      _dependencies.clear();
      vector<string>::const_iterator wi;
      for (wi = words.begin() + 2; wi != words.end(); ++wi) {
        string dirpath = (*wi);

        Dependency dep;
        dep._okcircular = false;

        if (dirpath.length() > 1 && dirpath[0] == '/') {
          // If the first character is '/', it means that the file has
          // been marked okcircular.
          dep._okcircular = true;
          dirpath = dirpath.substr(1);
        }

        if (dirpath.length() > 2 && dirpath.substr(0, 2) == "*/") {
          // This is an extra include file, not a file in this source
          // tree.
          _extra_includes.push_back(dirpath.substr(2));

        } else {
          dep._file = 
            tree->get_dependable_file_by_dirpath(dirpath, false);
          if (dep._file != (PPDependableFile *)NULL) {
            _dependencies.push_back(dep);
          }
        }
      }

      _flags |= F_from_cache;
      sort(_dependencies.begin(), _dependencies.end());
    }
  }

  return ((_flags & F_bad_cache) == 0);
}

////////////////////////////////////////////////////////////////////
//     Function: PPDependableFile::clear_cache
//       Access: Public
//  Description: Forgets the cache we just read.
////////////////////////////////////////////////////////////////////
void PPDependableFile::
clear_cache() {
  _dependencies.clear();
  _flags &= ~(F_bad_cache | F_from_cache);
}

////////////////////////////////////////////////////////////////////
//     Function: PPDependableFile::write_cache
//       Access: Public
//  Description: Writes the dependency information out as a single
//               line to the indicated dependency cache file.
////////////////////////////////////////////////////////////////////
void PPDependableFile::
write_cache(ostream &out) {
  out << _filename << " " << get_mtime();

  Dependencies::const_iterator di;
  for (di = _dependencies.begin(); di != _dependencies.end(); ++di) {
    out << " ";
    if ((*di)._okcircular) {
      out << "/";
    }
    if ((*di)._file->get_directory()->get_tree() != get_directory()->get_tree()) {
      out << "+";
    }
    out << (*di)._file->get_dirpath();
  }

  // Also write out the extra includes--those #include directives
  // which do not reference a file within this source tree.  We need
  // those just for comparison's sake later, so we can tell whether
  // the cache line is still current (without having to know which
  // files are part of the tree).
  ExtraIncludes::const_iterator ei;
  for (ei = _extra_includes.begin(); ei != _extra_includes.end(); ++ei) {
    out << " */" << (*ei);
  }

  out << "\n";
}

////////////////////////////////////////////////////////////////////
//     Function: PPDependableFile::get_directory
//       Access: Public
//  Description: Returns the directory that this file can be found in.
////////////////////////////////////////////////////////////////////
PPDirectory *PPDependableFile::
get_directory() const {
  return _directory;
}

////////////////////////////////////////////////////////////////////
//     Function: PPDependableFile::get_filename
//       Access: Public
//  Description: Returns the local filename of this particular file
//               within the directory.
////////////////////////////////////////////////////////////////////
const string &PPDependableFile::
get_filename() const {
  return _filename;
}

////////////////////////////////////////////////////////////////////
//     Function: PPDependableFile::get_pathname
//       Access: Public
//  Description: Returns the relative pathname from the root of the
//               source tree to this particular filename.
////////////////////////////////////////////////////////////////////
string PPDependableFile::
get_pathname() const {
  return _directory->get_path() + "/" + _filename;
}

////////////////////////////////////////////////////////////////////
//     Function: PPDependableFile::get_fullpath
//       Access: Public
//  Description: Returns the full pathname to this particular filename.
////////////////////////////////////////////////////////////////////
string PPDependableFile::
get_fullpath() const {
  return _directory->get_fullpath() + "/" + _filename;
}

////////////////////////////////////////////////////////////////////
//     Function: PPDependableFile::get_dirpath
//       Access: Public
//  Description: Returns an abbreviated pathname to this file, in the
//               form dirname/filename.
////////////////////////////////////////////////////////////////////
string PPDependableFile::
get_dirpath() const {
  return _directory->get_dirname() + "/" + _filename;
}

////////////////////////////////////////////////////////////////////
//     Function: PPDependableFile::exists
//       Access: Public
//  Description: Returns true if the file exists, false if it does
//               not.
////////////////////////////////////////////////////////////////////
bool PPDependableFile::
exists() {
  stat_file();
  return ((_flags & F_exists) != 0);
}

////////////////////////////////////////////////////////////////////
//     Function: PPDependableFile::get_mtime
//       Access: Public
//  Description: Returns the last modification time of the file.
////////////////////////////////////////////////////////////////////
time_t PPDependableFile::
get_mtime() {
  stat_file();
  return _mtime;
}

////////////////////////////////////////////////////////////////////
//     Function: PPDependableFile::get_num_dependencies
//       Access: Public
//  Description: Returns the number of files this file depends on.
////////////////////////////////////////////////////////////////////
int PPDependableFile::
get_num_dependencies() {
  update_dependencies();
  return _dependencies.size();
}

////////////////////////////////////////////////////////////////////
//     Function: PPDependableFile::get_dependency
//       Access: Public
//  Description: Returns the nth file this file depends on.
////////////////////////////////////////////////////////////////////
PPDependableFile *PPDependableFile::
get_dependency(int n) {
  assert((_flags & F_updated) != 0);
  assert(n >= 0 && n < (int)_dependencies.size());
  return _dependencies[n]._file;
}

////////////////////////////////////////////////////////////////////
//     Function: PPDependableFile::get_complete_dependencies
//       Access: Public
//  Description: This flavor of get_complete_dependencies() works like
//               the one below, except it returns the results in a
//               consistently-ordered vector.  This allows us to keep
//               the dependencies in the same order between sessions
//               and prevent makefiles from being arbitrarily
//               regenerated.
////////////////////////////////////////////////////////////////////
void PPDependableFile::
get_complete_dependencies(vector<PPDependableFile *> &files) {
  set<PPDependableFile *> files_set;
  get_complete_dependencies(files_set);

  copy(files_set.begin(), files_set.end(), back_inserter(files));
  sort(files.begin(), files.end(), SortDependableFilesByName());
}

////////////////////////////////////////////////////////////////////
//     Function: PPDependableFile::get_complete_dependencies
//       Access: Public
//  Description: Recursively determines the complete set of files this
//               file depends on.  It is the user's responsibility to
//               empty the set before calling this function; the
//               results will simply be added to the existing set.
////////////////////////////////////////////////////////////////////
void PPDependableFile::
get_complete_dependencies(set<PPDependableFile *> &files) {
  update_dependencies();
  Dependencies::const_iterator di;
  for (di = _dependencies.begin(); di != _dependencies.end(); ++di) {
    PPDependableFile *file = (*di)._file;
    if (files.insert(file).second) {
      file->get_complete_dependencies(files);
    }
  }
}

////////////////////////////////////////////////////////////////////
//     Function: PPDependableFile::is_circularity
//       Access: Public
//  Description: Returns true if a circular dependency exists between
//               this file and one or more other files.
////////////////////////////////////////////////////////////////////
bool PPDependableFile::
is_circularity() {
  update_dependencies();
  return (_flags & F_circularity) != 0;
}

////////////////////////////////////////////////////////////////////
//     Function: PPDependableFile::get_circularity
//       Access: Public
//  Description: If is_circularity() returns true, returns a string
//               describing the circular dependency path for the user.
////////////////////////////////////////////////////////////////////
string PPDependableFile::
get_circularity() {
  update_dependencies();
  return _circularity;
}

////////////////////////////////////////////////////////////////////
//     Function: PPDependableFile::was_examined
//       Access: Public
//  Description: Returns true if anyone ever asked this file for its
//               list of dependencies, or false otherwise.
////////////////////////////////////////////////////////////////////
bool PPDependableFile::
was_examined() const {
  return ((_flags & F_updated) != 0);
}

////////////////////////////////////////////////////////////////////
//     Function: PPDependableFile::was_cached
//       Access: Public
//  Description: Returns true if this file was found in the cache,
//               false otherwise.
////////////////////////////////////////////////////////////////////
bool PPDependableFile::
was_cached() const {
  return ((_flags & F_from_cache) != 0);
}

////////////////////////////////////////////////////////////////////
//     Function: PPDependableFile::update_dependencies
//       Access: Private
//  Description: Builds up the dependency list--the list of files this
//               file depends on--if it hasn't already been built.  If
//               a circular dependency is detected during this
//               process, _circularity and _circularity_detected will
//               be updated accordingly.
////////////////////////////////////////////////////////////////////
void PPDependableFile::
update_dependencies() {
  if ((_flags & F_updated) != 0) {
    return;
  }  

  assert((_flags & F_updating) == 0);
  string circularity;
  compute_dependencies(circularity);
}

////////////////////////////////////////////////////////////////////
//     Function: PPDependableFile::compute_dependencies
//       Access: Private
//  Description: Builds up the dependency list--the list of files this
//               file depends on--if it hasn't already been built.
//
//               If a circularity is detected, e.g. two files depend
//               on each other, a pointer to the offending file is
//               returned and the string is updated to indicate the
//               circularity.  Otherwise, if there is no circularity,
//               NULL is returned.
////////////////////////////////////////////////////////////////////
PPDependableFile *PPDependableFile::
compute_dependencies(string &circularity) {
  if ((_flags & F_updated) != 0) {
    return (PPDependableFile *)NULL;

  } else if ((_flags & F_updating) != 0) {
    // Oh oh, a circular dependency!
    circularity = get_dirpath();
    return this;
  }

  _flags |= F_updating;

  if ((_flags & F_from_cache) == 0) {
    // Now open the file and scan it for #include statements.
    Filename filename(get_fullpath());
    filename.set_text();
    ifstream in;
    if (!filename.open_read(in)) {
      // Can't read the file, or the file doesn't exist.  Interesting.
      if (exists()) {
        cerr << "Warning: dependent file " << filename
             << " exists but cannot be read.\n";
      } else {
        cerr << "Warning: dependent file " << filename
             << " does not exist.\n";
        _flags |= F_bad_cache;
      }

    } else {
      if (verbose) {
        cerr << "Reading (dep) \"" << filename << "\"\n";
      }
      PPDirectoryTree *tree = _directory->get_tree()->get_main_tree();
      
      bool okcircular = false;
      string line;
      getline(in, line);
      while (!in.fail() && !in.eof()) {
        if (line.substr(0, 16) == "/* okcircular */") {
          okcircular = true;
        } else {
          string filename = check_include(line);
          if (!filename.empty() && filename.find('/') == string::npos) {
            Dependency dep;
            dep._okcircular = okcircular;
            dep._file = tree->find_dependable_file(filename);
            if (dep._file != (PPDependableFile *)NULL) {
              // All right!  Here's a file we depend on.  Add it to the
              // list.
              _dependencies.push_back(dep);
          
            } else {
              // It's an include file from somewhere else, not from within
              // our source tree.  We don't care about it, but we do need
              // to record it so we can easily check later if the cache
              // file has gone stale.
              _extra_includes.push_back(filename);
            }
          }
          okcircular = false;
        }
        getline(in, line);
      }
    }
  }

  // Now recursively expand all our dependent files, so we can check
  // for circularities.
  PPDependableFile *circ = (PPDependableFile *)NULL;

  Dependencies::iterator di;
  for (di = _dependencies.begin(); 
       di != _dependencies.end() && circ == (PPDependableFile *)NULL;
       ++di) {
    // Skip this file if the user specifically marked it
    // with an "okcircular" comment.
    if (!(*di)._okcircular) {
      circ = (*di)._file->compute_dependencies(circularity);
      if (((*di)._file->_flags & F_bad_cache) != 0) {
        // If our dependent file had a broken cache, our own cache is
        // suspect.
        _flags |= F_bad_cache;
      }

      if (circ != (PPDependableFile *)NULL) {
        // Oops, a circularity.  Silly user.
        circularity = get_dirpath() + " => " + circularity;
    
        if (circ == this) {
          _flags |= F_circularity;
          _circularity = circularity;
        }
      }
    }
  }

  _flags = (_flags & ~F_updating) | F_updated;
  sort(_dependencies.begin(), _dependencies.end());

  if ((_flags & (F_bad_cache | F_from_cache)) == (F_bad_cache | F_from_cache)) {
    // Our cache is suspect.  Re-read the file to flush the cache.
    if (verbose) {
      cerr << "Dependency cache for \"" << get_fullpath() << "\" is suspect.\n";
    }

    clear_cache();
    _flags &= ~F_updated;

    return compute_dependencies(circularity);
  }

  return circ;
}

////////////////////////////////////////////////////////////////////
//     Function: PPDependableFile::stat_file
//       Access: Private
//  Description: Performs a stat() on the file, if it has not already
//               been performed, to check whether the file exists and
//               to get its last-modification time.
////////////////////////////////////////////////////////////////////
void PPDependableFile::
stat_file() {
  if ((_flags & F_statted) != 0) {
    // Already done.
    return;
  }

  _flags |= F_statted;
  struct stat st;
  Filename pathname(get_fullpath());
  string ospath = pathname.to_os_specific();
  if (stat(ospath.c_str(), &st) < 0) {
    // The file doesn't exist!
    return;
  }

#ifdef S_ISREG
  if (!S_ISREG(st.st_mode)) {
    // The file exists, but it's not a regular file--we consider that
    // not existing.
    return;
  }
#endif  // S_ISREG

  _flags |= F_exists;
  _mtime = st.st_mtime;
}
