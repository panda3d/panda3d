// Filename: ipc_file.I
// Created by:  jason (07Jun00)
//

#include <datagramIterator.h>
#include <config_express.h>
#include <nspr.h>

INLINE bool datagram_file::
open(file_mode mode, string header){
  //By definition a file should not be opened for both
  //reading and writing
  nassertr(mode != FILE_BOTH, false);
  _mode = mode; 

  if (!base_file::open(mode)) return false;

  if (!header.empty())
  {
    if (mode == FILE_READ)
    {
      string file_header;
      if (readin(file_header,header.size()) != (int)header.size()) return false;
      if (file_header != header)
      {
	//Note!  Using nout until a config_ipc file is made and therefore
	//ipc_cat is made
	nout << "File is either damaged or not valid" << endl;
	return false;
      }
    }
    else
    {
      if (writeout(header) != (int)header.size()) return false;
    }
  }

  _valid = true;
  return true;
}

INLINE bool datagram_file::
is_valid(void)
{
  return _valid;
}

INLINE bool datagram_file::
empty(void){
  return file::empty();
}

INLINE bool datagram_file::
get_datagram(Datagram& dataBlock) {
  //Only perform reads when mode is set to 
  //FILE_READ.   This is because datagram_file
  //can only read or write.
  nassertr(_mode == FILE_READ, false);  

  string header, packet;

  if (readin(header,sizeof(PN_uint32)) != sizeof(PN_uint32)) return false;
  
  Datagram size(header.data(), header.size());
  DatagramIterator sizescan(size);
  
  PN_uint32 packetsize = sizescan.get_uint32();

  if (readin(packet, packetsize) != (int)packetsize) return false;
  dataBlock.append_data(packet.data(), packetsize);

  return true;
}

INLINE bool datagram_file::
put_datagram(const Datagram& dataBlock) {
  //Only perform writes when mode is set to 
  //FILE_READ or FILE_APPEND.   This is because datagram_file
  //can only read or write.
  nassertr(_mode == FILE_APPEND || _mode == FILE_WRITE, false);
  DatagramIterator scanin(dataBlock);  
  Datagram out;
  int datalen = dataBlock.get_length();

  //Determine the size of the datagram and add it into the data to be
  //written to the file.  Then append the message.
  out.add_uint32(datalen);
  out.append_data(scanin.get_remaining_bytes().data(), datalen);

  DatagramIterator scanout(out);
  if (writeout(scanout.get_remaining_bytes()) != (int)out.get_length()) return false;

  return true;
}


