// Filename: bamWriter.cxx
// Created by:  jason (08Jun00)
//

#include <pandabase.h>
#include <notify.h>

#include "config_util.h"
#include "bam.h"
#include "bamWriter.h"

////////////////////////////////////////////////////////////////////
//     Function: BamWriter::Destructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
BamWriter::~BamWriter(void)
{
}

////////////////////////////////////////////////////////////////////
//     Function: BamWriter::write_handle
//       Access: Public
//  Description: Writes a type handle into the file.  If the handle 
//               has already been encountered then a number 
//               identifying that handle is written instead.  
////////////////////////////////////////////////////////////////////
void BamWriter::
write_handle(Datagram &packet, TypeHandle type)
{
  //No class should have a type handle index of 0
  nassertv(type.get_index() != 0);

  packet.add_uint16(type.get_index());
  if (_type_map.find(type.get_index()) == _type_map.end())
  {
    // This is the first time this TypeHandle has been written, so
    // also write out its name.
    _type_map.insert(type.get_index()); 
    packet.add_string(type.get_name());

    // We also need to write the derivation of the TypeHandle, in case
    // the program reading this file later has never heard of this
    // type before.
    int num_parent_classes = type.get_num_parent_classes();
    nassertv(num_parent_classes <= 255);  // Good grief!
    packet.add_uint8(num_parent_classes);
    for (int i = 0; i < num_parent_classes; i++) {
      write_handle(packet, type.get_parent_class(i));
    }
  }
}

////////////////////////////////////////////////////////////////////
//     Function: BamWriter::init
//       Access: Public
//  Description: This function initializes the BamWriter, setting 
//               up whatever needs to be set up before the BamWriter
//               is ready to write out objects.  It returns true if
//               successful, false on failure.
////////////////////////////////////////////////////////////////////
bool BamWriter::
init(void)
{
  //Write out the current major and minor BAM file version numbers
  Datagram header;

  header.add_uint16(_bam_major_ver);
  header.add_uint16(_bam_minor_ver);

  if (!_target->put_datagram(header)) {
    util_cat.error()
      << "Unable to write Bam header.\n";
    return false;
  }
  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: BamWriter::write_object
//       Access: Public
//  Description: Main function for performing the processing of 
//               writing an object to some Binary target.  Combined
//               with write_pointer this function can correctly handle
//               and break circular references, while still writing out
//               the necessary information for re-constructing those
//               references when the objects are read back in.
//
//               Returns true if the object is successfully written,
//               false otherwise.
////////////////////////////////////////////////////////////////////
bool BamWriter::
write_object(TypedWriteable* obj) 
{
  Datagram objData;

  nassertr(obj != TypedWriteable::Null, false);

  //No object should ever be written out that is not
  //registered as a child of TypedWriteable.  The 
  //only way this can get in here and have that true is
  //if someone forgot to set the classes init_type correctly.
  //So nassert on that to make it easy for them to track
  //down the error, because that will be an error, but one
  //that won't show up on the writing, it will show up in
  //reading, so could be potentially difficult to track down
  nassertr(obj->is_of_type(TypedWriteable::get_class_type()), false);

  //Need to keep track of the state of any objects written
  //or queued.  So every time write_object is called,
  //check to see if has been encountered before, if not
  //insert it into a map with a Key of the pointer to 
  //the object and the value being a class that contains
  //the unigue object ID for that object and a flag of
  //whether it has been written yet or no   
  if (_statemap.find(obj) == _statemap.end())
  {
   //If the object is not already in the map, then
    //add it and assign it and unique object number
    _statemap[obj].objId = _current;
    _current++;
    //We are making the assumption that there will never
    //be more than the max of an unsigned short in objects
    //in one file, but just in case, this nassert will make
    //it easy to find that possible error
    nassertr(_current != 0, false);
  }

  bool okflag = true;

  if (_writing) 
  {
    enqueue(obj);
  }
  else 
  {
    _writing = true;

    if (!_statemap[obj].written)
    {
      //Write the type handle of the object
      write_handle(objData, obj->get_type());
      //Write the unique ID of the object into the datagram
      //so that when it is read back, and there is a back
      //reference to this object, we know what indices correspond
      //to what objects
      objData.add_uint16(_statemap[obj].objId);
      obj->write_datagram(this, objData);
      _statemap[obj].written = true;
    }
    else
    {
      //If it is in the map, then we don't want to try
      //and write it again, so write the unique ID of the
      //object into the datagram so that when we read in 
      //later, we can resolve back references.  Write
      //0 for the type handle to identify back references
      objData.add_uint16(0);
      objData.add_uint16(_statemap[obj].objId);
    }

    if (!_target->put_datagram(objData)) {
      util_cat.error() 
	<< "Unable to write datagram to file.\n";
      okflag = false;
    }
    _writing = false;
    if (!_emptying)
    {
      if (!empty_queue()) {
	okflag = false;
      }
    }
  }

  return okflag;
}

////////////////////////////////////////////////////////////////////
//     Function: BamWriter::write_pointer
//       Access: Public
//  Description: Utility function to be called by the objects writing
//               themselves to a Datagram.  Basically amounts to a
//               request to BamWriter to queue an object to be written
//               and to write into the Datagram the necessary information
//               to reference that object
////////////////////////////////////////////////////////////////////
void BamWriter::
write_pointer(Datagram &packet , TypedWriteable *dest)
{
  //Write a zero for the object ID if the pointer is null
  if (dest == TypedWriteable::Null)
  {
    packet.add_uint16(0);
  }
  else
  {
    if (_statemap.find(dest) == _statemap.end())
    {
      write_object(dest);
    }
    
    packet.add_uint16(_statemap[dest].objId);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: BamWriter::register_pta
//       Access: Public
//  Description: Utility function to be called by the objects writing
//               themselves to a Datagram. Registers a PTA to be written
//               into the Datagram, and allows for shared references
//               to be captured in the datagram
////////////////////////////////////////////////////////////////////
bool BamWriter::
register_pta(Datagram &packet, void *ptr)
{
  if (ptr != (void*)NULL)
  {
    if (_ptamap.find(ptr) == _ptamap.end())
    {
      _ptamap[ptr] = _current_pta;
      packet.add_uint16(_current_pta);
      _current_pta++;
      return false;
    }

    packet.add_uint16(_ptamap[ptr]);
  }
  else
  {
    //A zero for the PTA ID indicates a NULL ptr
    packet.add_uint16(0);
    return false;
  }  

  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: BamWriter::enqueue
//       Access: Private
//  Description: Queue an object to be written
////////////////////////////////////////////////////////////////////
void BamWriter::
enqueue(TypedWriteable *obj)
{
  _deferred.push_back(obj);
}

////////////////////////////////////////////////////////////////////
//     Function: BamWriter::empty_queue
//       Access: Private
//  Description: For each object in the queue call write_object on it.
//               Returns true of all write_object calls returned true,
//               false otherwise.
////////////////////////////////////////////////////////////////////
bool BamWriter::
empty_queue(void)
{
  _emptying = true;
  bool okflag = true;
  while(!_deferred.empty())
  {
    if (!write_object(_deferred.front())) {
      okflag = false;
    }
    _deferred.pop_front();
  }
  _emptying = false;
  return okflag;
}











