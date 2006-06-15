// Filename: bamCache.cxx
// Created by:  drose (09Jun06)
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

#include "bamCache.h"
#include "hashVal.h"
#include "datagramInputFile.h"
#include "datagramOutputFile.h"
#include "config_util.h"
#include "bam.h"
#include "typeRegistry.h"

BamCache *BamCache::_global_ptr = NULL;

////////////////////////////////////////////////////////////////////
//     Function: BamCache::Constructor
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
BamCache::
BamCache() :
  _active(true)
{
}

////////////////////////////////////////////////////////////////////
//     Function: BamCache::Destructor
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
BamCache::
~BamCache() {
}

////////////////////////////////////////////////////////////////////
//     Function: BamCache::set_root
//       Access: Published
//  Description: Changes the current root pathname of the cache.  This
//               specifies where the cache files are stored on disk.
//               This should name a directory that is on a disk local
//               to the machine (not on a network-mounted disk), for
//               instance, /tmp/panda-cache or /c/panda-cache.
//
//               If the directory does not already exist, it will be
//               created as a result of this call.
////////////////////////////////////////////////////////////////////
void BamCache::
set_root(const Filename &root) {
  _root = root;

  // For now, the filename must be a directory.  Maybe eventually we
  // will support writing caches to a Panda multifile (though maybe it
  // would be better to implement this kind of thing at a lower level,
  // via a writable VFS, in which case the specified root filename
  // will still be a "directory").
  if (!root.is_directory()) {
    Filename dirname(_root, Filename("."));
    dirname.make_dir();
  }
  nassertv(root.is_directory());
}

////////////////////////////////////////////////////////////////////
//     Function: BamCache::lookup
//       Access: Published
//  Description: Looks up a file in the cache.  
//
//               If the file is cacheable, then regardless of whether
//               the file is found in the cache or not, this returns a
//               BamCacheRecord.  On the other hand, if the file
//               cannot be cached, returns NULL.
//
//               If record->has_data() returns true, then the file was
//               found in the cache, and you may call
//               record->extract_data() to get the object.  If
//               record->has_data() returns false, then the file was
//               not found in the cache or the cache was stale; and
//               you should reload the source file (calling
//               record->add_dependent_file() for each file loaded,
//               including the original source file), and then call
//               record->set_data() to record the resulting loaded
//               object; and finally, you should call store() to write
//               the cached record to disk.
////////////////////////////////////////////////////////////////////
PT(BamCacheRecord) BamCache::
lookup(const Filename &source_filename, const string &cache_extension) {
  Filename source_pathname(source_filename);
  source_pathname.make_absolute();

  Filename rel_pathname(source_pathname);
  rel_pathname.make_relative_to(_root, false);
  if (rel_pathname.is_local()) {
    // If the source pathname is already within the cache directory,
    // don't cache it further.
    return NULL;
  }

  Filename cache_filename = hash_filename(source_pathname.get_fullpath());
  cache_filename.set_extension(cache_extension);

  return find_and_read_record(source_pathname, cache_filename);
}

////////////////////////////////////////////////////////////////////
//     Function: BamCache::store
//       Access: Published
//  Description: Flushes a cache entry to disk.  You must have
//               retrieved the cache record via a prior call to
//               lookup(), and then stored the data via
//               record->set_data().  Returns true on success, false
//               on failure.
////////////////////////////////////////////////////////////////////
bool BamCache::
store(BamCacheRecord *record) {
  nassertr(!record->_cache_pathname.empty(), false);
  nassertr(record->has_data(), false);

#ifndef NDEBUG
  // Ensure that the cache_pathname is within the _root directory tree.
  Filename rel_pathname(record->_cache_pathname);
  rel_pathname.make_relative_to(_root, false);
  nassertr(rel_pathname.is_local(), false);
#endif  // NDEBUG

  record->_recorded_time = time(NULL);

  ofstream cache_file;

  Filename filename = Filename::binary_filename(record->_cache_pathname);
  if (!filename.open_write(cache_file)) {
    util_cat.error()
      << "Could not open cache file: " << filename << "\n";
    return false;
  }

  DatagramOutputFile dout;
  if (!dout.open(cache_file)) {
    util_cat.error()
      << "Could not write cache file: " << filename << "\n";
    return false;
  }

  if (!dout.write_header(_bam_header)) {
    util_cat.error()
      << "Unable to write to " << filename << "\n";
    return false;
  }

  BamWriter writer(&dout, filename);
  if (!writer.init()) {
    return false;
  }

  TypeRegistry *type_registry = TypeRegistry::ptr();
  TypeHandle texture_type = type_registry->find_type("Texture");
  if (record->get_data()->is_of_type(texture_type)) {
    // Texture objects write the actual texture image.
    writer.set_file_texture_mode(BTM_rawdata);
  } else {
    // Any other kinds of objects write texture references.
    writer.set_file_texture_mode(BTM_fullpath);
  }

  if (!writer.write_object(record)) {
    return false;
  }

  if (!writer.write_object(record->get_data())) {
    return false;
  }

  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: BamCache::find_and_read_record
//       Access: Private
//  Description: Looks for the existing cache file that corresponds
//               to the indicated filename.  Normally, this is the
//               specified cache filename exactly; but in the case of
//               a hash collision, it may be a variant of the cache
//               filename.
////////////////////////////////////////////////////////////////////
PT(BamCacheRecord) BamCache::
find_and_read_record(const Filename &source_pathname, 
                     const Filename &cache_filename) const {
  int pass = 0;
  while (true) {
    PT(BamCacheRecord) record = 
      read_record(source_pathname, cache_filename, pass);
    if (record != (BamCacheRecord *)NULL) {
      return record;
    }
    ++pass;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: BamCache::read_record
//       Access: Private
//  Description: Reads the indicated cache file and returns its
//               associated record if it can be read and it matches
//               the source filename.
////////////////////////////////////////////////////////////////////
PT(BamCacheRecord) BamCache::
read_record(const Filename &source_pathname, 
            const Filename &cache_filename,
            int pass) const {
  Filename filename(_root, cache_filename);
  if (pass != 0) {
    ostringstream strm;
    strm << filename.get_basename_wo_extension() << "_" << pass;
    filename.set_basename_wo_extension(strm.str());
  }
  
  if (!filename.exists()) {
    // There is no such cache file already.  Declare it.
    filename.touch();
    PT(BamCacheRecord) record =
      new BamCacheRecord(source_pathname, cache_filename);
    record->_cache_pathname = filename;
    return record;
  }

  filename.set_binary();
  ifstream cache_file;
  if (!filename.open_read(cache_file)) {
    util_cat.debug()
      << "Could not open cache file: " << filename << "\n";
    return NULL;
  }

  DatagramInputFile din;
    
  if (!din.open(cache_file)) {
    util_cat.debug()
      << "Could not read cache file: " << filename << "\n";
    return NULL;
  }
  
  string head;
  if (!din.read_header(head, _bam_header.size())) {
    util_cat.debug()
      << filename << " is not a cache file.\n";
    return NULL;
  }
  
  if (head != _bam_header) {
    util_cat.debug()
      << filename << " is not a cache file.\n";
    return NULL;
  }
  
  BamReader reader(&din, filename);
  if (!reader.init()) {
    return NULL;
  }
  
  TypedWritable *object = reader.read_object();
  if (object == (TypedWritable *)NULL) {
    util_cat.debug()
      << filename << " is empty.\n";
    return NULL;
    
  } else if (!object->is_of_type(BamCacheRecord::get_class_type())) {
    util_cat.debug()
      << "Cache file " << filename << "contains a "
      << object->get_type() << ", not a BamCacheRecord.\n";
    return NULL;
  }
  
  PT(BamCacheRecord) record = DCAST(BamCacheRecord, object);
  if (!reader.resolve()) {
    util_cat.debug()
      << "Unable to fully resolve cache record in " << filename << "\n";
    return NULL;
  }

  if (record->get_source_pathname() != source_pathname) {
    util_cat.debug()
      << "Cache file " << filename << " references "
      << record->get_source_pathname() << ", not "
      << source_pathname << "\n";
    return NULL;
  }

  // From this point below, we have validated that the selected
  // filename is indeed a cache record for the indicated source file,
  // and therefore the cache record will be returned.

  // We still need to decide whether the cache record is stale.
  if (record->dependents_unchanged()) {
    // The cache record doesn't appear to be stale.  Load the cached
    // object.
    object = reader.read_object();

    if (object != (TypedWritable *)NULL) {
      if (!reader.resolve()) {
        util_cat.debug()
          << "Unable to fully resolve cached object in " << filename << "\n";
        delete object;
      } else {
        // The object is valid.  Store it in the record.
        record->set_data(object, true);
      }
    }
  }

  if (!record->has_data()) {
    // If we didn't find any data, the caller will have to reload it.
    record->clear_dependent_files();
  }

  record->_cache_pathname = filename;
  return record;
}

////////////////////////////////////////////////////////////////////
//     Function: BamCache::hash_filename
//       Access: Private, Static
//  Description: Returns the appropriate filename to use for a cache
//               file, given the fullpath string to the source
//               filename.
////////////////////////////////////////////////////////////////////
string BamCache::
hash_filename(const string &filename) {
#ifdef HAVE_OPENSSL
  // With OpenSSl, use the MD5 hash of the filename.
  HashVal hv;
  hv.hash_string(filename);
  ostringstream strm;
  hv.output_hex(strm);
  return strm.str();

#else  // HAVE_OPENSSL
  // Without OpenSSL, don't get fancy; just build a simple hash.
  unsigned int hash = 0;
  for (string::const_iterator si = filename.begin(); 
       si != filename.end(); 
       ++si) {
    hash = (hash * 9109) + (unsigned int)(*si);
  }

  ostringstream strm;
  strm << hex << setw(8) << setfill('0') << hash;
  return strm.str();

#endif  // HAVE_OPENSSL
}

////////////////////////////////////////////////////////////////////
//     Function: BamCache::make_global
//       Access: Private, Static
//  Description: Constructs the global BamCache object.
////////////////////////////////////////////////////////////////////
void BamCache::
make_global() {
  _global_ptr = new BamCache;

  ConfigVariableFilename model_cache_dir
    ("model-cache-dir", Filename(), 
     PRC_DESC("The full path to a directory, local to this computer, in which "
              "model and texture files will be cached on load.  If a directory "
              "name is specified here, files may be loaded from the cache "
              "instead of from their actual pathnames, which may save load time, "
              "especially if you are loading egg files instead of bam files, "
              "or if you are loading models from a shared network drive.  "
              "If this is the empty string, no cache will be used."));
  if (model_cache_dir.empty()) {
    _global_ptr->set_active(false);

  } else {
    _global_ptr->set_active(true);
    _global_ptr->set_root(model_cache_dir);
  }
}
