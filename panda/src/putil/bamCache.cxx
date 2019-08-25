/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file bamCache.cxx
 * @author drose
 * @date 2006-06-09
 */

#include "bamCache.h"
#include "bamCacheIndex.h"
#include "bamReader.h"
#include "bamWriter.h"
#include "hashVal.h"
#include "datagramInputFile.h"
#include "datagramOutputFile.h"
#include "config_putil.h"
#include "bam.h"
#include "typeRegistry.h"
#include "string_utils.h"
#include "configVariableInt.h"
#include "configVariableString.h"
#include "configVariableFilename.h"
#include "virtualFileSystem.h"

using std::istream;
using std::ostream;
using std::ostringstream;
using std::string;

BamCache *BamCache::_global_ptr = nullptr;

/**
 *
 */
BamCache::
BamCache() :
  _active(true),
  _read_only(false),
  _index(new BamCacheIndex),
  _index_stale_since(0)
{
  ConfigVariableFilename model_cache_dir
    ("model-cache-dir", Filename(),
     PRC_DESC("The full path to a directory, local to this computer, in which "
              "model and texture files will be cached on load.  If a directory "
              "name is specified here, files may be loaded from the cache "
              "instead of from their actual pathnames, which may save load time, "
              "especially if you are loading egg files instead of bam files, "
              "or if you are loading models from a shared network drive.  "
              "If this is the empty string, no cache will be used."));

  ConfigVariableInt model_cache_flush
    ("model-cache-flush", 30,
     PRC_DESC("This is the amount of time, in seconds, between automatic "
              "flushes of the model-cache index."));

  ConfigVariableBool model_cache_models
    ("model-cache-models", true,
     PRC_DESC("If this is set to true, models will be cached in the "
              "model cache, as bam files."));

  ConfigVariableBool model_cache_textures
    ("model-cache-textures", true,
     PRC_DESC("If this is set to true, textures will also be cached in the "
              "model cache, as txo files."));

  ConfigVariableBool model_cache_compressed_textures
    ("model-cache-compressed-textures", false,
     PRC_DESC("If this is set to true, compressed textures will be cached "
              "in the model cache, in their compressed form as downloaded "
              "by the GSG.  This may be set in conjunction with "
              "model-cache-textures, or it may be independent."));

  ConfigVariableBool model_cache_compiled_shaders
    ("model-cache-compiled-shaders", false,
     PRC_DESC("If this is set to true, compiled shaders will be cached "
              "in the model cache, in their binary form as downloaded "
              "by the GSG."));

  ConfigVariableInt model_cache_max_kbytes
    ("model-cache-max-kbytes", 10485760,
     PRC_DESC("This is the maximum size of the model cache, in kilobytes."));

  _cache_models = model_cache_models;
  _cache_textures = model_cache_textures;
  _cache_compressed_textures = model_cache_compressed_textures;
  _cache_compiled_shaders = model_cache_compiled_shaders;

  _flush_time = model_cache_flush;
  _max_kbytes = model_cache_max_kbytes;

  if (!model_cache_dir.empty()) {
    set_root(model_cache_dir);
  }
}

/**
 *
 */
BamCache::
~BamCache() {
  flush_index();
  delete _index;
  _index = nullptr;
}

/**
 * Changes the current root pathname of the cache.  This specifies where the
 * cache files are stored on disk.  This should name a directory that is on a
 * disk local to the machine (not on a network-mounted disk), for instance,
 * /tmp/panda-cache or /c/panda-cache.
 *
 * If the directory does not already exist, it will be created as a result of
 * this call.
 */
void BamCache::
set_root(const Filename &root) {
  ReMutexHolder holder(_lock);
  flush_index();
  _root = root;

  // The root filename must be a directory.
  VirtualFileSystem *vfs = VirtualFileSystem::get_global_ptr();
  if (!vfs->is_directory(_root)) {
    vfs->make_directory_full(_root);
  }

  delete _index;
  _index = new BamCacheIndex;
  _index_stale_since = 0;
  read_index();
  check_cache_size();

  nassertv(vfs->is_directory(_root));
}

/**
 * Looks up a file in the cache.
 *
 * If the file is cacheable, then regardless of whether the file is found in
 * the cache or not, this returns a BamCacheRecord.  On the other hand, if the
 * file cannot be cached, returns NULL.
 *
 * If record->has_data() returns true, then the file was found in the cache,
 * and you may call record->extract_data() to get the object.  If
 * record->has_data() returns false, then the file was not found in the cache
 * or the cache was stale; and you should reload the source file (calling
 * record->add_dependent_file() for each file loaded, including the original
 * source file), and then call record->set_data() to record the resulting
 * loaded object; and finally, you should call store() to write the cached
 * record to disk.
 */
PT(BamCacheRecord) BamCache::
lookup(const Filename &source_filename, const string &cache_extension) {
  ReMutexHolder holder(_lock);
  consider_flush_index();

  VirtualFileSystem *vfs = VirtualFileSystem::get_global_ptr();

  Filename source_pathname(source_filename);
  source_pathname.make_absolute(vfs->get_cwd());

  Filename rel_pathname(source_pathname);
  rel_pathname.make_relative_to(_root, false);
  if (rel_pathname.is_local()) {
    // If the source pathname is already within the cache directory, don't
    // cache it further.
    return nullptr;
  }

  Filename cache_filename = hash_filename(source_pathname.get_fullpath());
  cache_filename.set_extension(cache_extension);

  return find_and_read_record(source_pathname, cache_filename);
}

/**
 * Flushes a cache entry to disk.  You must have retrieved the cache record
 * via a prior call to lookup(), and then stored the data via
 * record->set_data().  Returns true on success, false on failure.
 */
bool BamCache::
store(BamCacheRecord *record) {
  VirtualFileSystem *vfs = VirtualFileSystem::get_global_ptr();
  ReMutexHolder holder(_lock);
  nassertr(!record->_cache_pathname.empty(), false);
  nassertr(record->has_data(), false);

  if (_read_only) {
    return false;
  }

  consider_flush_index();

#ifndef NDEBUG
  // Ensure that the cache_pathname is within the _root directory tree.
  Filename rel_pathname(record->_cache_pathname);
  rel_pathname.make_relative_to(_root, false);
  nassertr(rel_pathname.is_local(), false);
#endif  // NDEBUG

  record->_recorded_time = time(nullptr);

  Filename cache_pathname = Filename::binary_filename(record->_cache_pathname);

  // We actually do the write to a temporary filename first, and then move it
  // into place, so that no one attempts to read the file while it is in the
  // process of being written.
  Thread *current_thread = Thread::get_current_thread();
  string extension = current_thread->get_unique_id() + string(".tmp");
  Filename temp_pathname = cache_pathname;
  temp_pathname.set_extension(extension);
  temp_pathname.set_binary();

  DatagramOutputFile dout;
  if (!dout.open(temp_pathname)) {
    util_cat.error()
      << "Could not write cache file: " << temp_pathname << "\n";
    vfs->delete_file(temp_pathname);
    emergency_read_only();
    return false;
  }

  if (!dout.write_header(_bam_header)) {
    util_cat.error()
      << "Unable to write to " << temp_pathname << "\n";
    vfs->delete_file(temp_pathname);
    return false;
  }

  {
    BamWriter writer(&dout);
    if (!writer.init()) {
      util_cat.error()
        << "Unable to write Bam header to " << temp_pathname << "\n";
      vfs->delete_file(temp_pathname);
      return false;
    }

    TypeRegistry *type_registry = TypeRegistry::ptr();
    TypeHandle texture_type = type_registry->find_type("Texture");
    if (record->get_data()->is_of_type(texture_type)) {
      // Texture objects write the actual texture image.
      writer.set_file_texture_mode(BamWriter::BTM_rawdata);
    } else {
      // Any other kinds of objects write texture references.
      writer.set_file_texture_mode(BamWriter::BTM_fullpath);
    }

    // This is necessary for relative NodePaths to work.
    TypeHandle node_type = type_registry->find_type("PandaNode");
    if (record->get_data()->is_of_type(node_type)) {
      writer.set_root_node(record->get_data());
    }

    if (!writer.write_object(record)) {
      util_cat.error()
        << "Unable to write object to " << temp_pathname << "\n";
      vfs->delete_file(temp_pathname);
      return false;
    }

    if (!writer.write_object(record->get_data())) {
      util_cat.error()
        << "Unable to write object data to " << temp_pathname << "\n";
      vfs->delete_file(temp_pathname);
      return false;
    }

    // Now that we are done with the BamWriter, it's important to let it
    // destruct now and clean itself up, or it might get mad if we delete any
    // TypedWritables below that haven't been written yet.
  }

  record->_record_size = dout.get_file_pos();
  dout.close();

  // Now move the file into place.
  if (!vfs->rename_file(temp_pathname, cache_pathname) && vfs->exists(temp_pathname)) {
    vfs->delete_file(cache_pathname);
    if (!vfs->rename_file(temp_pathname, cache_pathname)) {
      util_cat.error()
        << "Unable to rename " << temp_pathname << " to "
        << cache_pathname << "\n";
      vfs->delete_file(temp_pathname);
      return false;
    }
  }

  add_to_index(record);

  return true;
}

/**
 * Called when an attempt to write to the cache dir has failed, usually for
 * lack of disk space or because of incorrect file permissions.  Outputs an
 * error and puts the BamCache into read-only mode.
 */
void BamCache::
emergency_read_only() {
  util_cat.error() <<
    "Could not write to the Bam Cache.  Disabling future attempts.\n";
  _read_only = true;
}

/**
 * Flushes the index if enough time has elapsed since the index was last
 * flushed.
 */
void BamCache::
consider_flush_index() {
#if defined(HAVE_THREADS) || defined(DEBUG_THREADS)
  if (!_lock.try_lock()) {
    // If we can't grab the lock, no big deal.  We don't want to hold up
    // the frame waiting for a cache operation.  We can try again later.
    return;
  }
#endif

  if (_index_stale_since != 0) {
    int elapsed = (int)time(nullptr) - (int)_index_stale_since;
    if (elapsed > _flush_time) {
      flush_index();
    }
  }

#if defined(HAVE_THREADS) || defined(DEBUG_THREADS)
  _lock.unlock();
#endif
}

/**
 * Ensures the index is written to disk.
 */
void BamCache::
flush_index() {
  ReMutexHolder holder(_lock);
  if (_index_stale_since == 0) {
    // Never mind.
    return;
  }

  while (true) {
    if (_read_only) {
      return;
    }

    Filename temp_pathname = Filename::temporary(_root, "index-", ".boo");

    if (!do_write_index(temp_pathname, _index)) {
      emergency_read_only();
      return;
    }

    // Now atomically write the name of this index file to the index reference
    // file.
    VirtualFileSystem *vfs = VirtualFileSystem::get_global_ptr();
    Filename index_ref_pathname(_root, Filename("index_name.txt"));
    string old_index = _index_ref_contents;
    string new_index = temp_pathname.get_basename() + "\n";
    string orig_index;

    if (vfs->atomic_compare_and_exchange_contents(index_ref_pathname, orig_index, old_index, new_index)) {
      // We successfully wrote our version of the index, and no other process
      // beat us to it.  Our index is now the official one.  Remove the old
      // index.
      vfs->delete_file(_index_pathname);
      _index_pathname = temp_pathname;
      _index_ref_contents = new_index;
      _index_stale_since = 0;
      return;
    }

    // Shoot, some other process updated the index while we were trying to
    // update it, and they beat us to it.  We have to merge, and try again.
    vfs->delete_file(temp_pathname);
    _index_pathname = Filename(_root, Filename(trim(orig_index)));
    _index_ref_contents = orig_index;
    read_index();
  }
  check_cache_size();
}

/**
 * Writes the contents of the index to standard output.
 */
void BamCache::
list_index(ostream &out, int indent_level) const {
  _index->write(out, indent_level);
}

/**
 * Reads, or re-reads the index file from disk.  If _index_stale_since is
 * nonzero, the index file is read and then merged with our current index.
 */
void BamCache::
read_index() {
  if (!read_index_pathname(_index_pathname, _index_ref_contents)) {
    // Couldn't read the index ref; rebuild the index.
    rebuild_index();
    return;
  }

  while (true) {
    BamCacheIndex *new_index = do_read_index(_index_pathname);
    if (new_index != nullptr) {
      merge_index(new_index);
      return;
    }

    // We couldn't read the index.  Maybe it's been removed already.  See if
    // the index_pathname has changed.
    Filename old_index_pathname = _index_pathname;
    if (!read_index_pathname(_index_pathname, _index_ref_contents)) {
      // Couldn't read the index ref; rebuild the index.
      rebuild_index();
      return;
    }

    if (old_index_pathname == _index_pathname) {
      // Nope, we just couldn't read it.  Delete it and build a new one.
      VirtualFileSystem *vfs = VirtualFileSystem::get_global_ptr();
      vfs->delete_file(_index_pathname);
      rebuild_index();
      flush_index();
      return;
    }
  }
}

/**
 * Atomically reads the current index filename from the index reference file.
 * The index filename moves around as different processes update the index.
 */
bool BamCache::
read_index_pathname(Filename &index_pathname, string &index_ref_contents) const {
  VirtualFileSystem *vfs = VirtualFileSystem::get_global_ptr();
  index_ref_contents.clear();
  Filename index_ref_pathname(_root, Filename("index_name.txt"));
  if (!vfs->atomic_read_contents(index_ref_pathname, index_ref_contents)) {
    return false;
  }

  string trimmed = trim(index_ref_contents);
  if (trimmed.empty()) {
    index_pathname = Filename();
  } else {
    index_pathname = Filename(_root, Filename(trimmed));
  }
  return true;
}

/**
 * The supplied index file has been updated by some other process.  Merge it
 * with our current index.
 *
 * Ownership of the pointer is transferred with this call.  The caller should
 * assume that new_index will be deleted by this method.
 */
void BamCache::
merge_index(BamCacheIndex *new_index) {
  if (_index_stale_since == 0) {
    // If our index isn't stale, just replace it.
    delete _index;
    _index = new_index;
    return;
  }

  BamCacheIndex *old_index = _index;
  old_index->release_records();
  new_index->release_records();
  _index = new BamCacheIndex;

  BamCacheIndex::Records::const_iterator ai = old_index->_records.begin();
  BamCacheIndex::Records::const_iterator bi = new_index->_records.begin();

  while (ai != old_index->_records.end() &&
         bi != new_index->_records.end()) {
    if ((*ai).first < (*bi).first) {
      // Here is an entry we have in our index, not present in the new index.
      PT(BamCacheRecord) record = (*ai).second;
      Filename cache_pathname(_root, record->get_cache_filename());
      if (cache_pathname.exists()) {
        // The file exists; keep it.
        _index->_records.insert(_index->_records.end(), BamCacheIndex::Records::value_type(record->get_source_pathname(), record));
      }
      ++ai;

    } else if ((*bi).first < (*ai).first) {
      // Here is an entry in the new index, not present in our index.
      PT(BamCacheRecord) record = (*bi).second;
      Filename cache_pathname(_root, record->get_cache_filename());
      if (cache_pathname.exists()) {
        // The file exists; keep it.
        _index->_records.insert(_index->_records.end(), BamCacheIndex::Records::value_type(record->get_source_pathname(), record));
      }
      ++bi;

    } else {
      // Here is an entry we have in both.
      PT(BamCacheRecord) a_record = (*ai).second;
      PT(BamCacheRecord) b_record = (*bi).second;
      if (*a_record == *b_record) {
        // They're the same entry.  It doesn't really matter which one we
        // keep.
        _index->_records.insert(_index->_records.end(), BamCacheIndex::Records::value_type(a_record->get_source_pathname(), a_record));

      } else {
        // They're different.  Just throw them both away, and re-read the
        // current data from the cache file.

        Filename cache_pathname(_root, a_record->get_cache_filename());

        if (cache_pathname.exists()) {
          PT(BamCacheRecord) record = do_read_record(cache_pathname, false);
          if (record != nullptr) {
            _index->_records.insert(_index->_records.end(), BamCacheIndex::Records::value_type(record->get_source_pathname(), record));
          }
        }
      }

      ++ai;
      ++bi;
    }
  }

  while (ai != old_index->_records.end()) {
    // Here is an entry we have in our index, not present in the new index.
    PT(BamCacheRecord) record = (*ai).second;
    Filename cache_pathname(_root, record->get_cache_filename());
    if (cache_pathname.exists()) {
      // The file exists; keep it.
      _index->_records.insert(_index->_records.end(), BamCacheIndex::Records::value_type(record->get_source_pathname(), record));
    }
    ++ai;
  }

  while (bi != new_index->_records.end()) {
    // Here is an entry in the new index, not present in our index.
    PT(BamCacheRecord) record = (*bi).second;
    Filename cache_pathname(_root, record->get_cache_filename());
    if (cache_pathname.exists()) {
      // The file exists; keep it.
      _index->_records.insert(_index->_records.end(), BamCacheIndex::Records::value_type(record->get_source_pathname(), record));
    }
    ++bi;
  }

  _index->process_new_records();
}

/**
 * Regenerates the index from scratch by scanning the directory.
 */
void BamCache::
rebuild_index() {
  VirtualFileSystem *vfs = VirtualFileSystem::get_global_ptr();

  PT(VirtualFileList) contents = vfs->scan_directory(_root);
  if (contents == nullptr) {
    util_cat.error()
      << "Unable to read directory " << _root << ", caching disabled.\n";
    set_active(false);
    return;
  }

  delete _index;
  _index = new BamCacheIndex;

  int num_files = contents->get_num_files();
  for (int ci = 0; ci < num_files; ++ci) {
    VirtualFile *file = contents->get_file(ci);
    Filename filename = file->get_filename();
    if (filename.get_extension() == "bam" ||
        filename.get_extension() == "txo") {
      Filename pathname(_root, filename);

      PT(BamCacheRecord) record = do_read_record(pathname, false);
      if (record == nullptr) {
        // Well, it was invalid, so blow it away.
        if (util_cat.is_debug()) {
          util_cat.debug()
            << "Deleting invalid " << pathname << "\n";
        }
        file->delete_file();

      } else {
        record->_record_access_time = record->_recorded_time;

        bool inserted = _index->_records.insert(BamCacheIndex::Records::value_type(record->get_source_pathname(), record)).second;
        if (!inserted) {
          util_cat.info()
            << "Multiple cache files defining " << record->get_source_pathname() << "\n";
          file->delete_file();
        }
      }
    }
  }
  _index->process_new_records();

  _index_stale_since = time(nullptr);
  check_cache_size();
  flush_index();
}

/**
 * Updates the index entry for the indicated record.  Note that a copy of the
 * record is made first.
 */
void BamCache::
add_to_index(const BamCacheRecord *record) {
  PT(BamCacheRecord) new_record = record->make_copy();

  if (_index->add_record(new_record)) {
    mark_index_stale();
    check_cache_size();
  }
}

/**
 * Removes the index entry for the indicated record, if there is one.
 */
void BamCache::
remove_from_index(const Filename &source_pathname) {
  if (_index->remove_record(source_pathname)) {
    mark_index_stale();
  }
}

/**
 * If the cache size has exceeded its specified size limit, removes an old
 * file.
 */
void BamCache::
check_cache_size() {
  if (_index->_cache_size == 0) {
    // 0 means no limit.
    return;
  }

  if (_index->_cache_size / 1024 > _max_kbytes) {
    while (_index->_cache_size / 1024 > _max_kbytes) {
      PT(BamCacheRecord) record = _index->evict_old_file();
      if (record == nullptr) {
        // Never mind; the cache is empty.
        break;
      }
      VirtualFileSystem *vfs = VirtualFileSystem::get_global_ptr();
      Filename cache_pathname(_root, record->get_cache_filename());
      if (util_cat.is_debug()) {
        util_cat.debug()
          << "Deleting " << cache_pathname
          << " to keep cache size below " << _max_kbytes << "K\n";
      }
      vfs->delete_file(cache_pathname);
    }
    mark_index_stale();
  }
}

/**
 * Reads the index data from the specified filename.  Returns a newly-
 * allocated BamCacheIndex object on success, or NULL on failure.
 */
BamCacheIndex *BamCache::
do_read_index(const Filename &index_pathname) {
  if (index_pathname.empty()) {
    return nullptr;
  }

  DatagramInputFile din;
  if (!din.open(index_pathname)) {
    if (util_cat.is_debug()) {
      util_cat.debug()
        << "Could not read index file: " << index_pathname << "\n";
    }
    return nullptr;
  }

  string head;
  if (!din.read_header(head, _bam_header.size())) {
    if (util_cat.is_debug()) {
      util_cat.debug()
        << index_pathname << " is not an index file.\n";
    }
    return nullptr;
  }

  if (head != _bam_header) {
    if (util_cat.is_debug()) {
      util_cat.debug()
        << index_pathname << " is not an index file.\n";
    }
    return nullptr;
  }

  BamReader reader(&din);
  if (!reader.init()) {
    return nullptr;
  }

  TypedWritable *object = reader.read_object();

  if (object == nullptr) {
    util_cat.error()
      << "Cache index " << index_pathname << " is empty.\n";
    return nullptr;

  } else if (!object->is_of_type(BamCacheIndex::get_class_type())) {
    util_cat.error()
      << "Cache index " << index_pathname << " contains a "
      << object->get_type() << ", not a BamCacheIndex.\n";
    return nullptr;
  }

  BamCacheIndex *index = DCAST(BamCacheIndex, object);
  if (!reader.resolve()) {
    util_cat.error()
      << "Unable to fully resolve cache index file.\n";
    return nullptr;
  }

  return index;
}

/**
 * Writes the given index data to the specified filename.
 */
bool BamCache::
do_write_index(const Filename &index_pathname, const BamCacheIndex *index) {
  VirtualFileSystem *vfs = VirtualFileSystem::get_global_ptr();
  DatagramOutputFile dout;
  if (!dout.open(index_pathname)) {
    util_cat.error()
      << "Could not write index file: " << index_pathname << "\n";
    vfs->delete_file(index_pathname);
    return false;
  }

  if (!dout.write_header(_bam_header)) {
    util_cat.error()
      << "Unable to write to " << index_pathname << "\n";
    vfs->delete_file(index_pathname);
    return false;
  }

  {
    BamWriter writer(&dout);
    if (!writer.init()) {
      vfs->delete_file(index_pathname);
      return false;
    }

    if (!writer.write_object(index)) {
      vfs->delete_file(index_pathname);
      return false;
    }
  }

  return true;
}

/**
 * Looks for the existing cache file that corresponds to the indicated
 * filename.  Normally, this is the specified cache filename exactly; but in
 * the case of a hash collision, it may be a variant of the cache filename.
 */
PT(BamCacheRecord) BamCache::
find_and_read_record(const Filename &source_pathname,
                     const Filename &cache_filename) {
  int pass = 0;
  while (true) {
    PT(BamCacheRecord) record =
      read_record(source_pathname, cache_filename, pass);
    if (record != nullptr) {
      add_to_index(record);
      return record;
    }
    ++pass;
  }
}

/**
 * Reads the indicated cache file and returns its associated record if it can
 * be read and it matches the source filename.
 */
PT(BamCacheRecord) BamCache::
read_record(const Filename &source_pathname,
            const Filename &cache_filename,
            int pass) {
  VirtualFileSystem *vfs = VirtualFileSystem::get_global_ptr();
  Filename cache_pathname(_root, cache_filename);
  if (pass != 0) {
    ostringstream strm;
    strm << cache_pathname.get_basename_wo_extension() << "_" << pass;
    cache_pathname.set_basename_wo_extension(strm.str());
  }

  if (!cache_pathname.exists()) {
    // There is no such cache file already.  Declare it.
    if (util_cat.is_debug()) {
      util_cat.debug()
        << "Declaring new cache file " << cache_pathname << " for " << source_pathname << "\n";
    }
    PT(BamCacheRecord) record =
      new BamCacheRecord(source_pathname, cache_filename);
    record->_cache_pathname = cache_pathname;
    return record;
  }

  if (util_cat.is_debug()) {
    util_cat.debug()
      << "Reading cache file " << cache_pathname << " for " << source_pathname << "\n";
  }

  PT(BamCacheRecord) record = do_read_record(cache_pathname, true);
  if (record == nullptr) {
    // Well, it was invalid, so blow it away, and make a new one.
    if (util_cat.is_debug()) {
      util_cat.debug()
        << "Deleting invalid cache file " << cache_pathname << "\n";
    }
    vfs->delete_file(cache_pathname);
    remove_from_index(source_pathname);

    PT(BamCacheRecord) record =
      new BamCacheRecord(source_pathname, cache_filename);
    record->_cache_pathname = cache_pathname;
    return record;
  }

  if (record->get_source_pathname() != source_pathname) {
    // This might be just a hash conflict.
    if (util_cat.is_debug()) {
      util_cat.debug()
        << "Cache file " << cache_pathname << " references "
        << record->get_source_pathname() << ", not "
        << source_pathname << "\n";
    }
    return nullptr;
  }

  if (!record->has_data()) {
    // If we didn't find any data, the caller will have to reload it.
    record->clear_dependent_files();
  }

  record->_cache_pathname = cache_pathname;
  return record;
}

/**
 * Actually reads a record from the file.
 */
PT(BamCacheRecord) BamCache::
do_read_record(const Filename &cache_pathname, bool read_data) {
  DatagramInputFile din;
  if (!din.open(cache_pathname)) {
    if (util_cat.is_debug()) {
      util_cat.debug()
        << "Could not read cache file: " << cache_pathname << "\n";
    }
    return nullptr;
  }

  string head;
  if (!din.read_header(head, _bam_header.size())) {
    if (util_cat.is_debug()) {
      util_cat.debug()
        << cache_pathname << " is not a cache file.\n";
    }
    return nullptr;
  }

  if (head != _bam_header) {
    if (util_cat.is_debug()) {
      util_cat.debug()
        << cache_pathname << " is not a cache file.\n";
    }
    return nullptr;
  }

  BamReader reader(&din);
  if (!reader.init()) {
    return nullptr;
  }

  TypedWritable *object = reader.read_object();
  if (object == nullptr) {
    if (util_cat.is_debug()) {
      util_cat.debug()
        << cache_pathname << " is empty.\n";
    }
    return nullptr;

  } else if (!object->is_of_type(BamCacheRecord::get_class_type())) {
    if (util_cat.is_debug()) {
      util_cat.debug()
        << "Cache file " << cache_pathname << " contains a "
        << object->get_type() << ", not a BamCacheRecord.\n";
    }
    return nullptr;
  }

  PT(BamCacheRecord) record = DCAST(BamCacheRecord, object);
  if (!reader.resolve()) {
    if (util_cat.is_debug()) {
      util_cat.debug()
        << "Unable to fully resolve cache record in " << cache_pathname << "\n";
    }
    return nullptr;
  }

  // From this point below, we have validated that the selected filename is
  // indeed a cache record for the indicated source file, and therefore the
  // cache record will be returned.

  // We still need to decide whether the cache record is stale.
  if (read_data && record->dependents_unchanged()) {
    // The cache record doesn't appear to be stale.  Load the cached object.
    TypedWritable *ptr;
    ReferenceCount *ref_ptr;

    if (reader.read_object(ptr, ref_ptr)) {
      if (!reader.resolve()) {
        if (util_cat.is_debug()) {
          util_cat.debug()
            << "Unable to fully resolve cached object in " << cache_pathname << "\n";
        }
        delete object;
      } else {
        // The object is valid.  Store it in the record.
        record->set_data(ptr, ref_ptr);
      }
    }
  }

  // Also get the total file size.
  PT(VirtualFile) vfile = din.get_vfile();
  istream &in = din.get_stream();
  in.clear();
  record->_record_size = vfile->get_file_size(&in);

  // And the last access time is now, duh.
  record->_record_access_time = time(nullptr);

  return record;
}

/**
 * Returns the appropriate filename to use for a cache file, given the
 * fullpath string to the source filename.
 */
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
  strm << std::hex << std::setw(8) << std::setfill('0') << hash;
  return strm.str();

#endif  // HAVE_OPENSSL
}

/**
 * Constructs the global BamCache object.
 */
void BamCache::
make_global() {
  _global_ptr = new BamCache;

  if (_global_ptr->_root.empty()) {
    _global_ptr->set_active(false);
  }
}
