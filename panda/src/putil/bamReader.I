// Filename: bamReader.I
// Created by:  jason (12Jun00)
//

#include <algorithm>

////////////////////////////////////////////////////////////////////
//     Function: BamReader::Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE BamReader::
BamReader(DatagramGenerator *generator) 
  : _source(generator)
{
}


////////////////////////////////////////////////////////////////////
//     Function: BamReader::create_factory
//       Access: Private, Static
//  Description: Creates a new WriteableFactory for generating
//               TypedWriteable objects
////////////////////////////////////////////////////////////////////
INLINE void BamReader::
create_factory(void)
{
  _factory = new WriteableFactory;
}

////////////////////////////////////////////////////////////////////
//     Function: BamReader::get_factory
//       Access: Public, Static
//  Description: Returns the global WriteableFactory for generating
//               TypedWriteable objects
////////////////////////////////////////////////////////////////////
INLINE WriteableFactory* BamReader::
get_factory(void)
{
  if (_factory == NullFactory)
    create_factory();
  return _factory;
}

////////////////////////////////////////////////////////////////////
//     Function: BamReader::get_file_major_ver
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE int BamReader::
get_file_major_ver(void)
{
  return _file_major;
}

////////////////////////////////////////////////////////////////////
//     Function: BamReader::get_file_minor_ver
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE int BamReader::
get_file_minor_ver(void)
{
  return _file_minor;
}

////////////////////////////////////////////////////////////////////
//     Function: BamReader::get_current_major_ver
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE int BamReader::
get_current_major_ver(void)
{
  return _cur_major;
}

////////////////////////////////////////////////////////////////////
//     Function: BamReader::get_current_minor_ver
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE int BamReader::
get_current_minor_ver(void)
{
  return _cur_minor;
}

////////////////////////////////////////////////////////////////////
//     Function: BamReader::queue
//       Access: Private
//  Description: queues the objId of the object needing to be created
////////////////////////////////////////////////////////////////////
INLINE void BamReader::
queue(PN_uint16 objId)
{
  if (!binary_search(_deferred_reads.begin(), _deferred_reads.end(), objId))
      _deferred_reads.push_back(objId);
}

////////////////////////////////////////////////////////////////////
//     Function: BamReader::clear_queue
//       Access: Private
//  Description: Clears all pending object reads
////////////////////////////////////////////////////////////////////
INLINE void BamReader::
clear_queue(void)
{ 
  _deferred_reads.clear();
}

////////////////////////////////////////////////////////////////////
//     Function: parse_params
//       Access: Private, Static
//  Description: Takes in a FactoryParams, passed from a WriteableFactory
//               into any TypedWriteable's make function, and parses
//               out the datagram that contatins the data it needs to
//               construct itself, and parses out the pointer to the
//               managing BamReader object 
////////////////////////////////////////////////////////////////////
INLINE void
parse_params(const FactoryParams &params, BamReader* &manager, Datagram &packet)
{
  WriteableParam *datagram_param = DCAST(WriteableParam, params.get_param(0));
  BamReaderParam *manager_param = DCAST(BamReaderParam, params.get_param(1));

  Datagram temp_packet = datagram_param->get_datagram();
  DatagramIterator scan(temp_packet);
  string mesg = scan.get_remaining_bytes();
  packet.append_data(mesg.data(), mesg.size());

  manager = manager_param->get_manager();
}
