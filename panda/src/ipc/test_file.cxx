// Filename: test_file.cxx
// Created by:  jason (05Sep00)
//

#include <pandabase.h>
#include "ipc_file.h"
#include <datagram.h>
#include <datagramIterator.h>
#include <notify.h>

#define _ERROR_TEST

int main(int argc, char* argv[])
{
  //Test the writing capabilities
  string test_file("test.out");
  string test_data("Jason Michael Clark");

  Datagram test_out;  test_out.add_string(test_data);
  datagram_file stream(test_file);
  
  nout << "WRITE TEST" << endl;
  nout << "Opening file for writing: " << test_file  << endl;
  if (!stream.open(file::FILE_WRITE)) nout << "Didn't open!" << endl;
  nout << "Writing datagram with data: " << test_data << endl;
  if (!stream.put_datagram(test_out)) nout << "Error in writing!" << endl;
  nout << "Closing file: " << test_file << endl;
  stream.close();

  //Test the reading capabilites
  Datagram test_in;

  nout << "READ TEST" << endl;
  nout << "Opening file for reading: " << test_file << endl;
  if (!stream.open(file::FILE_READ)) nout << "Didn't open!" << endl;
  if (!stream.get_datagram(test_in)) nout << "Error reading!" << endl;
  DatagramIterator scan(test_in);
  nout << "Read datagram with data: " << scan.get_string() << endl;
  nout << "Closing file: " << test_file << endl;
  stream.close();
 
#ifdef _ERROR_TEST
  //Test error handling
  Datagram test_error;

  nout << "ERROR TEST" << endl;
  nout << "Opening file for writing: " << test_file  << endl;
  stream.open(file::FILE_WRITE);
  nout << "Trying to read datagram with data: " << endl;
  stream.get_datagram(test_error);
  nout << "Closing file: " << test_file << endl;
  stream.close();
  nout << "Opening file for reading: " << test_file  << endl;
  stream.open(file::FILE_READ);
  nout << "Trying to write datagram with data: " << test_data << endl;
  test_error.add_string(test_data);
  stream.put_datagram(test_error);
  nout << "Closing file: " << test_file << endl;
  stream.close();
#endif

  return 1;
}




