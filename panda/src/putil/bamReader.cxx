/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file bamReader.cxx
 * @author jason
 * @date 2000-06-12
 */

#include "pandabase.h"
#include "pnotify.h"

#include "bam.h"
#include "bamReader.h"
#include "datagramIterator.h"
#include "config_putil.h"
#include "pipelineCyclerBase.h"

using std::string;

TypeHandle BamReaderAuxData::_type_handle;

WritableFactory *BamReader::_factory = nullptr;
BamReader *const BamReader::Null = nullptr;
WritableFactory *const BamReader::NullFactory = nullptr;

BamReader::NewTypes BamReader::_new_types;

const int BamReader::_cur_major = _bam_major_ver;
const int BamReader::_cur_minor = _bam_minor_ver;


/**
 *
 */
BamReader::
BamReader(DatagramGenerator *source)
  : _source(source)
{
  _needs_init = true;
  _num_extra_objects = 0;
  _nesting_level = 0;
  _now_creating = _created_objs.end();
  _reading_cycler = nullptr;
  _pta_id = -1;
  _long_object_id = false;
  _long_pta_id = false;
}


/**
 *
 */
BamReader::
~BamReader() {
  nassertv(_num_extra_objects == 0);
  nassertv(_nesting_level == 0);
}

/**
 * Changes the source of future datagrams for this BamReader.  This also
 * implicitly calls init() if it has not already been called.
 */
void BamReader::
set_source(DatagramGenerator *source) {
  _source = source;
  if (_needs_init && _source != nullptr) {
    bool success = init();
    nassertv(success);
  }
}

/**
 * Initializes the BamReader prior to reading any objects from its source.
 * This includes reading the Bam header.
 *
 * This returns true if the BamReader successfully initialized, false
 * otherwise.
 */
bool BamReader::
init() {
  nassertr(_source != nullptr, false);
  nassertr(_needs_init, false);
  _needs_init = false;
  Datagram header;

  if (_source->is_error()) {
    return false;
  }

  if (!get_datagram(header)) {
    bam_cat.error()
      << "Unable to read Bam header.\n";
    return false;
  }

  DatagramIterator scan(header);

  _file_major = scan.get_uint16();
  _file_minor = scan.get_uint16();

  // If the major version is different, or the minor version is *newer*, we
  // can't safely load the file.
  if (_file_major != _bam_major_ver ||
      _file_minor < _bam_first_minor_ver ||
      _file_minor > _bam_minor_ver) {
    bam_cat.error()
      << "Bam file is version " << _file_major << "." << _file_minor
      << ".\n";

    if (_bam_minor_ver == _bam_first_minor_ver) {
      bam_cat.error()
        << "This program can only load version "
        << _bam_major_ver << "." << _bam_first_minor_ver << " bams.\n";
    } else {
      bam_cat.error()
        << "This program can only load version "
        << _bam_major_ver << "." << _bam_first_minor_ver << " through "
        << _bam_major_ver << "." << _bam_minor_ver << " bams.\n";
    }

    return false;
  }

  _file_endian = (BamEndian)scan.get_uint8();

  _file_stdfloat_double = false;
  if (_file_minor >= 27) {
    _file_stdfloat_double = scan.get_bool();
  }

  if (scan.get_current_index() > header.get_length()) {
    bam_cat.error()
      << "Bam header is too short.\n";
    return false;
  }

  return true;
}

/**
 * Associates an arbitrary block of data with the indicated object (or NULL),
 * and the indicated name.
 *
 * This is intended to provide a place for temporary storage for objects
 * reading themselves from the bam file.  To use it, inherit from
 * BamReader::AuxData and store whatever data you like there.  Then associate
 * your AuxData with the object as it is being read with set_aux_data().  You
 * may later set the aux data to NULL to remove it; or it will automatically
 * be removed (and deleted) after finalize() is called for the object in
 * question.
 *
 * If the TypedWritable pointer is NULL, the the aux data is stored globally
 * for the BamReader in general.  This pointer is available to any bam
 * objects, and will not be automatically removed until the BamReader itself
 * destructs.
 *
 * In either case, the name is just an arbitrary user-defined key.  If there
 * is already a data pointer stored for the obj/name pair, that data pointer
 * will be replaced (and deleted).
 */
void BamReader::
set_aux_data(TypedWritable *obj, const string &name, BamReader::AuxData *data) {
  if (data == nullptr) {
    AuxDataTable::iterator ti = _aux_data.find(obj);
    if (ti != _aux_data.end()) {
      AuxDataNames &names = (*ti).second;
      names.erase(name);
      if (names.empty()) {
        _aux_data.erase(ti);
      }
    }

  } else {
    _aux_data[obj][name] = data;
  }
}

/**
 * Returns the pointer previously associated with the bam reader by a previous
 * call to set_aux_data(), or NULL if data with the indicated key has not been
 * set.
 */
BamReader::AuxData *BamReader::
get_aux_data(TypedWritable *obj, const string &name) const {
  AuxDataTable::const_iterator ti = _aux_data.find(obj);
  if (ti != _aux_data.end()) {
    const AuxDataNames &names = (*ti).second;
    AuxDataNames::const_iterator ni = names.find(name);
    if (ni != names.end()) {
      return (*ni).second;
    }
  }

  return nullptr;
}


/**
 * Reads a single object from the Bam file.  If the object type is known, a
 * new object of the appropriate type is created and returned; otherwise, NULL
 * is returned.  NULL is also returned when the end of the file is reached.
 * is_eof() may be called to differentiate between these two cases.
 *
 * This may be called repeatedly to extract out all the objects in the Bam
 * file, but typically (especially for scene graph files, indicated with the
 * .bam extension), only one object is retrieved directly from the Bam file:
 * the root of the scene graph.  The remaining objects will all be retrieved
 * recursively by the first object.
 *
 * Note that the object returned may not yet be complete.  In particular, some
 * of its pointers may not be filled in; you must call resolve() to fill in
 * all the available pointers before you can safely use any objects returned
 * by read_object().
 *
 * This flavor of read_object() requires the caller to know what type of
 * object it has received in order to properly manage the reference counts.
 */
TypedWritable *BamReader::
read_object() {
  TypedWritable *ptr;
  ReferenceCount *ref_ptr;

  if (!read_object(ptr, ref_ptr)) {
    return nullptr;
  }

  return ptr;
}

/**
 * Reads a single object from the Bam file.
 *
 * This flavor of read_object() returns both a TypedWritable and a
 * ReferenceCount pointer to the same object, so the reference count may be
 * tracked reliably, without having to know precisely what type of object we
 * have.
 * @return true on success, or false on failure.
 */
bool BamReader::
read_object(TypedWritable *&ptr, ReferenceCount *&ref_ptr) {
  ptr = nullptr;
  ref_ptr = nullptr;
  nassertr(_num_extra_objects == 0, false);

  int start_level = _nesting_level;

  // First, read the base object.
  int object_id = p_read_object();

  // Now that object might have included some pointers to other objects, which
  // may still need to be read.  And those objects might in turn require
  // reading additional objects.  Read all the remaining objects.

  // Prior to 6.21, we kept track of _num_extra_objects to know when we're
  // done.
  while (_num_extra_objects > 0) {
    p_read_object();
    _num_extra_objects--;
  }

  // Beginning with 6.21, we use explicit nesting commands to know when we're
  // done.
  while (_nesting_level > start_level) {
    p_read_object();
  }

  // Now look up the pointer of the object we read first.  It should be
  // available now.
  if (object_id == 0) {
    if (bam_cat.is_spam()) {
      bam_cat.spam()
        << "Returning false\n";
    }
    return false;
  }

  CreatedObjs::iterator oi = _created_objs.find(object_id);

  if (oi == _created_objs.end()) {
    bam_cat.error()
      << "Undefined object encountered!\n";
    return false;

  } else {
    CreatedObj &created_obj = (*oi).second;
    ptr = created_obj._ptr;
    ref_ptr = created_obj._ref_ptr;

    if (bam_cat.is_spam()) {
      if (ptr != nullptr) {
        bam_cat.spam()
          << "Returning object of type " << ptr->get_type() << "\n";
      }
    }
    if (created_obj._change_this != nullptr ||
        created_obj._change_this_ref != nullptr) {
      bam_cat.warning()
        << "Returning pointer to " << ptr->get_type()
        << " that might change.\n";
    }

    return true;
  }
}

/**
 * This may be called at any time during processing of the Bam file to resolve
 * all the known pointers so far.  It is usually called at the end of the
 * processing, after all objects have been read, which is generally the best
 * time to call it.
 *
 * This must be called at least once after reading a particular object via
 * get_object() in order to validate that object.
 *
 * The return value is true if all objects have been resolved, or false if
 * some objects are still outstanding (in which case you will need to call
 * resolve() again later).
 */
bool BamReader::
resolve() {
  bool all_completed;
  bool any_completed_this_pass;

  do {
    if (bam_cat.is_spam()) {
      bam_cat.spam()
        << "resolve pass begin\n";
    }
    all_completed = true;
    any_completed_this_pass = false;

    ObjectPointers::iterator oi;
    oi = _object_pointers.begin();
    while (oi != _object_pointers.end()) {
      int object_id = (*oi).first;
      PointerReference &pref = (*oi).second;

      CreatedObjs::iterator ci = _created_objs.find(object_id);
      nassertr(ci != _created_objs.end(), false);

      CreatedObj &created_obj = (*ci).second;

      TypedWritable *object_ptr = created_obj._ptr;

      // Update _now_creating, so a call to get_int_tag() from within
      // complete_pointers() will come to the right place.
      CreatedObjs::iterator was_creating = _now_creating;
      _now_creating = ci;

      if (resolve_object_pointers(object_ptr, pref)) {
        // Now remove this object from the list of things that need
        // completion.  We have to be a bit careful when deleting things from
        // the STL container while we are traversing it.
        ObjectPointers::iterator old = oi;
        ++oi;
        _object_pointers.erase(old);
        any_completed_this_pass = true;

        // Does the pointer need to change?
        if (created_obj._change_this_ref != nullptr) {
          // Reference-counting variant.
          TypedWritableReferenceCount *object_ref_ptr = (TypedWritableReferenceCount *)object_ptr;
          nassertr(created_obj._ref_ptr == nullptr || created_obj._ref_ptr == object_ref_ptr, false);
          PT(TypedWritableReferenceCount) new_ptr = created_obj._change_this_ref(object_ref_ptr, this);
          if (new_ptr != object_ref_ptr) {
            // Also update the reverse
            vector_int &old_refs = _created_objs_by_pointer[object_ptr];
            vector_int &new_refs = _created_objs_by_pointer[new_ptr];
            for (vector_int::const_iterator oi = old_refs.begin();
                 oi != old_refs.end();
                 ++oi) {
              new_refs.push_back(*oi);
            }
            _created_objs_by_pointer.erase(object_ptr);

            // Remove the pointer from the finalize list (the new pointer
            // presumably doesn't require finalizing).
            _finalize_list.erase(object_ptr);
          }
          created_obj.set_ptr(new_ptr, new_ptr);
          created_obj._change_this = nullptr;
          created_obj._change_this_ref = nullptr;

        } else if (created_obj._change_this != nullptr) {
          // Non-reference-counting variant.
          TypedWritable *new_ptr = created_obj._change_this(object_ptr, this);
          if (new_ptr != object_ptr) {
            // Also update the reverse
            vector_int &old_refs = _created_objs_by_pointer[object_ptr];
            vector_int &new_refs = _created_objs_by_pointer[new_ptr];
            for (vector_int::const_iterator oi = old_refs.begin();
                 oi != old_refs.end();
                 ++oi) {
              new_refs.push_back(*oi);
            }
            _created_objs_by_pointer.erase(object_ptr);

            // Remove the pointer from the finalize list (the new pointer
            // presumably doesn't require finalizing).
            _finalize_list.erase(object_ptr);
          }
          created_obj.set_ptr(new_ptr, new_ptr->as_reference_count());
          created_obj._change_this = nullptr;
          created_obj._change_this_ref = nullptr;
        }

      } else {
        // Couldn't complete this object yet; it'll wait for next time.
        ++oi;
        all_completed = false;
      }

      _now_creating = was_creating;
    }

    if (bam_cat.is_spam()) {
      bam_cat.spam()
        << "resolve pass end: all_completed = " << all_completed
        << " any_completed_this_pass = " << any_completed_this_pass
        << "\n";
    }
  } while (!all_completed && any_completed_this_pass);

  if (all_completed) {
    finalize();
  } else {
    // Report all the uncompleted objects for no good reason.  This will
    // probably have to come out later when we have cases in which some
    // objects might legitimately be uncompleted after calling resolve(), but
    // for now we expect resolve() to always succeed.
    ObjectPointers::const_iterator oi;
    for (oi = _object_pointers.begin();
         oi != _object_pointers.end();
         ++oi) {
      int object_id = (*oi).first;
      CreatedObjs::iterator ci = _created_objs.find(object_id);
      nassertr(ci != _created_objs.end(), false);
      CreatedObj &created_obj = (*ci).second;
      TypedWritable *object_ptr = created_obj._ptr;

      bam_cat.warning()
        << "Unable to complete " << object_ptr->get_type() << "\n";
    }
  }

  return all_completed;
}

/**
 * Indicates that an object recently read from the bam stream should be
 * replaced with a new object.  Any future occurrences of the original object
 * in the stream will henceforth return the new object instead.
 *
 * The return value is true if the replacement was successfully made, or false
 * if the object was not read from the stream (or if change_pointer had
 * already been called on it).
 */
bool BamReader::
change_pointer(const TypedWritable *orig_pointer, const TypedWritable *new_pointer) {
  if (orig_pointer == new_pointer) {
    return false;
  }
  CreatedObjsByPointer::iterator ci = _created_objs_by_pointer.find(orig_pointer);
  if (ci == _created_objs_by_pointer.end()) {
    // No record of this object.
    return false;
  }

  if (bam_cat.is_spam()) {
    bam_cat.spam()
      << "change_pointer(" << (void *)orig_pointer << ", "
      << (void *)new_pointer << ") (" << new_pointer->get_type() << ")\n";
  }

  const vector_int &old_refs = (*ci).second;
  vector_int &new_refs = _created_objs_by_pointer[new_pointer];

  for (vector_int::const_iterator oi = old_refs.begin();
       oi != old_refs.end();
       ++oi) {
    int object_id = (*oi);

    CreatedObjs::iterator ci = _created_objs.find(object_id);
    nassertr(ci != _created_objs.end(), false);
    nassertr((*ci).second._ptr == orig_pointer, false);

    TypedWritable *ptr = (TypedWritable *)new_pointer;
    (*ci).second.set_ptr(ptr, ptr->as_reference_count());
    new_refs.push_back(object_id);
  }

  _created_objs_by_pointer.erase(ci);

  // Also change the pointer on the finalize_list.
  Finalize::iterator fi = _finalize_list.find((TypedWritable *)orig_pointer);
  if (fi != _finalize_list.end()) {
    _finalize_list.insert((TypedWritable *)new_pointer);
    _finalize_list.erase(fi);
  }

  return true;
}


/**
 * Reads a TypeHandle out of the Datagram.
 */
TypeHandle BamReader::
read_handle(DatagramIterator &scan) {
  // We encode TypeHandles within the Bam file by writing a unique index
  // number for each one to the file.  When we write a particular TypeHandle
  // for the first time, we assign it a new index number and then immediately
  // follow it by its definition; when we write the same TypeHandle on
  // subsequent times we only write the index number.

  // Thus, to read a TypeHandle, we first read the index number.  If it is a
  // number we have not yet encountered, we must then read the definition.

  // Here's the index number.
  int id = scan.get_uint16();

  if (id == 0) {
    // Index number 0 is always, by convention, TypeHandle::none().
    return TypeHandle::none();
  }

  IndexMap::const_iterator mi = _index_map.find(id);
  if (mi != _index_map.end()) {
    // We've encountered this index number before, so there should be no type
    // definition following the id.  Simply return the TypeHandle we
    // previously associated with the id.
    TypeHandle type = (*mi).second;
    return type;
  }

  // We haven't encountered this index number before.  This means it will be
  // immediately followed by the type definition.  This consists of the string
  // name, followed by the list of parent TypeHandles for this type.

  string name = scan.get_string();
  bool new_type = false;

  TypeHandle type = TypeRegistry::ptr()->find_type(name);
  if (type == TypeHandle::none()) {
    // We've never heard of this type before!  This is really an error
    // condition, but we'll do the best we can and declare it on-the-fly.

    type = TypeRegistry::ptr()->register_dynamic_type(name);
    bam_cat.warning()
      << "Bam file '" << get_filename() << "' contains objects of unknown type: "
      << type << "\n";
    new_type = true;
    _new_types.insert(type);
  }

  // Now pick up the derivation information.
  int num_parent_classes = scan.get_uint8();
  for (int i = 0; i < num_parent_classes; i++) {
    TypeHandle parent_type = read_handle(scan);
    if (new_type) {
      TypeRegistry::ptr()->record_derivation(type, parent_type);
    } else {
      if (type.get_parent_towards(parent_type) != parent_type) {
        if (bam_cat.is_debug()) {
          bam_cat.debug()
            << "Bam file indicates a derivation of " << type
            << " from " << parent_type << " which is no longer true.\n";
        }
      }
    }
  }

  bool inserted = _index_map.insert(IndexMap::value_type(id, type)).second;
  nassertr(inserted, type);

  if (bam_cat.is_spam()) {
    bam_cat.spam()
      << "Read TypeHandle for " << type << ".\n";
  }

  return type;
}

/**
 * The interface for reading a pointer to another object from a Bam file.
 * Objects reading themselves from a Bam file should call this when they
 * expect to read a pointer to some other object.  This function reads
 * whatever is stored in the bam file to represent the pointer, and advances
 * the datagram iterator accordingly.
 *
 * Rather than returning a pointer immediately, this function reads the
 * internal pointer information from the datagram and queues up the request.
 * The pointer itself may not be available until later (it may be a pointer to
 * an object that appears later in the Bam file).  Later, when all pointers
 * are available, the complete_pointers() callback function will be called
 * with an array of actual pointers, one for each time read_pointer() was
 * called.  It is then the calling object's responsibility to store these
 * pointers in the object properly.
 *
 * We don't know what the final pointer will be yet, but we do know whether it
 * was NULL, so this method returns true if the pointer is non-NULL, false if
 * NULL.
 */
bool BamReader::
read_pointer(DatagramIterator &scan) {
  Thread::consider_yield();

  nassertr(_now_creating != _created_objs.end(), false);
  int requestor_id = (*_now_creating).first;

  // Read the object ID, and associate it with the requesting object.
  int object_id = read_object_id(scan);

  PointerReference &pref = _object_pointers[requestor_id];
  if (_reading_cycler == nullptr) {
    // This is not being read within a read_cdata() call.
    pref._objects.push_back(object_id);
  } else {
    // This *is* being read within a read_cdata() call.
    pref._cycler_pointers[_reading_cycler].push_back(object_id);
  }

  // If the object ID is zero (which indicates a NULL pointer), we don't have
  // to do anything else.
  if (object_id != 0) {
    /*
    CreatedObj new_created_obj;
    _created_objs.insert(CreatedObjs::value_type(object_id, new_created_obj)).second;
    */

    if (get_file_minor_ver() < 21) {
      // Prior to bam version 6.21, we expect to read an adjunct object for
      // each non-NULL pointer we read.
      _num_extra_objects++;
    }

    return true;
  } else {
    return false;
  }
}

/**
 * A convenience function to read a contiguous list of pointers.  This is
 * equivalent to calling read_pointer() count times.
 */
void BamReader::
read_pointers(DatagramIterator &scan, int count) {
  for (int i = 0; i < count; i++) {
    read_pointer(scan);
  }
}

/**
 * Reads and discards a pointer value from the Bam file.  This pointer will
 * not be counted among the pointers read for a given object, and will not be
 * in the list of pointers passed to complete_pointers().
 */
void BamReader::
skip_pointer(DatagramIterator &scan) {
  read_object_id(scan);
}

/**
 * Reads a block of auxiliary file data from the Bam file.  This can be a
 * block of arbitrary size, and it is assumed it may be quite large.  Rather
 * than reading the entire block into memory, a file reference is returned to
 * locate the block on disk.  The data must have been written by a matching
 * call to write_file_data().
 */
void BamReader::
read_file_data(SubfileInfo &info) {
  // write_file_data() actually writes the blocks in datagrams prior to this
  // particular datagram.  Assume we get the calls to read_file_data() in the
  // same order as the corresponding calls to write_file_data(), and just pop
  // the first one off the queue.  There's no actual data written to the
  // stream at this point.
  nassertv(!_file_data_records.empty());
  info = _file_data_records.front();
  _file_data_records.pop_front();
}

/**
 * Reads in the indicated CycleData object.  This should be used by classes
 * that store some or all of their data within a CycleData subclass, in
 * support of pipelining.  This will call the virtual CycleData::fillin()
 * method to do the actual reading.
 */
void BamReader::
read_cdata(DatagramIterator &scan, PipelineCyclerBase &cycler) {
  PipelineCyclerBase *old_cycler = _reading_cycler;
  _reading_cycler = &cycler;
  CycleData *cdata = cycler.write(Thread::get_current_thread());
  cdata->fillin(scan, this);
  cycler.release_write(cdata);
  _reading_cycler = old_cycler;
}

/**
 * This flavor of read_cdata allows passing an additional parameter to
 * cdata->fillin().
 */
void BamReader::
read_cdata(DatagramIterator &scan, PipelineCyclerBase &cycler,
           void *extra_data) {
  PipelineCyclerBase *old_cycler = _reading_cycler;
  _reading_cycler = &cycler;
  CycleData *cdata = cycler.write(Thread::get_current_thread());
  cdata->fillin(scan, this, extra_data);
  cycler.release_write(cdata);
  _reading_cycler = old_cycler;
}

/**
 * Allows the creating object to store a temporary data value on the
 * BamReader.  This method may be called during an object's fillin() method;
 * it will associate an integer value with an arbitrary string key (which is
 * in turn associated with the calling object only). Later, in the
 * complete_pointers() method, the same object may query this data again via
 * get_int_tag().
 *
 * The tag string need not be unique between different objects, but it should
 * be unique between an object and its CData object(s).
 */
void BamReader::
set_int_tag(const string &tag, int value) {
  nassertv(_now_creating != _created_objs.end());
  int requestor_id = (*_now_creating).first;

  PointerReference &pref = _object_pointers[requestor_id];
  pref._int_tags[tag] = value;
}

/**
 * Returns the value previously set via set_int_tag(). It is an error if no
 * value has been set.
 */
int BamReader::
get_int_tag(const string &tag) const {
  nassertr(_now_creating != _created_objs.end(), 0);
  int requestor_id = (*_now_creating).first;

  ObjectPointers::const_iterator opi = _object_pointers.find(requestor_id);
  nassertr(opi != _object_pointers.end(), 0);
  const PointerReference &pref = (*opi).second;

  IntTags::const_iterator iti = pref._int_tags.find(tag);
  nassertr(iti != pref._int_tags.end(), 0);
  return (*iti).second;
}

/**
 * Allows the creating object to store a temporary data value on the
 * BamReader.  This method may be called during an object's fillin() method;
 * it will associate a newly-allocated BamReaderAuxData construct with an
 * arbitrary string key (which is in turn associated with the calling object
 * only).  Later, in the complete_pointers() method, the same object may query
 * this data again via get_aux_tag().
 *
 * The BamReader will maintain the reference count on the BamReaderAuxData,
 * and destruct it when it is cleaned up.
 *
 * The tag string need not be unique between different objects, but it should
 * be unique between an object and its CData object(s).
 */
void BamReader::
set_aux_tag(const string &tag, BamReaderAuxData *value) {
  nassertv(_now_creating != _created_objs.end());
  int requestor_id = (*_now_creating).first;

  PointerReference &pref = _object_pointers[requestor_id];
  pref._aux_tags[tag] = value;
}

/**
 * Returns the value previously set via set_aux_tag(). It is an error if no
 * value has been set.
 */
BamReaderAuxData *BamReader::
get_aux_tag(const string &tag) const {
  nassertr(_now_creating != _created_objs.end(), nullptr);
  int requestor_id = (*_now_creating).first;

  ObjectPointers::const_iterator opi = _object_pointers.find(requestor_id);
  nassertr(opi != _object_pointers.end(), nullptr);
  const PointerReference &pref = (*opi).second;

  AuxTags::const_iterator ati = pref._aux_tags.find(tag);
  nassertr(ati != pref._aux_tags.end(), nullptr);
  return (*ati).second;
}

/**
 * Should be called by an object reading itself from the Bam file to indicate
 * that this particular object would like to receive the finalize() callback
 * when all the objects and pointers in the Bam file are completely read.
 *
 * This provides a hook for objects that need to do any additional
 * finalization work after all of their related pointers are guaranteed to be
 * filled in.
 */
void BamReader::
register_finalize(TypedWritable *whom) {
  nassertv(whom != nullptr);

  if (bam_cat.is_spam()) {
    bam_cat.spam()
      << "register_finalize(" << (void *)whom << ") (" << whom->get_type()
      << ")\n";
  }

  _finalize_list.insert(whom);
}

/**
 * Called by an object reading itself from the bam file to indicate that the
 * object pointer that will be returned is temporary, and will eventually need
 * to be replaced with another pointer.
 *
 * The supplied function pointer will later be called on the object,
 * immediately after complete_pointers() is called; it should return the new
 * and final pointer.
 *
 * We use a static function pointer instead of a virtual function (as in
 * finalize()), to allow the function to destruct the old pointer if
 * necessary.  (It is invalid to destruct the this pointer within a virtual
 * function.)
 */
void BamReader::
register_change_this(ChangeThisFunc func, TypedWritable *object) {
  nassertv(_now_creating != _created_objs.end());
  CreatedObj &created_obj = (*_now_creating).second;

#ifndef NDEBUG
  // Sanity check the pointer--it should always be the same pointer after we
  // set it the first time.
  if (created_obj._ptr == nullptr) {
    created_obj.set_ptr(object, object->as_reference_count());
  } else {
    // We've previously assigned this pointer, and we should have assigned it
    // to the same this pointer we have now.
    nassertv(created_obj._ptr == object);
  }
#endif  // NDEBUG

  created_obj._change_this = func;
  created_obj._change_this_ref = nullptr;
}

/**
 * Called by an object reading itself from the bam file to indicate that the
 * object pointer that will be returned is temporary, and will eventually need
 * to be replaced with another pointer.
 *
 * The supplied function pointer will later be called on the object,
 * immediately after complete_pointers() is called; it should return the new
 * and final pointer.
 *
 * We use a static function pointer instead of a virtual function (as in
 * finalize()), to allow the function to destruct the old pointer if
 * necessary.  (It is invalid to destruct the this pointer within a virtual
 * function.)
 */
void BamReader::
register_change_this(ChangeThisRefFunc func, TypedWritableReferenceCount *object) {
  nassertv(_now_creating != _created_objs.end());
  CreatedObj &created_obj = (*_now_creating).second;

#ifndef NDEBUG
  // Sanity check the pointer--it should always be the same pointer after we
  // set it the first time.
  if (created_obj._ptr == nullptr) {
    created_obj.set_ptr(object, object);
  } else {
    // We've previously assigned this pointer, and we should have assigned it
    // to the same this pointer we have now.
    nassertv(created_obj._ptr == object);
    nassertv(created_obj._ref_ptr == object);
  }
#endif  // NDEBUG

  created_obj._change_this = nullptr;
  created_obj._change_this_ref = func;
}

/**
 * Forces the finalization of a particular object.  This may be called by any
 * of the objects during finalization, to guarantee finalization ordering
 * where it is important.
 */
void BamReader::
finalize_now(TypedWritable *whom) {
  if (whom == nullptr) {
    return;
  }

  Finalize::iterator fi = _finalize_list.find(whom);
  if (fi != _finalize_list.end()) {
    _finalize_list.erase(fi);
    if (bam_cat.is_spam()) {
      bam_cat.spam()
        << "finalizing " << (void *)whom << " (" << whom->get_type()
        << ")\n";
    }
    whom->finalize(this);
  }
}

/**
 * This function works in conjection with register_pta(), below, to read a
 * PointerToArray (PTA) from the Bam file, and unify references to the same
 * PTA.
 *
 * The first time get_pta() encounters a particular PTA, it will return NULL.
 * This is the indication that the caller should then read in the data
 * associated with the PTA, and subsequently call register_pta() with the
 * address of the filled-in array.
 *
 * The next time (and all subsequent times) that get_pta() encounters this
 * same PTA, it will return the pointer that was passed with register_pta().
 *
 * Also see the READ_PTA() macro, which consolidates all the work that must be
 * done to read a PTA.
 */
void *BamReader::
get_pta(DatagramIterator &scan) {
  nassertr(_pta_id == -1, nullptr);
  int id = read_pta_id(scan);

  if (id == 0) {
    // As always, a 0 ID indicates a NULL pointer.  The caller will not be
    // able to differentiate this case from that of a previously-read pointer,
    // but that's OK because the next data in the Bam file is the length of
    // the array, which will be zero--indicating an empty or NULL array.
    return nullptr;
  }

  PTAMap::iterator pi = _pta_map.find(id);
  if (pi == _pta_map.end()) {
    // This is the first time we've encountered this particular ID, meaning we
    // need to read the data now and register it.
    _pta_id = id;
    return nullptr;
  }

  return (*pi).second;
}

/**
 * The second part of read_pta(), this should be called with the pointer to
 * the array that was read in after read_pta() returned NULL.  This associates
 * the pointer with the ID that was previously read, so that future calls to
 * read_pta() will return the same pointer.
 *
 * Also see the READ_PTA() macro, which consolidates all the work that must be
 * done to read a PTA.
 */
void BamReader::
register_pta(void *ptr) {
  if (_pta_id != -1) {
    bool inserted = _pta_map.insert(PTAMap::value_type(_pta_id, ptr)).second;
    _pta_id = -1;
    nassertv(inserted);
  }
}

/**
 * Handles a record that begins with the _remove_flag TypeHandle; this
 * contains a list of object ID's that will no longer be used in this file and
 * can safely be removed.
 */
void BamReader::
free_object_ids(DatagramIterator &scan) {
  // We have to fully complete any objects before we remove them.  Might as
  // well try to complete everything before we get started.
  resolve();

  while (scan.get_remaining_size() > 0) {
    int object_id = read_object_id(scan);

    CreatedObjs::iterator ci = _created_objs.find(object_id);
    if (ci == _created_objs.end()) {
      util_cat.warning()
        << "Bam file suggests eliminating object_id " << object_id
        << ", already gone.\n";
    } else {

      // Make sure the object was successfully completed.
      ObjectPointers::iterator oi = _object_pointers.find(object_id);
      if (oi != _object_pointers.end()) {
        util_cat.warning()
          << "Unable to resolve object " << object_id
          << " before removing from table.\n";
      }

      _created_objs_by_pointer.erase((*ci).second._ptr);
      _created_objs.erase(ci);
    }
  }
}


/**
 * Reads an object id from the datagram.
 */
int BamReader::
read_object_id(DatagramIterator &scan) {
  int object_id;

  if (_long_object_id) {
    object_id = scan.get_uint32();

  } else {
    object_id = scan.get_uint16();
    if (object_id == 0xffff) {
      _long_object_id = true;
    }
  }

  return object_id;
}

/**
 * Reads an pta id from the datagram.
 */
int BamReader::
read_pta_id(DatagramIterator &scan) {
  int pta_id;

  if (_long_pta_id) {
    pta_id = scan.get_uint32();

  } else {
    pta_id = scan.get_uint16();
    if (pta_id == 0xffff) {
      _long_pta_id = true;
    }
  }

  return pta_id;
}

/**
 * The private implementation of read_object(); this reads an object from the
 * file and returns its object ID.
 */
int BamReader::
p_read_object() {
  Datagram dg;

  // First, read a datagram for the object.
  if (!get_datagram(dg)) {
    // When we run out of datagrams, we're at the end of the file.
    if (bam_cat.is_debug()) {
      bam_cat.debug()
        << "Reached end of bam source.\n";
    }
    return 0;
  }

  // Now extract the object definition from the datagram.
  DatagramIterator scan(dg);

  // First, read the BamObjectCode.  In bam versions prior to 6.21, there was
  // no BamObjectCode in the stream.
  BamObjectCode boc = BOC_adjunct;
  if (get_file_minor_ver() >= 21) {
    boc = (BamObjectCode)scan.get_uint8();
  }
  switch (boc) {
  case BOC_push:
    ++_nesting_level;
    break;

  case BOC_pop:
    --_nesting_level;
    return 0;

  case BOC_adjunct:
    break;

  case BOC_remove:
    // The BOC_remove code is a special case; it begins a record that simply
    // lists all of the object ID's that are no longer important to the file
    // and may be released.
    free_object_ids(scan);

    // Now that we've freed all of the object id's indicate, read the next
    // object id in the stream.  It's easiest to do this by calling
    // recursively.
    return p_read_object();

  case BOC_file_data:
    // Another special case.  This marks an auxiliary file data record that we
    // skip over for now, but we note its position within the stream, so that
    // we can hand it to a future object who may request it.
    {
      SubfileInfo info;
      if (!_source->save_datagram(info)) {
        bam_cat.error()
          << "Failed to read file data.\n";
        return 0;
      }
      _file_data_records.push_back(info);
    }

    return p_read_object();

  default:
    bam_cat.error()
      << "Encountered invalid BamObjectCode 0x" << std::hex << (int)boc << std::dec << ".\n";
    return 0;
  }

  // An object definition in a Bam file consists of a TypeHandle definition,
  // defining the object's type, followed by an object ID index, defining the
  // particular instance (e.g.  pointer) of this object.

  TypeHandle type = read_handle(scan);

  int object_id = read_object_id(scan);

  if (scan.get_current_index() > dg.get_length()) {
    bam_cat.error()
      << "Found truncated datagram in bam stream\n";
    return 0;
  }

  // There are two cases (not counting the special _remove_flag case, above).
  // Either this is a new object definition, or this is a reference to an
  // object that was previously defined.

  // We use the TypeHandle to differentiate these two cases.  By convention,
  // we write a TypeHandle::none() to the Bam file when we are writing a
  // reference to a previously-defined object, but we write the object's
  // actual type when we are writing its definition right now.

  // Therefore, if the type is TypeHandle::none(), then we must have already
  // read in and created the object (although its pointers may not be fully
  // instantiated yet).  On the other hand, if the type is anything else, then
  // we must read the definition to follow.

  if (type != TypeHandle::none()) {
    // Now we are going to read and create a new object.

    // First, we must add an entry into the map for this object ID, so that in
    // case this function is called recursively during the object's factory
    // constructor, we will have some definition for the object.  For now, we
    // give it a NULL pointer.
    CreatedObj new_created_obj;
    CreatedObjs::iterator oi =
      _created_objs.insert(CreatedObjs::value_type(object_id, new_created_obj)).first;
    CreatedObj &created_obj = (*oi).second;

    if (created_obj._ptr != nullptr) {
      // This object had already existed; thus, we are just receiving an
      // update for it.

      if (_object_pointers.find(object_id) != _object_pointers.end()) {
        // Aieee! This object isn't even complete from the last time we
        // encountered it in the stream! This should never happen. Something's
        // corrupt or the stream was maliciously crafted.
        bam_cat.error()
          << "Found object " << object_id << " in bam stream again while "
          << "trying to resolve its own pointers.\n";
        return 0;
      }

      // Update _now_creating during this call so if this function calls
      // read_pointer() or register_change_this() we'll match it up properly.
      // This might recursively call back into this p_read_object(), so be
      // sure to save and restore the original value of _now_creating.
      CreatedObjs::iterator was_creating = _now_creating;
      _now_creating = oi;
      created_obj._ptr->fillin(scan, this);
      _now_creating = was_creating;

      if (scan.get_remaining_size() > 0) {
        bam_cat.warning()
          << "Skipping " << scan.get_remaining_size() << " remaining bytes "
          << "in datagram containing type " << type << "\n";
      }

    } else {
      // We are receiving a new object.  Now we can call the factory to create
      // the object.

      // Define the parameters for passing to the object factory.
      FactoryParams fparams;
      fparams.add_param(new BamReaderParam(scan, this));

      // As above, we update and preserve _now_creating during this call.
      CreatedObjs::iterator was_creating = _now_creating;
      _now_creating = oi;
      TypedWritable *object =
        _factory->make_instance_more_general(type, fparams);
      _now_creating = was_creating;

      // And now we can store the new object pointer in the map.
      nassertr(created_obj._ptr == object || created_obj._ptr == nullptr, object_id);
      if (object == nullptr) {
        created_obj.set_ptr(nullptr, nullptr);
      } else {
        created_obj.set_ptr(object, object->as_reference_count());
      }
      created_obj._created = true;

      if (created_obj._change_this_ref != nullptr) {
        // If the pointer is scheduled to change after complete_pointers(),
        // but we have no entry in _object_pointers for this object (and hence
        // no plan to call complete_pointers()), then just change the pointer
        // immediately.
        ObjectPointers::const_iterator ri = _object_pointers.find(object_id);
        if (ri == _object_pointers.end()) {
          PT(TypedWritableReferenceCount) object_ref = (*created_obj._change_this_ref)((TypedWritableReferenceCount *)object, this);
          TypedWritable *new_ptr = object_ref;
          created_obj.set_ptr(object_ref, object_ref);
          created_obj._change_this = nullptr;
          created_obj._change_this_ref = nullptr;

          // Remove the pointer from the finalize list (the new pointer
          // presumably doesn't require finalizing).
          if (new_ptr != object) {
            _finalize_list.erase(object);
          }
          object = new_ptr;
        }

      } else if (created_obj._change_this != nullptr) {
        // Non-reference-counting variant.
        ObjectPointers::const_iterator ri = _object_pointers.find(object_id);
        if (ri == _object_pointers.end()) {
          TypedWritable *new_ptr = (*created_obj._change_this)(object, this);
          created_obj.set_ptr(new_ptr, new_ptr->as_reference_count());
          created_obj._change_this = nullptr;
          created_obj._change_this_ref = nullptr;

          if (new_ptr != object) {
            _finalize_list.erase(object);
          }
          object = new_ptr;
        }
      }

      _created_objs_by_pointer[created_obj._ptr].push_back(object_id);

      // Just some sanity checks
      if (object == nullptr) {
        if (bam_cat.is_debug()) {
          bam_cat.debug()
            << "Unable to create an object of type " << type << std::endl;
        }

      } else if (object->get_type() != type) {
        if (_new_types.find(type) != _new_types.end()) {
          // This was a type we hadn't heard of before, so it's not really
          // surprising we didn't know how to create it.  Suppress the warning
          // (make it a debug statement instead).
          if (bam_cat.is_debug()) {
            bam_cat.warning()
              << "Attempted to create a " << type.get_name()    \
              << " but a " << object->get_type()                \
              << " was created instead." << std::endl;
          }

        } else {
          // This was a normal type that we should have known how to create.
          // Report the error.
          bam_cat.warning()
            << "Attempted to create a " << type.get_name()      \
            << " but a " << object->get_type()                  \
            << " was created instead." << std::endl;
        }

      } else {
        if (bam_cat.is_spam()) {
          bam_cat.spam()
            << "Read a " << object->get_type() << ": " << (void *)object
            << " (id=" << object_id << ")\n";
        }
      }
    }

    // Sanity check that we read the expected number of bytes.
    if (scan.get_current_index() > dg.get_length()) {
      bam_cat.error()
        << "End of datagram reached while reading bam object "
        << type << ": " << (void *)created_obj._ptr << "\n";
    }
  }

  return object_id;
}

/**
 * Checks whether all of the pointers a particular object is waiting for have
 * been filled in yet.  If they have, calls complete_pointers() on the object
 * and returns true; otherwise, returns false.
 */
bool BamReader::
resolve_object_pointers(TypedWritable *object,
                        BamReader::PointerReference &pref) {
  // Some objects further require all of their nested objects to have been
  // completed (i.e.  complete_pointers has been called on each nested object)
  // before they can themselves be completed.
  bool require_fully_complete = object->require_fully_complete();

  // First do the PipelineCycler objects.
  CyclerPointers::iterator ci;
  ci = pref._cycler_pointers.begin();
  while (ci != pref._cycler_pointers.end()) {
    PipelineCyclerBase *cycler = (*ci).first;
    const vector_int &pointer_ids = (*ci).second;

    if (resolve_cycler_pointers(cycler, pointer_ids, require_fully_complete)) {
      // Now remove this cycler from the list of things that need completion.
      // We have to be a bit careful when deleting things from the STL
      // container while we are traversing it.
      CyclerPointers::iterator old = ci;
      ++ci;
      pref._cycler_pointers.erase(old);

    } else {
      // Couldn't complete this cycler yet; it'll wait for next time.
      ++ci;
    }
  }

  if (!pref._cycler_pointers.empty()) {
    // If we didn't get all the cyclers, we have to wait.
    if (bam_cat.is_spam()) {
      bam_cat.spam()
        << "some cyclers pending: complete_pointers for " << (void *)object
        << " (" << object->get_type() << ")\n";
    }
    return false;
  }

  // Now make sure we have all of the pointers this object is waiting for.  If
  // any of the pointers has not yet been read in, we can't resolve this
  // object--we can't do anything for a given object until we have *all*
  // outstanding pointers for that object.
  bool is_complete = true;

  vector_typedWritable references;
  references.reserve(pref._objects.size());

  vector_int::const_iterator pi;
  for (pi = pref._objects.begin();
       pi != pref._objects.end() && is_complete;
       ++pi) {
    int child_id = (*pi);
    if (child_id == 0) {
      // A NULL pointer is a NULL pointer.
      references.push_back(nullptr);
      continue;
    }

    // See if we have the pointer available now.
    CreatedObjs::const_iterator oi = _created_objs.find(child_id);
    if (oi == _created_objs.end()) {
      // No, too bad.
      is_complete = false;
      break;
    }

    const CreatedObj &child_obj = (*oi).second;
    if (!child_obj._created) {
      // The child object hasn't yet been created.
      is_complete = false;
      break;
    }

    if (child_obj._change_this != nullptr || child_obj._change_this_ref != nullptr) {
      // It's been created, but the pointer might still change.
      is_complete = false;
      break;
    }

    if (require_fully_complete &&
        _object_pointers.find(child_id) != _object_pointers.end()) {
      // It's not yet complete itself.
      is_complete = false;
      break;
    }

    // Yes, it's ready.
    references.push_back(child_obj._ptr);
  }

  if (is_complete) {
    // Okay, here's the complete list of pointers for you!
    nassertr(references.size() == pref._objects.size(), false);

    if (bam_cat.is_spam()) {
      bam_cat.spam()
        << "complete_pointers for " << (void *)object
        << " (" << object->get_type() << "), " << references.size()
        << " pointers.\n";
    }
    int num_completed = 0;
    if (!references.empty()) {
      num_completed = object->complete_pointers(&references[0], this);
    }
    if (num_completed != (int)references.size()) {
      bam_cat.warning()
        << object->get_type() << " completed " << num_completed
        << " of " << references.size() << " pointers.\n";
      nassertr(num_completed < (int)references.size(), true);
    }
    return true;

  } else {
    if (bam_cat.is_spam()) {
      bam_cat.spam()
        << "not ready: complete_pointers for " << (void *)object
        << " (" << object->get_type() << ")\n";
    }
  }

  return false;
}

/**
 * Checks whether all of the pointers a particular PipelineCycler is waiting
 * for have been filled in yet.  If they have, calls complete_pointers() on
 * the cycler and returns true; otherwise, returns false.
 */
bool BamReader::
resolve_cycler_pointers(PipelineCyclerBase *cycler,
                        const vector_int &pointer_ids,
                        bool require_fully_complete) {
  // Now make sure we have all of the pointers this cycler is waiting for.  If
  // any of the pointers has not yet been read in, we can't resolve this
  // cycler--we can't do anything for a given cycler until we have *all*
  // outstanding pointers for that cycler.

  bool is_complete = true;
  vector_typedWritable references;

  vector_int::const_iterator pi;
  for (pi = pointer_ids.begin(); pi != pointer_ids.end() && is_complete; ++pi) {
    int child_id = (*pi);

    if (child_id == 0) {
      // A NULL pointer is a NULL pointer.
      references.push_back(nullptr);
      continue;
    }

    // See if we have the pointer available now.
    CreatedObjs::const_iterator oi = _created_objs.find(child_id);
    if (oi == _created_objs.end()) {
      // No, too bad.
      is_complete = false;
      break;
    }

    const CreatedObj &child_obj = (*oi).second;
    if (child_obj._change_this != nullptr || child_obj._change_this_ref != nullptr) {
      // It's been created, but the pointer might still change.
      is_complete = false;
      break;
    }

    if (require_fully_complete &&
        _object_pointers.find(child_id) != _object_pointers.end()) {
      // It's not yet complete itself.
      is_complete = false;
      break;
    }

    // Yes, it's ready.
    references.push_back(child_obj._ptr);
  }

  if (is_complete) {
    // Okay, here's the complete list of pointers for you!
    CycleData *cdata = cycler->write(Thread::get_current_thread());
    if (bam_cat.is_spam()) {
      bam_cat.spam()
        << "complete_pointers for CycleData object " << (void *)cdata
        << "\n";
    }
    int num_completed = cdata->complete_pointers(&references[0], this);
    cycler->release_write(cdata);
    if (num_completed != (int)references.size()) {
      bam_cat.warning()
        << "CycleData object completed " << num_completed
        << " of " << references.size() << " pointers.\n";
      nassertr(num_completed < (int)references.size(), true);
    }
    return true;
  }

  return false;
}

/**
 * Should be called after all objects have been read, this will finalize all
 * the objects that registered themselves for the finalize callback.
 */
void BamReader::
finalize() {
  if (bam_cat.is_debug()) {
    bam_cat.debug()
      << "Finalizing bam source\n";
  }

  Finalize::iterator fi = _finalize_list.begin();
  while (fi != _finalize_list.end()) {
    TypedWritable *object = (*fi);
    nassertv(object != nullptr);
    _finalize_list.erase(fi);
    if (bam_cat.is_spam()) {
      bam_cat.spam()
        << "finalizing " << (void *)object << " (" << object->get_type()
        << ")\n";
    }
    object->finalize(this);
    _aux_data.erase(object);
    fi = _finalize_list.begin();
  }

  // Now clear the aux data of all objects, except the NULL object.
  if (!_aux_data.empty()) {
    AuxDataTable::iterator ti = _aux_data.find(nullptr);

    if (ti != _aux_data.end()) {
      if (_aux_data.size() > 1) {
        // Move the NULL data to the new table; remove the rest.
        AuxDataTable new_aux_data;
        AuxDataTable::iterator nti =
          new_aux_data.insert(AuxDataTable::value_type(nullptr, AuxDataNames())).first;
        (*nti).second.swap((*ti).second);
        _aux_data.swap(new_aux_data);
      }
    } else {
      // There's no NULL data; clear the whole table.
      _aux_data.clear();
    }
  }
}
