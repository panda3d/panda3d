// Filename: bamReader.cxx
// Created by:  jason (12Jun00)
//

#include <pandabase.h>
#include <notify.h>
#include <nspr.h>

#include "bam.h"
#include "bamReader.h"
#include <datagramIterator.h>
#include "config_util.h"

WriteableFactory *BamReader::_factory = (WriteableFactory*)0L;
BamReader* const BamReader::Null = (BamReader*)0L;
WriteableFactory* const BamReader::NullFactory = (WriteableFactory*)0L;

const int BamReader::_cur_major = _bam_major_ver;
const int BamReader::_cur_minor = _bam_minor_ver;


////////////////////////////////////////////////////////////////////
//     Function: BamReader::Destructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
BamReader::
~BamReader(void)
{
}

////////////////////////////////////////////////////////////////////
//     Function: BamReader::init
//       Access: Public
//  Description: Does all initialization necessary for BamReader to work
//               Current this means reading in the Type Map stored as
//               the header of the file.  This TypeMap contains the 
//               name of all types written out and their type indeces
//               as set when the file was created.  This map is used
//               for later determing the current TypeHandle for each
//               object written.  Each Object is identified in the file
//               with the old index for space reason.
//
//               This returns true if the BamReader successfully
//               initialized, false otherwise.
////////////////////////////////////////////////////////////////////
bool BamReader::
init(void)
{
  Datagram header;

  if (!_source->is_valid())
  {
    return false;
  }

  if (!_source->get_datagram(header)) {
    util_cat->error()
      << "Unable to read Bam header.\n";
    return false;
  }

  DatagramIterator scan(header);

  _file_major = scan.get_uint16();
  _file_minor = scan.get_uint16();

  // If the major version is different, or the minor version is
  // *newer*, we can't safely load the file.
  if (_file_major != _bam_major_ver || _file_minor > _bam_minor_ver)
  {
    util_cat->error()
      << "Bam file is version " << _file_major << "." << _file_minor
      << ".\n";

    if (_bam_minor_ver == 0) {
      util_cat.error()
	<< "This program can only load version " 
	<< _bam_major_ver << ".0 bams.\n";
    } else {
      util_cat.error()
	<< "This program can only load version " 
	<< _bam_major_ver << ".0 through " 
	<< _bam_major_ver << "." << _bam_minor_ver << " bams.\n";
    }
      
    return false;
  }

  if (util_cat->is_debug()) {
    util_cat->debug() 
      << "Bam file is version " << _file_major << "." << _file_minor
      << ".\n";
    if (_file_minor != _bam_minor_ver) {
      util_cat.debug()
	<< "(Current version is " << _bam_major_ver << "." << _bam_minor_ver
	<< ".)\n";
    }
  }

  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: BamReader::read_handle
//       Access: Public
//  Description: Reads a TypeHandle out of the Datagram.  If the Type
//               has not been registered yet, BamReader will register
//               the Type with TypeRegistry
////////////////////////////////////////////////////////////////////
TypeHandle BamReader::
read_handle(DatagramIterator& scan)
{
  int id = scan.get_uint16();
  
  if (id == 0)
  {
    //This indicates an object that should have already been read in,
    //so return TypeHandle::none() to indicate this.
    return TypeHandle::none();
  }

  if (_index_map.find(id) == _index_map.end())
  {
    //Have not loaded this type before
    string name = scan.get_string();
    TypeHandle type = TypeRegistry::ptr()->find_type(name);
    bool new_type = false;
   
    if (type == TypeHandle::none())
    {
      //This is a new type we've never heard of before so register it
      //with the type registry
      type = TypeRegistry::ptr()->register_dynamic_type(name);
      util_cat.warning()
	<< "Bam file contains objects of unknown type: " << type << "\n";
      new_type = true;
    }
    _index_map[id] = type;

    // Now pick up the derivation information.
    int num_parent_classes = scan.get_uint8();
    for (int i = 0; i < num_parent_classes; i++) {
      TypeHandle parent_type = read_handle(scan);
      if (new_type) {
	TypeRegistry::ptr()->record_derivation(type, parent_type);
      } else {
	if (type.get_parent_towards(parent_type) != parent_type) {
	  util_cat.warning()
	    << "Bam file indicates a derivation of " << type 
	    << " from " << parent_type << " which is no longer true.\n";
	}
      }
    }
    
  }

  return _index_map[id];
}

////////////////////////////////////////////////////////////////////
//     Function: BamReader::read_object
//       Access: Public
//  Description: Reads an object definition from the DatagramGenerator
//               and generates an object of the correct type, and returns
//               a pointer to that object.
////////////////////////////////////////////////////////////////////
TypedWriteable* BamReader::
read_object(void)
{
  Datagram packet, body;

  //if (_source->empty())  //If the source is empty, then we merely have
  //{                      //back reference read's queue, so clear the queue
  //  clear_queue();
  //  return TypedWriteable::Null;
  //}

  if (!_source->is_valid())
  {
    return TypedWriteable::Null;
  }

  if (!_source->get_datagram(packet)) {
    // The datagram source is empty.
    return TypedWriteable::Null;
  }

  //Now pull out the type ID and Object ID from the
  //datagram and pass the remaining data in that 
  //datagram, the body to the WriteableParam for 
  //creation of the object
  DatagramIterator scan(packet);
  TypeHandle type = read_handle(scan);
  PN_uint16 objId = scan.get_uint16();

  //Before anything else, check to see if type is none
  //If that is the case, then we SHOULD have already read
  //in and created the object (although pointers in that object
  //may not be fully instanciated yet), so just return a pointer
  //to that already created obj and don't try to re-create it
  if (type != TypeHandle::none())
  {
    string message = scan.get_remaining_bytes();
    body.append_data(message.data(), message.size());
    
    //Generate FactoryParams object to pass to the factory for
    //creating the object
    FactoryParams list;
    list.add_param(new WriteableParam(body));
    list.add_param(new BamReaderParam(this));

    //Due to the recursive nature of reading in objects (and their pointers)
    //this line is to ensure that objId gets into the map _created_objs
    //before we call make_instance.   It works due to the nature of map.
    //What happens is that a pair consistenting of objId and some random
    //pointer is put into the map.  But this is okay, since in the 
    //recursion we only refer to this objId for purposes of whether
    //it is there or not and we immediately override it with a good
    //value otherwise
    _created_objs[objId];
    _created_objs[objId] = _factory->make_instance_more_general(type, list);
    //Just some sanity checks
    if (_created_objs[objId] == (TypedWriteable *)NULL)
    {
      util_cat->error() << "Failed to create a " << type.get_name() << endl;
    }
    else if (_created_objs[objId]->get_type() != type)
    {
      util_cat->warning() << "Attempted to create a " << type.get_name() \
			  << " but a " << _created_objs[objId]->get_type().get_name() \
			  << " was created instead." << endl;
    }
  }

  empty_queue();
  return _created_objs[objId];
}

////////////////////////////////////////////////////////////////////
//     Function: BamReader::read_pointer
//       Access: Public
//  Description: Utility function provided to correctly read in objects
//               from the Datagram source, that any other object contains
//               pointers to.  Will correctly handle all circular references
//               Any caller of this function should pass a pointer to itself
//               because the pointer request will be stored and completed
//               at a later pointer when the object is actually generated
////////////////////////////////////////////////////////////////////
void BamReader::
read_pointer(DatagramIterator& scan, TypedWriteable* forWhom)
{
  PN_uint16 objId = scan.get_uint16();
  _deferred_pointers[forWhom].push_back(objId);
  //This is safe since we have already read the full datagram
  //for the object requesting this object.  So there can be
  //no collision on that front. 
  //IMPORTANT NOTE:  This does make the assumption that
  //that objects are requested by other objects in the same 
  //order that they wrote them originally.

  //Don't queue a read of a null pointer 
  //or if the object has already been read
  if ((objId != 0) && (_created_objs.find(objId) == _created_objs.end()))
    queue(objId);
}

////////////////////////////////////////////////////////////////////
//     Function: BamReader::register_finalize
//       Access: Public
//  Description: Register for later finalization
////////////////////////////////////////////////////////////////////
void BamReader::
register_finalize(TypedWriteable *whom)
{
  if (whom == TypedWriteable::Null)
  {
    util_cat->error() << "Can't register a null pointer to finalize!" << endl;
    return;
  }
  _finalize_list.insert(whom);
}

////////////////////////////////////////////////////////////////////
//     Function: BamReader::finalize_now
//       Access: Public
//  Description: Force finalization of a particular object
////////////////////////////////////////////////////////////////////
void BamReader::
finalize_now(TypedWriteable *whom)
{
  if (whom == TypedWriteable::Null)
  {
    util_cat->error() << "Can't finalize null pointer!" << endl;
    return;
  }
  if (_finalize_list.find(whom) != _finalize_list.end())
  {
    whom->finalize();
    _finalize_list.erase(whom);
  }
  else
  {
    util_cat->warning() << "Request to finalize object of type " 
			<< whom->get_type().get_name() << " failed" 
			<< endl;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: BamReader::finalize_this
//       Access: Private
//  Description: 
////////////////////////////////////////////////////////////////////
void BamReader::
finalize_this(TypedWriteable *whom)
{
  whom->finalize();
  _finalize_list.erase(whom);
}

////////////////////////////////////////////////////////////////////
//     Function: BamReader::finalize
//       Access: Private
//  Description: Call finalize_this on all objects stored
////////////////////////////////////////////////////////////////////
void BamReader::
finalize(void)
{
  Finalize::iterator fi = _finalize_list.begin();
  while(fi != _finalize_list.end())
  {
    if (*fi == TypedWriteable::Null)
    {
      util_cat->error() << "Spun off into the weeds. "
			<< "somehow a null was registered to be "
			<< "finalized." << endl;
      _finalize_list.erase(fi);
    }
    else
    {
      finalize_this(*fi);
    }
    fi = _finalize_list.begin();
  }
}

////////////////////////////////////////////////////////////////////
//     Function: BamReader::get_pta
//       Access: Public
//  Description: Requests an already read PTA from BamReader.  If
//               the PTA has not already been read and registered, 
//               returns a NULL
////////////////////////////////////////////////////////////////////
void* BamReader::
get_pta(DatagramIterator &scan)
{
  int _id = scan.get_uint16();

  if (_id == 0)
  {
    _pta_id = -1;
    return (void*)NULL;
  }

  if (_ptamap.find(_id) == _ptamap.end())
  {
    _pta_id = _id;
    return (void*)NULL;
  }
  else
  {
    //Just to enforce that register_pta should do nothing if
    //get_pta returns a good value or a NULL was what was
    //actually written
    _pta_id = -1;
  }
  
  return _ptamap[_id];
}

////////////////////////////////////////////////////////////////////
//     Function: BamReader::register_pta
//       Access: Public
//  Description: Utility function to be called by the objects reading
//               themselves from a Datagram. Registers a PTA with 
//               BamReader, so that if there are any shared ptr references,
//               BamReader can detect and resolve those.  To be used
//               in conjunction with get_pta
////////////////////////////////////////////////////////////////////
void BamReader::
register_pta(void *ptr)
{
  //Just a sanity check to make sure that register_pta
  //does nothing if get_pta has not been called first
  if (_pta_id != -1)
  {
    _ptamap[_pta_id] = ptr;
  }
  _pta_id = -1;
}

////////////////////////////////////////////////////////////////////
//     Function: BamReader::resolve
//       Access: Public
//  Description: Calling this function, asks BamReader to go ahead
//               and resolve all pointer requests that can be resolved
////////////////////////////////////////////////////////////////////
bool BamReader::
resolve(void)
{
  Requests::iterator whom, old;
  vector_ushort::iterator request;
  bool objReferencesComplete;
  vector_typedWriteable references;

  //Loop through the list of all objects who registered themselves as wanting
  //a pointer to another object to be completed
  whom = _deferred_pointers.begin(); 
  while(whom != _deferred_pointers.end())
  {
    //For each of those objects, loop through its list of objects that it is
    //interested in.  If any one of those is not complete, then do nothing
    //for that object, as when it is passed the list of pointers, it will
    //expect them in a certain order and in totality.
    objReferencesComplete = true;
    references.clear();
    for(request = (*whom).second.begin(); request != (*whom).second.end(); request++)
    {
      //Check and see if the request was for a null pointer
      if (*request == 0)
      {
	references.push_back(TypedWriteable::Null);      
      }
      else
      {
	//Test to see if the object requested has been created or not yet
	if (_created_objs.find((*request)) == _created_objs.end())
	{
	  objReferencesComplete = false;
	  break;
	}
       	references.push_back(_created_objs[(*request)]);
      }
    }
    if (objReferencesComplete)
    {
      (*whom).first->complete_pointers(references, this);
      //The request list has been completed, so delete the 
      //list froms storage
      //Annoying STL design flaw forces this delete trick
      old = whom;
      whom++;
      _deferred_pointers.erase(old);
    }
    else
    {
      // nassertv(objReferencesComplete);
      util_cat.warning()
	<< "Unable to complete " << (*whom).first->get_type() << "\n";
      whom++;
    }
  }

  finalize();

  return true;
}


