// Filename: bamReader.cxx
// Created by:  jason (12Jun00)
//

#include <pandabase.h>
#include <notify.h>

#include "bam.h"
#include "bamReader.h"
#include <datagramIterator.h>
#include "config_util.h"

WritableFactory *BamReader::_factory = (WritableFactory*)0L;
BamReader *const BamReader::Null = (BamReader*)0L;
WritableFactory *const BamReader::NullFactory = (WritableFactory*)0L;

const int BamReader::_cur_major = _bam_major_ver;
const int BamReader::_cur_minor = _bam_minor_ver;


////////////////////////////////////////////////////////////////////
//     Function: BamReader::Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
BamReader::
BamReader(DatagramGenerator *generator) 
  : _source(generator)
{
  _num_extra_objects = 0;
  _pta_id = -1;
}


////////////////////////////////////////////////////////////////////
//     Function: BamReader::Destructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
BamReader::
~BamReader() {
}

////////////////////////////////////////////////////////////////////
//     Function: BamReader::init
//       Access: Public
//  Description: Initializes the BamReader prior to reading any
//               objects from its source.  This includes reading the
//               Bam header.
//
//               This returns true if the BamReader successfully
//               initialized, false otherwise.
////////////////////////////////////////////////////////////////////
bool BamReader::
init() {
  Datagram header;

  if (_source->is_error()) {
    return false;
  }

  if (!_source->get_datagram(header)) {
    bam_cat.error()
      << "Unable to read Bam header.\n";
    return false;
  }

  DatagramIterator scan(header);

  _file_major = scan.get_uint16();
  _file_minor = scan.get_uint16();

  // If the major version is different, or the minor version is
  // *newer*, we can't safely load the file.
  if (_file_major != _bam_major_ver || _file_minor > _bam_minor_ver) {
    bam_cat.error()
      << "Bam file is version " << _file_major << "." << _file_minor
      << ".\n";

    if (_bam_minor_ver == 0) {
      bam_cat.error()
        << "This program can only load version " 
        << _bam_major_ver << ".0 bams.\n";
    } else {
      bam_cat.error()
        << "This program can only load version " 
        << _bam_major_ver << ".0 through " 
        << _bam_major_ver << "." << _bam_minor_ver << " bams.\n";
    }
      
    return false;
  }

  if (bam_cat.is_debug()) {
    bam_cat.debug() 
      << "Bam file is version " << _file_major << "." << _file_minor
      << ".\n";
    if (_file_minor != _bam_minor_ver) {
      bam_cat.debug()
        << "(Current version is " << _bam_major_ver << "." << _bam_minor_ver
        << ".)\n";
    }
  }

  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: BamReader::read_object
//       Access: Public
//  Description: Reads a single object from the Bam file.  If the
//               object type is known, a new object of the appropriate
//               type is created and returned; otherwise, NULL is
//               returned.  NULL is also returned when the end of the
//               file is reached.  is_eof() may be called to
//               differentiate between these two cases.
//
//               This may be called repeatedly to extract out all the
//               objects in the Bam file, but typically (especially
//               for scene graph files, indicated with the .bam
//               extension), only one object is retrieved directly
//               from the Bam file: the root of the scene graph.  The
//               remaining objects will all be retrieved recursively
//               by the first object.
//
//               Note that the object returned may not yet be
//               complete.  In particular, some of its pointers may
//               not be filled in; you must call resolve() to fill in
//               all the available pointers before you can safely use
//               any objects returned by read_object().
////////////////////////////////////////////////////////////////////
TypedWritable *BamReader::
read_object() {
  nassertr(_num_extra_objects == 0, (TypedWritable *)NULL);

  // First, read the base object.
  int object_id = p_read_object();

  // Now that object might have included some pointers to other
  // objects, which may still need to be read.  And those objects
  // might in turn require reading additional objects.  Read all the
  // remaining objects.
  while (_num_extra_objects > 0) {
    p_read_object();
    _num_extra_objects--;
  }

  // Now look up the pointer of the object we read first.  It should
  // be available now.
  if (object_id == 0) {
    if (bam_cat.is_spam()) {
      bam_cat.spam()
        << "Returning NULL\n";
    }
    return (TypedWritable *)NULL;
  }

  CreatedObjs::iterator oi = _created_objs.find(object_id);
    
  if (oi == _created_objs.end()) {
    bam_cat.error() 
      << "Undefined object encountered!\n";
    return (TypedWritable *)NULL;

  } else {
    TypedWritable *object = (*oi).second;

    if (bam_cat.is_spam()) {
      if (object != (TypedWritable *)NULL) {
        bam_cat.spam()
          << "Returning object of type " << object->get_type() << "\n";
      }
    }
    
    return object;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: BamReader::resolve
//       Access: Public
//  Description: This may be called at any time during processing of
//               the Bam file to resolve all the known pointers so
//               far.  It is usually called at the end of the
//               processing, after all objects have been read, which
//               is generally the best time to call it.
//
//               This must be called at least once after reading a
//               particular object via get_object() in order to
//               validate that object.
//
//               The return value is true if all objects have been
//               resolved, or false if some objects are still
//               outstanding (in which case you will need to call
//               resolve() again later).
////////////////////////////////////////////////////////////////////
bool BamReader::
resolve() {
  bool all_completed = true;

  // Walk through all the objects that still have outstanding pointers.
  Requests::iterator ri;
  ri = _deferred_pointers.begin(); 
  while (ri != _deferred_pointers.end()) {
    TypedWritable *whom = (*ri).first;
    const vector_ushort &pointers = (*ri).second;

    // Now make sure we have all of the pointers this object is
    // waiting for.  If any of the pointers has not yet been read in,
    // we can't resolve this object--we can't do anything for a given
    // object until we have *all* outstanding pointers for that
    // object.

    bool is_complete = true;
    vector_typedWritable references;

    vector_ushort::const_iterator pi;
    for (pi = pointers.begin(); pi != pointers.end() && is_complete; ++pi) {
      int object_id = (*pi);

      if (object_id == 0) {
        // A NULL pointer is a NULL pointer.
        references.push_back((TypedWritable *)NULL);

      } else {
        // See if we have the pointer available now.
        CreatedObjs::const_iterator oi = _created_objs.find(object_id);
        if (oi == _created_objs.end()) {
          // No, too bad.
          is_complete = false;

        } else {
          // Yes, it's ready.
          references.push_back((*oi).second);
        }
      }
    }

    if (is_complete) {
      // Okay, here's the complete list of pointers for you!
      whom->complete_pointers(references, this);

      // Now remove this object from the list of things that need
      // completion.  We have to be a bit careful when deleting things
      // from the STL container while we are traversing it.
      Requests::iterator old = ri;
      ++ri;
      _deferred_pointers.erase(old);

    } else {
      // Couldn't complete this object yet; it'll wait for next time.
      bam_cat.warning()
        << "Unable to complete " << whom->get_type() << "\n";
      ++ri;
      all_completed = false;
    }
  }

  if (all_completed) {
    finalize();
  }

  return all_completed;
}

////////////////////////////////////////////////////////////////////
//     Function: BamReader::read_handle
//       Access: Public
//  Description: Reads a TypeHandle out of the Datagram.
////////////////////////////////////////////////////////////////////
TypeHandle BamReader::
read_handle(DatagramIterator &scan) {
  // We encode TypeHandles within the Bam file by writing a unique
  // index number for each one to the file.  When we write a
  // particular TypeHandle for the first time, we assign it a new
  // index number and then immediately follow it by its definition;
  // when we write the same TypeHandle on subsequent times we only
  // write the index number.

  // Thus, to read a TypeHandle, we first read the index number.  If
  // it is a number we have not yet encountered, we must then read the
  // definition.

  // Here's the index number.
  int id = scan.get_uint16();
  
  if (id == 0) {
    // Index number 0 is always, by convention, TypeHandle::none().
    return TypeHandle::none();
  }

  IndexMap::const_iterator mi = _index_map.find(id);
  if (mi != _index_map.end()) {
    // We've encountered this index number before, so there should be
    // no type definition following the id.  Simply return the
    // TypeHandle we previously associated with the id.
    TypeHandle type = (*mi).second;
    return type;
  }

  // We haven't encountered this index number before.  This means it
  // will be immediately followed by the type definition.  This
  // consists of the string name, followed by the list of parent
  // TypeHandles for this type.

  string name = scan.get_string();
  bool new_type = false;

  TypeHandle type = TypeRegistry::ptr()->find_type(name);
  if (type == TypeHandle::none()) {
    // We've never heard of this type before!  This is really an error
    // condition, but we'll do the best we can and declare it
    // on-the-fly.

    type = TypeRegistry::ptr()->register_dynamic_type(name);
    bam_cat.warning()
      << "Bam file contains objects of unknown type: " << type << "\n";
    new_type = true;
  }

  // Now pick up the derivation information.
  int num_parent_classes = scan.get_uint8();
  for (int i = 0; i < num_parent_classes; i++) {
    TypeHandle parent_type = read_handle(scan);
    if (new_type) {
      TypeRegistry::ptr()->record_derivation(type, parent_type);
    } else {
      if (type.get_parent_towards(parent_type) != parent_type) {
        bam_cat.warning()
          << "Bam file indicates a derivation of " << type 
          << " from " << parent_type << " which is no longer true.\n";
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

////////////////////////////////////////////////////////////////////
//     Function: BamReader::read_pointer
//       Access: Public
//  Description: The interface for reading a pointer to another object
//               from a Bam file.  Objects reading themselves from a
//               Bam file should call this when they expect to read a
//               pointer, passing in the datagram iterator and a
//               pointer to their own object, i.e. 'this'.
//
//               Rather than returning a pointer immediately, this
//               function reads the internal pointer information from
//               the datagram and queues up the request.  The pointer
//               itself may not be available until later (it may be a
//               pointer to an object that appears later in the Bam
//               file).  Later, when all pointers are available, the
//               complete_pointers() callback function will be called
//               with an array of actual pointers, one for time
//               read_pointer() was called.  It is then the calling
//               object's responsibilty to store these pointers in the
//               object properly.
////////////////////////////////////////////////////////////////////
void BamReader::
read_pointer(DatagramIterator &scan, TypedWritable *for_whom) {
  // Read the object ID, and associate it with the requesting object.
  int object_id = scan.get_uint16();
  _deferred_pointers[for_whom].push_back(object_id);

  // If the object ID is zero (which indicates a NULL pointer), we
  // don't have to do anything else.
  if (object_id != 0) {
    if (_created_objs.count(object_id) == 0) {
      // If we don't already have an entry in the map for this object
      // ID (that is, we haven't encountered this object before), we
      // must remember to read the object definition later.
      _num_extra_objects++;
    }
  }
}

////////////////////////////////////////////////////////////////////
//     Function: BamReader::read_pointers
//       Access: Public
//  Description: A convenience function to read a contiguous list of
//               pointers.  This is equivalent to calling
//               read_pointer() count times.
////////////////////////////////////////////////////////////////////
void BamReader::
read_pointers(DatagramIterator &scan, TypedWritable *for_whom, int count) {
  for (int i = 0; i < count; i++) {
    read_pointer(scan, for_whom);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: BamReader::skip_pointer
//       Access: Public
//  Description: Reads and discards a pointer value from the Bam file.
//               This pointer will not be counted among the pointers
//               read for a given object, and will not be in the list
//               of pointers passed to complete_pointers().
////////////////////////////////////////////////////////////////////
void BamReader::
skip_pointer(DatagramIterator &scan) {
  scan.get_uint16();
}

////////////////////////////////////////////////////////////////////
//     Function: BamReader::register_finalize
//       Access: Public
//  Description: Should be called by an object reading itself from the
//               Bam file to indicate that this particular object
//               would like to receive the finalize() callback when
//               all the objects and pointers in the Bam file are
//               completely read.
//
//               This provides a hook for objects (like Characters)
//               that need to do any additional finalization work
//               after all of their related pointers are guaranteed to
//               be filled in.
////////////////////////////////////////////////////////////////////
void BamReader::
register_finalize(TypedWritable *whom) {
  if (whom == TypedWritable::Null) {
    bam_cat.error() << "Can't register a null pointer to finalize!" << endl;
    return;
  }
  _finalize_list.insert(whom);
}

////////////////////////////////////////////////////////////////////
//     Function: BamReader::finalize_now
//       Access: Public
//  Description: Forces the finalization of a particular object.  This
//               may be called by any of the objects during
//               finalization, to guarantee finalization ordering
//               where it is important.
////////////////////////////////////////////////////////////////////
void BamReader::
finalize_now(TypedWritable *whom) {
  nassertv(whom != (TypedWritable *)NULL);
  
  Finalize::iterator fi = _finalize_list.find(whom);
  if (fi != _finalize_list.end()) {
    _finalize_list.erase(fi);
    whom->finalize();
  }
}

////////////////////////////////////////////////////////////////////
//     Function: BamReader::get_pta
//       Access: Public
//  Description: This function works in conjection with
//               register_pta(), below, to read a PointerToArray (PTA)
//               from the Bam file, and unify references to the same
//               PTA.
//
//               The first time get_pta() encounters a particular PTA,
//               it will return NULL.  This is the indication that the
//               caller should then read in the data associated with
//               the PTA, and subsequently call register_pta() with
//               the address of the filled-in array.
//
//               The next time (and all subsequent times) that
//               get_pta() encounters this same PTA, it will return
//               the pointer that was passed with register_pta().
//
//               Also see the READ_PTA() macro, which consolidates all
//               the work that must be done to read a PTA.
////////////////////////////////////////////////////////////////////
void *BamReader::
get_pta(DatagramIterator &scan) {
  nassertr(_pta_id == -1, (void *)NULL);
  int id = scan.get_uint16();

  if (id == 0) {
    // As always, a 0 ID indicates a NULL pointer.  The caller will
    // not be able to differentiate this case from that of a
    // previously-read pointer, but that's OK because the next data in
    // the Bam file is the length of the array, which will be
    // zero--indicating an empty or NULL array.
    return (void *)NULL;
  }

  PTAMap::iterator pi = _pta_map.find(id);
  if (pi == _pta_map.end()) {
    // This is the first time we've encountered this particular ID,
    // meaning we need to read the data now and register it.
    _pta_id = id;
    return (void *)NULL;
  }

  return (*pi).second;
}

////////////////////////////////////////////////////////////////////
//     Function: BamReader::register_pta
//       Access: Public
//  Description: The second part of read_pta(), this should be called
//               with the pointer to the array that was read in after
//               read_pta() returned NULL.  This associates the
//               pointer with the ID that was previously read, so that
//               future calls to read_pta() will return the same
//               pointer.
//
//               Also see the READ_PTA() macro, which consolidates all
//               the work that must be done to read a PTA.
////////////////////////////////////////////////////////////////////
void BamReader::
register_pta(void *ptr) {
  if (_pta_id != -1) {
    bool inserted = _pta_map.insert(PTAMap::value_type(_pta_id, ptr)).second;
    _pta_id = -1;
    nassertv(inserted);
  }
}



////////////////////////////////////////////////////////////////////
//     Function: BamReader::p_read_object
//       Access: Private
//  Description: The private implementation of read_object(), this
//               reads an object from the file and returns its object
//               ID.
////////////////////////////////////////////////////////////////////
int BamReader::
p_read_object() {
  Datagram packet;

  if (_source->is_error()) {
    return 0;
  }

  // First, read a datagram for the object.
  if (!_source->get_datagram(packet)) {
    // When we run out of datagrams, we're at the end of the file.

    if (bam_cat.is_debug()) {
      bam_cat.debug()
        << "Reached end of bam source.\n";
    }
    return 0;
  }

  // Now extract the object definition from the datagram.
  DatagramIterator scan(packet);

  // An object definition in a Bam file consists of a TypeHandle
  // definition, defining the object's type, followed by an object ID
  // index, defining the particular instance (e.g. pointer) of this
  // object.

  TypeHandle type = read_handle(scan);
  int object_id = scan.get_uint16();

  // There are two cases.  Either this is a new object definition, or
  // this is a reference to an object that was previously defined.

  // We use the TypeHandle to differentiate these two cases.  By
  // convention, we write a TypeHandle::none() to the Bam file when we
  // are writing a reference to a previously-defined object, but we
  // write the object's actual type when we are writing its definition
  // right now.

  // Therefore, if the type is TypeHandle::none(), then we must have
  // already read in and created the object (although its pointers may
  // not be fully instantiated yet).  On the other hand, if the type
  // is anything else, then we must read the definition to follow.

  if (type != TypeHandle::none()) {
    // Now we are going to read and create a new object.

    // Defined the parameters for passing to the object factory.
    FactoryParams fparams;
    fparams.add_param(new BamReaderParam(scan, this));

    // First, we must add an entry into the map for this object ID, so
    // that in case this function is called recursively during the
    // object's factory constructor, we will have some definition for
    // the object.  It doesn't matter yet what the pointer is.
    CreatedObjs::iterator oi = 
      _created_objs.insert(CreatedObjs::value_type(object_id, NULL)).first;

    // Now we can call the factory to create the object.
    TypedWritable *object = 
      _factory->make_instance_more_general(type, fparams);

    // And now we can store the new object pointer in the map.
    (*oi).second = object;

    //Just some sanity checks
    if (object == (TypedWritable *)NULL) {
      bam_cat.error() 
        << "Unable to create an object of type " << type << endl;

    } else if (object->get_type() != type) {
      bam_cat.warning()
        << "Attempted to create a " << type.get_name() \
        << " but a " << object->get_type() \
        << " was created instead." << endl;

    } else {
      if (bam_cat.is_spam()) {
        bam_cat.spam()
          << "Read a " << object->get_type() << "\n";
      }
    }
  }

  return object_id;
}

////////////////////////////////////////////////////////////////////
//     Function: BamReader::finalize
//       Access: Private
//  Description: Should be called after all objects have been read,
//               this will finalize all the objects that registered
//               themselves for the finalize callback.
////////////////////////////////////////////////////////////////////
void BamReader::
finalize() {
  if (bam_cat.is_debug()) {
    bam_cat.debug()
      << "Finalizing bam source\n";
  }

  Finalize::iterator fi = _finalize_list.begin();
  while (fi != _finalize_list.end()) {
    TypedWritable *object = (*fi);
    nassertv(object != (TypedWritable *)NULL);
    _finalize_list.erase(fi);
    object->finalize();

    fi = _finalize_list.begin();
  }
}
