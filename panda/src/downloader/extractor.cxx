// Filename: extractor.cxx
// Created by:  mike (09Jan97)
//
////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////
// Includes
////////////////////////////////////////////////////////////////////
#include "extractor.h"
#include "config_downloader.h"

#include <filename.h>

////////////////////////////////////////////////////////////////////
// Defines
////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////
//     Function: Extractor::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
Extractor::
Extractor(void) {
  PT(Buffer) buffer = new Buffer(extractor_buffer_size);
  init(buffer);
}

////////////////////////////////////////////////////////////////////
//     Function: Extractor::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
Extractor::
Extractor(PT(Buffer) buffer) {
  init(buffer);
}

////////////////////////////////////////////////////////////////////
//     Function: Extractor::Constructor
//       Access: Private 
//  Description:
////////////////////////////////////////////////////////////////////
void Extractor::
init(PT(Buffer) buffer) {
  nassertv(!buffer.is_null());
  _buffer = buffer;
}

////////////////////////////////////////////////////////////////////
//     Function: Extractor::Destructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
Extractor::
~Extractor(void) {
}

////////////////////////////////////////////////////////////////////
//     Function: Extractor::initiate
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
int Extractor::
initiate(Filename &source_file, const Filename &rel_path) {

  // Open source file
  _source_file = source_file;
  _source_file.set_binary();
  if (!_source_file.open_read(_read_stream)) {
    downloader_cat.error()
      << "Extractor::extract() - Error opening source file: " 
      << _source_file << endl;
    return ES_error_write;
  } 

  _rel_path = rel_path;

  // Determine source file length
  _read_stream.seekg(0, ios::end);
  _source_file_length = _read_stream.tellg();
  _read_stream.seekg(0, ios::beg);

  // Read from the source file and write to the appropriate extracted file
  _total_bytes_read = 0;
  _read_all_input = false;
  _handled_all_input = false;
  return ES_success;
}

////////////////////////////////////////////////////////////////////
//     Function: Extractor::run
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
int Extractor::
run(void) {
  // See if there is anything left in the source file
  if (_read_all_input == false) {
    _read_stream.read(_buffer->_buffer, _buffer->get_length());
    _source_buffer_length = _read_stream.gcount();
    _total_bytes_read += _source_buffer_length;
    if (_read_stream.eof()) {
      nassertr(_total_bytes_read == _source_file_length, false);
      _read_all_input = true;
    }
  }

  // Write to the out file
  char *start = _buffer->_buffer;
  int size = _source_buffer_length; 
  if (_mfile.write_extract(start, size, _rel_path) == true) {
    _read_stream.close();
    _source_file.unlink();
    return ES_success;
  }
  return ES_ok;
}

////////////////////////////////////////////////////////////////////
//     Function: Extractor::extract
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
bool Extractor::
extract(Filename &source_file, const Filename &rel_path) {
  int ret = initiate(source_file, rel_path);
  if (ret < 0)
    return false;
  for (;;) {
    ret = run();
    if (ret == ES_success)
      return true;
    if (ret < 0)
      return false;
  }
  return false;
}
