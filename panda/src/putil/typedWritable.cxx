// Filename: typedWritable.cxx
// Created by:  jason (08Jun00)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) Carnegie Mellon University.  All rights reserved.
//
// All use of this software is subject to the terms of the revised BSD
// license.  You should have received a copy of this license along
// with this source code in a file named "LICENSE."
//
////////////////////////////////////////////////////////////////////

#include "typedWritable.h"
#include "bamWriter.h"
#include "bamReader.h"
#include "datagramOutputFile.h"
#include "datagramInputFile.h"
#include "lightMutexHolder.h"
#include "bam.h"

LightMutex TypedWritable::_bam_writers_lock;

TypeHandle TypedWritable::_type_handle;
TypedWritable* const TypedWritable::Null = (TypedWritable*)0L;

#ifdef HAVE_PYTHON
#include "py_panda.h"  
#ifndef CPPPARSER
extern EXPCL_PANDA_PUTIL Dtool_PyTypedObject Dtool_BamWriter;
#endif  // CPPPARSER
#endif  // HAVE_PYTHON

////////////////////////////////////////////////////////////////////
//     Function: TypedWritable::Destructor
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
TypedWritable::
~TypedWritable() {
  // Remove the object pointer from the BamWriters that reference it.
  if (_bam_writers != (BamWriters *)NULL) {
    BamWriters temp;
    {
      LightMutexHolder holder(_bam_writers_lock);
      _bam_writers->swap(temp);
      delete _bam_writers;
      _bam_writers = NULL;
    }
    BamWriters::iterator wi;
    for (wi = temp.begin(); wi != temp.end(); ++wi) {
      BamWriter *writer = (*wi);
      writer->object_destructs(this);
    }
  }
}

////////////////////////////////////////////////////////////////////
//     Function: TypedWritable::write_datagram
//       Access: Public, Virtual
//  Description: Writes the contents of this object to the datagram
//               for shipping out to a Bam file.
////////////////////////////////////////////////////////////////////
void TypedWritable::
write_datagram(BamWriter *, Datagram &) {
}

////////////////////////////////////////////////////////////////////
//     Function: TypedWritable::update_bam_nested
//       Access: Public, Virtual
//  Description: Called by the BamWriter when this object has not
//               itself been modified recently, but it should check
//               its nested objects for updates.
////////////////////////////////////////////////////////////////////
void TypedWritable::
update_bam_nested(BamWriter *) {
}

////////////////////////////////////////////////////////////////////
//     Function: TypedWritable::complete_pointers
//       Access: Public, Virtual
//  Description: Receives an array of pointers, one for each time
//               manager->read_pointer() was called in fillin().
//               Returns the number of pointers processed.
//
//               This is the callback function that is made by the
//               BamReader at some later point, after all of the
//               required pointers have been filled in.  It is
//               necessary because there might be forward references
//               in a bam file; when we call read_pointer() in
//               fillin(), the object may not have been read from the
//               file yet, so we do not have a pointer available at
//               that time.  Thus, instead of returning a pointer,
//               read_pointer() simply reserves a later callback.
//               This function provides that callback.  The calling
//               object is responsible for keeping track of the number
//               of times it called read_pointer() and extracting the
//               same number of pointers out of the supplied vector,
//               and storing them appropriately within the object.
////////////////////////////////////////////////////////////////////
int TypedWritable::
complete_pointers(TypedWritable **, BamReader *) {
  return 0;
}

////////////////////////////////////////////////////////////////////
//     Function: TypedWritable::require_fully_complete
//       Access: Public, Virtual
//  Description: Some objects require all of their nested pointers to
//               have been completed before the objects themselves can
//               be completed.  If this is the case, override this
//               method to return true, and be careful with circular
//               references (which would make the object unreadable
//               from a bam file).
////////////////////////////////////////////////////////////////////
bool TypedWritable::
require_fully_complete() const {
  return false;
}

////////////////////////////////////////////////////////////////////
//     Function: TypedWritable::fillin
//       Access: Public, Virtual
//  Description: This internal function is intended to be called by
//               each class's make_from_bam() method to read in all of
//               the relevant data from the BamFile for the new
//               object.  It is also called directly by the BamReader
//               to re-read the data for an object that has been
//               placed on the stream for an update.
////////////////////////////////////////////////////////////////////
void TypedWritable::
fillin(DatagramIterator &, BamReader *) {
}


////////////////////////////////////////////////////////////////////
//     Function: TypedWritable::finalize
//       Access: Public, Virtual
//  Description: Called by the BamReader to perform any final actions
//               needed for setting up the object after all objects
//               have been read and all pointers have been completed.
////////////////////////////////////////////////////////////////////
void TypedWritable::
finalize(BamReader *) {
}

////////////////////////////////////////////////////////////////////
//     Function: TypedWritable::as_reference_count
//       Access: Public, Virtual
//  Description: Returns the pointer cast to a ReferenceCount pointer,
//               if it is in fact of that type.
////////////////////////////////////////////////////////////////////
ReferenceCount *TypedWritable::
as_reference_count() {
  return NULL;
}

#ifdef HAVE_PYTHON
////////////////////////////////////////////////////////////////////
//     Function: TypedWritable::__reduce__
//       Access: Published
//  Description: This special Python method is implement to provide
//               support for the pickle module.
//
//               This hooks into the native pickle and cPickle
//               modules, but it cannot properly handle
//               self-referential BAM objects.
////////////////////////////////////////////////////////////////////
PyObject *TypedWritable::
__reduce__(PyObject *self) const {
  return __reduce_persist__(self, NULL);
}
#endif  // HAVE_PYTHON

#ifdef HAVE_PYTHON
////////////////////////////////////////////////////////////////////
//     Function: TypedWritable::__reduce_persist__
//       Access: Published
//  Description: This special Python method is implement to provide
//               support for the pickle module.
//
//               This is similar to __reduce__, but it provides
//               additional support for the missing persistent-state
//               object needed to properly support self-referential
//               BAM objects written to the pickle stream.  This hooks
//               into the pickle and cPickle modules implemented in
//               direct/src/stdpy.
////////////////////////////////////////////////////////////////////
PyObject *TypedWritable::
__reduce_persist__(PyObject *self, PyObject *pickler) const {
  // We should return at least a 2-tuple, (Class, (args)): the
  // necessary class object whose constructor we should call
  // (e.g. this), and the arguments necessary to reconstruct this
  // object.

  // Check that we have a decode_from_bam_stream python method.  If not,
  // we can't use this interface.
  PyObject *method = PyObject_GetAttrString(self, "decode_from_bam_stream");
  if (method == NULL) {
    ostringstream stream;
    stream << "Cannot pickle objects of type " << get_type() << "\n";
    string message = stream.str();
    PyErr_SetString(PyExc_TypeError, message.c_str());
    return NULL;
  }
  Py_DECREF(method);

  BamWriter *writer = NULL;
  if (pickler != NULL) {
    PyObject *py_writer = PyObject_GetAttrString(pickler, "bamWriter");
    if (py_writer == NULL) {
      // It's OK if there's no bamWriter.
      PyErr_Clear();
    } else {
      DTOOL_Call_ExtractThisPointerForType(py_writer, &Dtool_BamWriter, (void **)&writer);
      Py_DECREF(py_writer);
    }
  }

  // First, streamify the object, if possible.
  string bam_stream;
  if (!encode_to_bam_stream(bam_stream, writer)) {
    ostringstream stream;
    stream << "Could not bamify object of type " << get_type() << "\n";
    string message = stream.str();
    PyErr_SetString(PyExc_TypeError, message.c_str());
    return NULL;
  }

  // Start by getting this class object.
  PyObject *this_class = PyObject_Type(self);
  if (this_class == NULL) {
    return NULL;
  }

  PyObject *func;
  if (writer != NULL) {
    // The modified pickle support: call the "persistent" version of
    // this function, which receives the unpickler itself as an
    // additional parameter.
    func = find_global_decode(this_class, "py_decode_TypedWritable_from_bam_stream_persist");
    if (func == NULL) {
      PyErr_SetString(PyExc_TypeError, "Couldn't find py_decode_TypedWritable_from_bam_stream_persist()");
      Py_DECREF(this_class);
      return NULL;
    }

  } else {
    // The traditional pickle support: call the non-persistent version
    // of this function.

    func = find_global_decode(this_class, "py_decode_TypedWritable_from_bam_stream");
    if (func == NULL) {
      PyErr_SetString(PyExc_TypeError, "Couldn't find py_decode_TypedWritable_from_bam_stream()");
      Py_DECREF(this_class);
      return NULL;
    }
  }

  PyObject *result = Py_BuildValue("(O(Os#))", func, this_class, bam_stream.data(), bam_stream.size());
  Py_DECREF(func);
  Py_DECREF(this_class);
  return result;
}
#endif  // HAVE_PYTHON

////////////////////////////////////////////////////////////////////
//     Function: TypedWritable::encode_to_bam_stream
//       Access: Published
//  Description: Converts the TypedWritable object into a single
//               stream of data using a BamWriter, and stores that
//               data in the indicated string.  Returns true on
//               success, false on failure.
//
//               This is a convenience method particularly useful for
//               cases when you are only serializing a single object.
//               If you have many objects to process, it is more
//               efficient to use the same BamWriter to serialize all
//               of them together.
////////////////////////////////////////////////////////////////////
bool TypedWritable::
encode_to_bam_stream(string &data, BamWriter *writer) const {
  data.clear();
  ostringstream stream;

  // We use nested scoping to ensure the destructors get called in the
  // right order.
  {
    DatagramOutputFile dout;
    if (!dout.open(stream)) {
      return false;
    }
    
    if (writer == NULL) {
      // Create our own writer.
    
      if (!dout.write_header(_bam_header)) {
        return false;
      }

      BamWriter writer(&dout);
      if (!writer.init()) {
        return false;
      }
      
      if (!writer.write_object(this)) {
        return false;
      }
    } else {
      // Use the existing writer.
      writer->set_target(&dout);
      bool result = writer->write_object(this);
      writer->set_target(NULL);
      if (!result) {
        return false;
      }
    }
  }

  data = stream.str();
  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: TypedWritable::decode_raw_from_bam_stream
//       Access: Published, Static
//  Description: Reads the string created by a previous call to
//               encode_to_bam_stream(), and extracts the single
//               object on that string.  Returns true on success,
//               false on on error.
//
//               This variant sets the TypedWritable and
//               ReferenceCount pointers separately; both are pointers
//               to the same object.  The reference count is not
//               incremented; it is the caller's responsibility to
//               manage the reference count.
//
//               Note that this method cannot be used to retrieve
//               objects that do not inherit from ReferenceCount,
//               because these objects cannot persist beyond the
//               lifetime of the BamReader that reads them.  To
//               retrieve these objects from a bam stream, you must
//               construct a BamReader directly.
//
//               If you happen to know that the particular object in
//               question inherits from TypedWritableReferenceCount or
//               PandaNode, consider calling the variant of
//               decode_from_bam_stream() defined for those methods,
//               which presents a simpler interface.
////////////////////////////////////////////////////////////////////
bool TypedWritable::
decode_raw_from_bam_stream(TypedWritable *&ptr, ReferenceCount *&ref_ptr,
                           const string &data, BamReader *reader) {
  istringstream stream(data);

  DatagramInputFile din;
  if (!din.open(stream)) {
    return false;
  }

  if (reader == NULL) {
    // Create a local reader.
  
    string head;
    if (!din.read_header(head, _bam_header.size())) {
      return false;
    }
    
    if (head != _bam_header) {
      return false;
    }

    BamReader reader(&din);
    if (!reader.init()) {
      return false;
    }
    
    if (!reader.read_object(ptr, ref_ptr)) {
      return false;
    }
    
    if (!reader.resolve()) {
      return false;
    }
    
    if (ref_ptr == NULL) {
      // Can't support non-reference-counted objects.
      return false;
    }

    // Protect the pointer from accidental deletion when the BamReader
    // goes away.
    ref_ptr->ref();

  } else {
    // Use the existing reader.
    reader->set_source(&din);
    if (!reader->read_object(ptr, ref_ptr)) {
      reader->set_source(NULL);
      return false;
    }
    
    if (!reader->resolve()) {
      reader->set_source(NULL);
      return false;
    }
    
    if (ref_ptr == NULL) {
      // Can't support non-reference-counted objects.
      reader->set_source(NULL);
      return false;
    }

    // This BamReader isn't going away, but we have to balance the
    // unref() below.
    ref_ptr->ref();
    reader->set_source(NULL);
  }


  // Now decrement the ref count, without deleting the object.  This
  // may reduce the reference count to zero, but that's OK--we trust
  // the caller to manage the reference count from this point on.
  ref_ptr->unref();
  return true;
}

#ifdef HAVE_PYTHON
////////////////////////////////////////////////////////////////////
//     Function: TypedWritable::find_global_decode
//       Access: Public, Static
//  Description: This is a support function for __reduce__().  It
//               searches for the global function
//               py_decode_TypedWritable_from_bam_stream() in this
//               class's module, or in the module for any base class.
//               (It's really looking for the libpanda module, but we
//               can't be sure what name that module was loaded under,
//               so we search upwards this way.)
//
//               Returns: new reference on success, or NULL on failure.
////////////////////////////////////////////////////////////////////
PyObject *TypedWritable::
find_global_decode(PyObject *this_class, const char *func_name) {
  PyObject *module_name = PyObject_GetAttrString(this_class, "__module__");
  if (module_name != NULL) {
    // borrowed reference
    PyObject *sys_modules = PyImport_GetModuleDict();
    if (sys_modules != NULL) {
      // borrowed reference
      PyObject *module = PyDict_GetItem(sys_modules, module_name);
      if (module != NULL){ 
        PyObject *func = PyObject_GetAttrString(module, (char *)func_name);
        if (func != NULL) {
          Py_DECREF(module_name);
          return func;
        }
      }
    }
  }
  Py_DECREF(module_name);

  PyObject *bases = PyObject_GetAttrString(this_class, "__bases__");
  if (bases != NULL) {
    if (PySequence_Check(bases)) {
      Py_ssize_t size = PySequence_Size(bases);
      for (Py_ssize_t i = 0; i < size; ++i) {
        PyObject *base = PySequence_GetItem(bases, i);
        if (base != NULL) {
          PyObject *func = find_global_decode(base, func_name);
          Py_DECREF(base);
          if (func != NULL) {
            Py_DECREF(bases);
            return func;
          }
        }
      }
    }
    Py_DECREF(bases);
  }

  return NULL;
}
#endif  // HAVE_PYTHON

#ifdef HAVE_PYTHON
////////////////////////////////////////////////////////////////////
//     Function: py_decode_TypedWritable_from_bam_stream
//       Access: Published
//  Description: This wrapper is defined as a global function to suit
//               pickle's needs.
//
//               This hooks into the native pickle and cPickle
//               modules, but it cannot properly handle
//               self-referential BAM objects.
////////////////////////////////////////////////////////////////////
PyObject *
py_decode_TypedWritable_from_bam_stream(PyObject *this_class, const string &data) {
  return py_decode_TypedWritable_from_bam_stream_persist(NULL, this_class, data);
}
#endif  // HAVE_PYTHON


#ifdef HAVE_PYTHON
////////////////////////////////////////////////////////////////////
//     Function: py_decode_TypedWritable_from_bam_stream_persist
//       Access: Published
//  Description: This wrapper is defined as a global function to suit
//               pickle's needs.
//
//               This is similar to
//               py_decode_TypedWritable_from_bam_stream, but it
//               provides additional support for the missing
//               persistent-state object needed to properly support
//               self-referential BAM objects written to the pickle
//               stream.  This hooks into the pickle and cPickle
//               modules implemented in direct/src/stdpy.
////////////////////////////////////////////////////////////////////
PyObject *
py_decode_TypedWritable_from_bam_stream_persist(PyObject *pickler, PyObject *this_class, const string &data) {

  PyObject *py_reader = NULL;
  if (pickler != NULL) {
    py_reader = PyObject_GetAttrString(pickler, "bamReader");
    if (py_reader == NULL) {
      // It's OK if there's no bamReader.
      PyErr_Clear();
    }
  }

  // We need the function PandaNode::decode_from_bam_stream or
  // TypedWritableReferenceCount::decode_from_bam_stream, which
  // invokes the BamReader to reconstruct this object.  Since we use
  // the specific object's class as the pointer, we get the particular
  // instance of decode_from_bam_stream appropriate to this class.

  PyObject *func = PyObject_GetAttrString(this_class, "decode_from_bam_stream");
  if (func == NULL) {
    return NULL;
  }

  PyObject *result;
  if (py_reader != NULL){
    result = PyObject_CallFunction(func, (char *)"(s#O)", data.data(), data.size(), py_reader);
    Py_DECREF(py_reader);
  } else {
    result = PyObject_CallFunction(func, (char *)"(s#)", data.data(), data.size());
  }

  if (result == NULL) {
    return NULL;
  }

  if (result == Py_None) {
    Py_DECREF(result);
    PyErr_SetString(PyExc_ValueError, "Could not unpack bam stream");
    return NULL;
  }    

  return result;
}
#endif  // HAVE_PYTHON

