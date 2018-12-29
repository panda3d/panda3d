/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file bamWriter.cxx
 * @author jason
 * @date 2000-06-08
 */

#include "pandabase.h"
#include "pnotify.h"

#include "typedWritable.h"
#include "config_putil.h"
#include "bam.h"
#include "bamWriter.h"
#include "bamReader.h"
#include "lightMutexHolder.h"
#include "simpleHashMap.h"

#include <algorithm>

// Keeps track of older type names in case we want to write out older .bam
// files.
struct ObsoleteName {
  std::string _name;
  int _before_major;
  int _before_minor;

  bool operator < (const ObsoleteName &other) const {
    if (_before_major != other._before_major) {
      return _before_major < other._before_major;
    }
    return _before_minor < other._before_minor;
  }
};

// This is a SimpleHashMap to avoid static init ordering issues.
static SimpleHashMap<TypeHandle, std::set<ObsoleteName> > obsolete_type_names;

/**
 *
 */
BamWriter::
BamWriter(DatagramSink *target) :
  _target(target)
{
  ++_writing_seq;
  _next_boc = BOC_adjunct;
  _needs_init = true;

  // Initialize the next object and PTA ID's.  These start counting at 1,
  // since 0 is reserved for NULL.
  _next_object_id = 1;
  _long_object_id = false;
  _next_pta_id = 1;
  _long_pta_id = false;

  // Check which version .bam files we should write.
  if (bam_version.get_num_words() > 0) {
    if (bam_version.get_num_words() != 2) {
      util_cat.error()
        << "bam-version configuration variable requires two arguments.\n";
    }
    _file_major = bam_version[0];
    _file_minor = bam_version[1];

    if (_file_major < _bam_major_ver || _file_minor < 21) {
      util_cat.error()
        << "bam-version is set to " << bam_version << ", but this version of "
           "Panda3D cannot produce .bam files older than 6.21.  Set "
           "bam-version to 6 21 in Config.prc to suppress this error, or "
           "leave it blank to write version " << _bam_major_ver << "."
           << _bam_minor_ver << " files.\n";
      _file_major = 6;
      _file_minor = 21;
      bam_version.set_string_value("6 21");

    } else if (_file_major > _bam_major_ver || _file_minor > _bam_minor_ver) {
      util_cat.error()
        << "bam-version is set to " << bam_version << ", but this version of "
           "Panda3D cannot produce .bam files newer than " << _bam_major_ver
        << "." << _bam_minor_ver << ".  Set bam-version to a supported "
           "version or leave it blank to write version " << _bam_major_ver
        << "." << _bam_minor_ver << " files.\n";

      _file_major = _bam_major_ver;
      _file_minor = _bam_minor_ver;
      bam_version.set_word(0, _bam_major_ver);
      bam_version.set_word(1, _bam_minor_ver);
    }
  } else {
    _file_major = _bam_major_ver;
    _file_minor = _bam_minor_ver;
  }
  _file_endian = bam_endian;
  _file_stdfloat_double = bam_stdfloat_double;
  _file_texture_mode = bam_texture_mode;
}

/**
 *
 */
BamWriter::
~BamWriter() {
  // Tell all the TypedWritables whose pointer we are still keeping to forget
  // about us.
  StateMap::iterator si;
  for (si = _state_map.begin(); si != _state_map.end(); ++si) {
    TypedWritable *object = (TypedWritable *)(*si).first;
    object->remove_bam_writer(this);

    if ((*si).second._refcount != nullptr) {
      unref_delete((*si).second._refcount);
    }
  }
}

/**
 * Changes the destination of future datagrams written by the BamWriter.  This
 * also implicitly calls init() if it has not already been called.
 */
void BamWriter::
set_target(DatagramSink *target) {
  if (_target != nullptr) {
    _target->flush();
  }
  _target = target;

  if (_needs_init && _target != nullptr) {
    init();
  }
}

/**
 * Initializes the BamWriter prior to writing any objects to its output
 * stream.  This includes writing out the Bam header.
 *
 * This returns true if the BamWriter successfully initialized, false
 * otherwise.
 */
bool BamWriter::
init() {
  nassertr(_target != nullptr, false);
  nassertr(_needs_init, false);
  _needs_init = false;

  // Initialize the next object and PTA ID's.  These start counting at 1,
  // since 0 is reserved for NULL.
  _next_object_id = 1;
  _long_object_id = false;
  _next_pta_id = 1;
  _long_pta_id = false;

  nassertr_always(_file_major == _bam_major_ver, false);
  nassertr_always(_file_minor <= _bam_minor_ver && _file_minor >= 21, false);

  _file_endian = bam_endian;
  _file_texture_mode = bam_texture_mode;

  // Write out the current major and minor BAM file version numbers.
  Datagram header;

  header.add_uint16(_file_major);
  header.add_uint16(_file_minor);
  header.add_uint8(_file_endian);

  if (_file_major >= 6 || _file_minor >= 27) {
    header.add_bool(_file_stdfloat_double);
  } else {
    _file_stdfloat_double = false;
  }

  if (!_target->put_datagram(header)) {
    util_cat.error()
      << "Unable to write Bam header.\n";
    return false;
  }
  return true;
}

/**
 * Writes a single object to the Bam file, so that the
 * BamReader::read_object() can later correctly restore the object and all its
 * pointers.
 *
 * This implicitly also writes any additional objects this object references
 * (if they haven't already been written), so that pointers may be fully
 * resolved.
 *
 * This may be called repeatedly to write a sequence of objects to the Bam
 * file, but typically (especially for scene graph files, indicated with the
 * .bam extension), only one object is written directly from the Bam file: the
 * root of the scene graph.  The remaining objects will all be written
 * recursively by the first object.
 *
 * Returns true if the object is successfully written, false otherwise.
 */
bool BamWriter::
write_object(const TypedWritable *object) {
  nassertr(_target != nullptr, false);

  // Increment the _writing_seq, so we can check for newly stale objects
  // during this operation.
  ++_writing_seq;

  // If there are any freed objects to indicate, write them out now.
  if (!_freed_object_ids.empty()) {
    Datagram dg;
    dg.add_uint8(BOC_remove);

    FreedObjectIds::iterator fi;
    for (fi = _freed_object_ids.begin(); fi != _freed_object_ids.end(); ++fi) {
      write_object_id(dg, (*fi));
    }
    _freed_object_ids.clear();

    if (!_target->put_datagram(dg)) {
      util_cat.error()
        << "Unable to write data to output.\n";
      return false;
    }
  }

  nassertr(_object_queue.empty(), false);
  _next_boc = BOC_push;

  int object_id = enqueue_object(object);
  nassertr(object_id != 0, false);
  if (!flush_queue()) {
    return false;
  }

  // Finally, write the closing pop.
  if (_next_boc != BOC_push) {
    Datagram dg;
    dg.add_uint8(BOC_pop);
    if (!_target->put_datagram(dg)) {
      util_cat.error()
        << "Unable to write data to output.\n";
      return false;
    }
  }

  return true;
}

/**
 * Returns true if the object has previously been written (or at least
 * requested to be written) to the bam file, or false if we've never heard of
 * it before.
 */
bool BamWriter::
has_object(const TypedWritable *object) const {
  StateMap::const_iterator si = _state_map.find(object);
  return (si != _state_map.end());
}

/**
 * Ensures that all data written thus far is manifested on the output stream.
 */
void BamWriter::
flush() {
  nassertv(_target != nullptr);
  _target->flush();
}

/**
 * Should be called from TypedWritable::update_bam_nested() to recursively
 * check the entire hiererachy of writable objects for needed updates.  This
 * tests the indicated TypedWritable object and writes it to the bam stream if
 * it has recently been modified, then recurses through update_bam_nested.
 */
void BamWriter::
consider_update(const TypedWritable *object) {
  StateMap::iterator si = _state_map.find(object);
  if (si == _state_map.end()) {
    // This object has never even been seen before.
    enqueue_object(object);

  } else if ((*si).second._written_seq.is_initial()) {
    // This object has not been written yet.
    enqueue_object(object);

  } else if ((*si).second._written_seq == _writing_seq) {
    // We have already visited this object this pass, so no need to look
    // closer.

  } else if ((*si).second._modified != object->get_bam_modified()) {
    // This object has been recently modified and needs to be rewritten.
    enqueue_object(object);

  } else {
    // Mark that we have now visited this object and pronounced it clean.
    (*si).second._written_seq = _writing_seq;

    // Recurse to child objects.
    ((TypedWritable *)object)->update_bam_nested(this);
  }
}

/**
 * The interface for writing a pointer to another object to a Bam file.  This
 * is intended to be called by the various objects that write themselves to
 * the Bam file, within the write_datagram() method.
 *
 * This writes the pointer out in such a way that the BamReader will be able
 * to restore the pointer later.  If the pointer is to an object that has not
 * yet itself been written to the Bam file, that object will automatically be
 * written.
 */
void BamWriter::
write_pointer(Datagram &packet, const TypedWritable *object) {
  // If the pointer is NULL, we always simply write a zero for an object ID
  // and leave it at that.
  if (object == nullptr) {
    write_object_id(packet, 0);

  } else {
    StateMap::iterator si = _state_map.find(object);
    if (si == _state_map.end()) {
      // We have not written this pointer out yet.  This means we must queue
      // the object definition up for later.
      int object_id = enqueue_object(object);
      write_object_id(packet, object_id);

    } else {
      // We have already assigned this pointer an ID, so it has previously
      // been written; but we might still need to rewrite it if it is stale.
      int object_id = (*si).second._object_id;
      bool already_written = !(*si).second._written_seq.is_initial();
      if ((*si).second._written_seq != _writing_seq &&
          (*si).second._modified != object->get_bam_modified()) {
        // This object was previously written, but it has since been modified,
        // so we should write it again.
        already_written = false;
      }

      write_object_id(packet, object_id);

      if (!already_written) {
        // It's stale, so queue the object for rewriting too.
        enqueue_object(object);
      } else {
        // Not stale, but maybe its child object is.
        ((TypedWritable *)object)->update_bam_nested(this);
      }
    }
  }
}

/**
 * Writes a block of auxiliary file data from the indicated file (within the
 * vfs).  This can be a block of arbitrary size, and it is assumed it may be
 * quite large.  This must be balanced by a matching call to read_file_data()
 * on restore.
 */
void BamWriter::
write_file_data(SubfileInfo &result, const Filename &filename) {
  // We write file data by preceding with a singleton datagram that contains
  // only the BOC_file_data token.
  Datagram dg;
  dg.add_uint8(BOC_file_data);
  if (!_target->put_datagram(dg)) {
    util_cat.error()
      << "Unable to write data to output.\n";
    return;
  }

  // Then we can write the file data itself, as its own (possibly quite large)
  // followup datagram.
  if (!_target->copy_datagram(result, filename)) {
    util_cat.error()
      << "Unable to write file data to output.\n";
    return;
  }

  // Both of those get written to the bam stream prior to the datagram that
  // represents this particular object, but they'll get pulled out in the same
  // order and queued up in the BamReader.
}

/**
 * Writes a block of auxiliary file data from the indicated file (outside of
 * the vfs).  This can be a block of arbitrary size, and it is assumed it may
 * be quite large.  This must be balanced by a matching call to
 * read_file_data() on restore.
 */
void BamWriter::
write_file_data(SubfileInfo &result, const SubfileInfo &source) {
  // We write file data by preceding with a singleton datagram that contains
  // only the BOC_file_data token.
  Datagram dg;
  dg.add_uint8(BOC_file_data);
  if (!_target->put_datagram(dg)) {
    util_cat.error()
      << "Unable to write data to output.\n";
    return;
  }

  // Then we can write the file data itself, as its own (possibly quite large)
  // followup datagram.
  if (!_target->copy_datagram(result, source)) {
    util_cat.error()
      << "Unable to write file data to output.\n";
    return;
  }

  // Both of those get written to the bam stream prior to the datagram that
  // represents this particular object, but they'll get pulled out in the same
  // order and queued up in the BamReader.
}

/**
 * Writes out the indicated CycleData object.  This should be used by classes
 * that store some or all of their data within a CycleData subclass, in
 * support of pipelining.  This will call the virtual
 * CycleData::write_datagram() method to do the actual writing.
 */
void BamWriter::
write_cdata(Datagram &packet, const PipelineCyclerBase &cycler) {
  const CycleData *cdata = cycler.read(Thread::get_current_thread());
  cdata->write_datagram(this, packet);
  cycler.release_read(cdata);
}

/**
 * This version of write_cdata allows passing an additional parameter to
 * cdata->write_datagram().
 */
void BamWriter::
write_cdata(Datagram &packet, const PipelineCyclerBase &cycler,
            void *extra_data) {
  const CycleData *cdata = cycler.read(Thread::get_current_thread());
  cdata->write_datagram(this, packet, extra_data);
  cycler.release_read(cdata);
}

/**
 * Prepares to write a PointerToArray to the Bam file, unifying references to
 * the same pointer across the Bam file.
 *
 * The writing object should call this prior to writing out a PointerToArray.
 * It will return true if the same pointer has been written previously, in
 * which case the writing object need do nothing further; or it will return
 * false if this particular pointer has not yet been written, in which case
 * the writing object must then write out the contents of the array.
 *
 * Also see the WRITE_PTA() macro, which consolidates the work that must be
 * done to write a PTA.
 */
bool BamWriter::
register_pta(Datagram &packet, const void *ptr) {
  if (ptr == nullptr) {
    // A zero for the PTA ID indicates a NULL pointer.  This is a special
    // case.
    write_pta_id(packet, 0);

/*
 * We return false to indicate the user must now write out the "definition" of
 * the NULL pointer.  This is necessary because of a quirk in the BamReader's
 * design, which forces callers to read the definition of every NULL pointer.
 * Presumably, the caller will be able to write the definition in a concise
 * way that will clearly indicate a NULL pointer; in the case of a
 * PointerToArray, this will generally be simply a zero element count.
 */
    return false;
  }

  PTAMap::iterator pi = _pta_map.find(ptr);
  if (pi == _pta_map.end()) {
    // We have not encountered this pointer before.
    int pta_id = _next_pta_id;
    _next_pta_id++;

    bool inserted = _pta_map.insert(PTAMap::value_type(ptr, pta_id)).second;
    nassertr(inserted, false);

    write_pta_id(packet, pta_id);

    // Return false to indicate the caller must now write out the array
    // definition.
    return false;

  } else {
    // We have encountered this pointer before.
    int pta_id = (*pi).second;
    write_pta_id(packet, pta_id);

    // Return true to indicate the caller need do nothing further.
    return true;
  }
}

/**
 * Writes a TypeHandle to the file in such a way that the BamReader can read
 * the same TypeHandle later via read_handle().
 */
void BamWriter::
write_handle(Datagram &packet, TypeHandle type) {
  // We encode TypeHandles within the Bam file by writing a unique index
  // number for each one to the file.  When we write a particular TypeHandle
  // for the first time, we assign it a new index number and then immediately
  // follow it by its definition; when we write the same TypeHandle on
  // subsequent times we only write the index number.

  // The unique number we choose is actually the internal index number of the
  // TypeHandle.  Why not?
  int index = type.get_index();

  // Also make sure the index number fits within a uint16_t.
  nassertv(index <= 0xffff);

  packet.add_uint16(index);

  if (index != 0) {
    bool inserted = _types_written.insert(index).second;

    if (inserted) {
      // This is the first time this TypeHandle has been written, so also
      // write out its definition.

      if (_file_major == _bam_major_ver && _file_minor == _bam_minor_ver) {
        packet.add_string(type.get_name());
      } else {
        // We are writing an older .bam format, so we need to look up whether
        // we may need to write an older type name.
        packet.add_string(get_obsolete_type_name(type, _file_major, _file_minor));
      }

      // We also need to write the derivation of the TypeHandle, in case the
      // program reading this file later has never heard of this type before.
      int num_parent_classes = type.get_num_parent_classes();
      nassertv(num_parent_classes <= 255);  // Good grief!
      packet.add_uint8(num_parent_classes);
      for (int i = 0; i < num_parent_classes; i++) {
        write_handle(packet, type.get_parent_class(i));
      }
    }
  }
}

/**
 * Returns the name that the given type had in an older .bam version.
 */
std::string BamWriter::
get_obsolete_type_name(TypeHandle type, int major, int minor) {
  int index = obsolete_type_names.find(type);
  if (index >= 0) {
    // Iterate over the names.  It is sorted such that the lower versions are
    // listed first.
    for (const ObsoleteName &name : obsolete_type_names.get_data((size_t)index)) {
      if (major < name._before_major ||
          (major == name._before_major && minor < name._before_minor)) {
        // We have a hit.
        return name._name;
      }
    }
  }

  return TypeRegistry::ptr()->get_name(type, nullptr);
}

/**
 * Registers the given type as having an older name in .bam files *before* the
 * indicated version.  You can call this multiple times for the same type in
 * order to establish a history of renames for this type.
 */
void BamWriter::
record_obsolete_type_name(TypeHandle type, std::string name,
                          int before_major, int before_minor) {
  // Make sure it is registered as alternate name for reading.
  TypeRegistry *reg = TypeRegistry::ptr();
  reg->record_alternate_name(type, name);

  ObsoleteName obsolete_name;
  obsolete_name._name = std::move(name);
  obsolete_name._before_major = before_major;
  obsolete_name._before_minor = before_minor;
  obsolete_type_names[type].insert(std::move(obsolete_name));
}

/**
 * This is called by the TypedWritable destructor.  It should remove the
 * pointer from any structures that keep a reference to it, and also write a
 * flag to the bam file (if it is open) so that a reader will know the object
 * id will no longer be used.
 */
void BamWriter::
object_destructs(TypedWritable *object) {
  StateMap::iterator si = _state_map.find(object);
  if (si != _state_map.end()) {
    // We ought to have written out the object by the time it destructs, or
    // we're in trouble when we do write it out.
    nassertv(!(*si).second._written_seq.is_initial());

    // This cannot be called if we are still holding a reference to it.
    nassertv((*si).second._refcount == nullptr);

    int object_id = (*si).second._object_id;
    _freed_object_ids.push_back(object_id);

    _state_map.erase(si);
  }
}

/**
 * Writes the indicated object id to the datagram.
 */
void BamWriter::
write_object_id(Datagram &dg, int object_id) {
  if (_long_object_id) {
    dg.add_uint32(object_id);

  } else {
    dg.add_uint16(object_id);
    // Once we fill up our uint16, we write all object id's thereafter with a
    // uint32.
    if (object_id == 0xffff) {
      _long_object_id = true;
    }
  }
}

/**
 * Writes the indicated pta id to the datagram.
 */
void BamWriter::
write_pta_id(Datagram &dg, int pta_id) {
  if (_long_pta_id) {
    dg.add_uint32(pta_id);

  } else {
    dg.add_uint16(pta_id);
    // Once we fill up our uint16, we write all pta id's thereafter with a
    // uint32.
    if (pta_id == 0xffff) {
      _long_pta_id = true;
    }
  }
}

/**
 * Assigns an object ID to the object and queues it up for later writing to
 * the Bam file.
 *
 * The return value is the object ID, or 0 if there is an error.
 */
int BamWriter::
enqueue_object(const TypedWritable *object) {
  Datagram dg;

  nassertr(object != TypedWritable::Null, 0);

  // No object should ever be written out that is not registered as a child of
  // TypedWritable.  The only way this can happen is if someone failed to
  // initialize their type correctly in init_type().
#ifndef NDEBUG
  if (!object->is_of_type(TypedWritable::get_class_type())) {
    util_cat.error()
      << "Type " << object->get_type()
      << " does not indicate inheritance from TypedWritable.\n"
      << "(this is almost certainly an oversight in " << object->get_type()
      << "::init_type().)\n";
  }
#endif

  // We need to assign a unique index number to every object we write out.
  // Has this object been assigned a number yet?
  int object_id;

  StateMap::iterator si = _state_map.find(object);
  if (si == _state_map.end()) {
    // No, it hasn't, so assign it the next number in sequence arbitrarily.
    object_id = _next_object_id;

    StateMap::iterator si;
    bool inserted;
    tie(si, inserted) =
      _state_map.insert(StateMap::value_type(object, StoreState(_next_object_id)));
    nassertr(inserted, false);

    // Store ourselves on the TypedWritable so that we get notified when it
    // destructs.
    (const_cast<TypedWritable*>(object))->add_bam_writer(this);
    _next_object_id++;

    // Increase the reference count if this inherits from ReferenceCount,
    // until we get a chance to write this object for the first time.
    const ReferenceCount *rc = ((TypedWritable *)object)->as_reference_count();
    if (rc != nullptr) {
      rc->ref();
      (*si).second._refcount = rc;
    }

  } else {
    // Yes, it has; get the object ID.
    object_id = (*si).second._object_id;
  }

  _object_queue.push_back(object);
  return object_id;
}

/**
 * Writes all of the objects on the _object_queue to the bam stream, until the
 * queue is empty.
 *
 * Returns true on success, false on failure.
 */
bool BamWriter::
flush_queue() {
  nassertr(_target != nullptr, false);
  // Each object we write may append more to the queue.
  while (!_object_queue.empty()) {
    const TypedWritable *object = _object_queue.front();
    _object_queue.pop_front();

    // Look up the object in the map.  It had better be there!
    StateMap::iterator si = _state_map.find(object);
    nassertr(si != _state_map.end(), false);

    if ((*si).second._written_seq == _writing_seq) {
      // We have already visited this object; no need to consider it again.
      continue;
    }

    int object_id = (*si).second._object_id;
    bool already_written = !(*si).second._written_seq.is_initial();
    if ((*si).second._modified != object->get_bam_modified()) {
      // This object was previously written, but it has since been modified,
      // so we should write it again.
      already_written = false;
    }

    Datagram dg;
    dg.set_stdfloat_double(_file_stdfloat_double);
    dg.add_uint8(_next_boc);
    _next_boc = BOC_adjunct;

    if (!already_written) {
      // The first time we write a particular object, or when we update the
      // same object later, we do so by writing its TypeHandle (which had
      // better not be TypeHandle::none(), since that's our code for a
      // previously-written object), followed by the object ID number,
      // followed by the object definition.

      TypeHandle type = object->get_type();
      nassertr(type != TypeHandle::none(), false);

      // Determine what the nearest kind of type is that the reader will be
      // able to handle, and write that instead.
      TypeHandle registered_type =
        BamReader::get_factory()->find_registered_type(type);
      if (registered_type == TypeHandle::none()) {
        // We won't be able to read this type again.
        util_cat.warning()
          << "Objects of type " << type << " cannot be read; bam file is invalid.\n";
      } else if (registered_type != type) {
        util_cat.info()
          << "Writing " << registered_type << " instead of " << type << "\n";
        type = registered_type;

      } else if (util_cat.is_debug()) {
        util_cat.debug()
          << "Writing " << type << " object id " << object_id
          << " to bam file\n";
      }

      write_handle(dg, type);
      write_object_id(dg, object_id);

      // We cast the const pointer to non-const so that we may call
      // write_datagram() on it.  Really, write_datagram() should be a const
      // method anyway, but there may be times when a class object wants to
      // update some transparent cache value during writing or something like
      // that, so it's more convenient to cheat and define it as a non-const
      // method.
      ((TypedWritable *)object)->write_datagram(this, dg);

      (*si).second._written_seq = _writing_seq;
      (*si).second._modified = object->get_bam_modified();

      // Now release any reference we hold to it, so that it may destruct.
      const ReferenceCount *rc = (*si).second._refcount;
      if (rc != nullptr) {
        // We need to assign this pointer to null before deleting the object,
        // since that may end up calling object_destructs.
        (*si).second._refcount = nullptr;
        unref_delete(rc);
      }

    } else {
      // On subsequent times when we write a particular object, we write
      // simply TypeHandle::none(), followed by the object ID. The occurrence
      // of TypeHandle::none() is an indicator to the BamReader that this is a
      // previously-written object.

      write_handle(dg, TypeHandle::none());
      write_object_id(dg, object_id);

      // The object has not been modified, but maybe one of its child objects
      // has.
      ((TypedWritable *)object)->update_bam_nested(this);
    }

    if (!_target->put_datagram(dg)) {
      util_cat.error()
        << "Unable to write data to output.\n";
      return false;
    }
  }

  return true;
}
