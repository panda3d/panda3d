// Filename: downloader.cxx
// Created by:  shochet (08Sep00)
//
////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////
// Includes
////////////////////////////////////////////////////////////////////
#include "downloadDb.h"
#include "config_downloader.h"
#include <algorithm>

////////////////////////////////////////////////////////////////////
// Defines
////////////////////////////////////////////////////////////////////
PN_uint32 DownloadDb::_magic_number = 0xfeedfeed;

////////////////////////////////////////////////////////////////////
//     Function: DownloadDb::Constructor
//       Access: Public
//  Description: Create a download db with these client and server dbs
////////////////////////////////////////////////////////////////////
DownloadDb::
DownloadDb(Filename &server_file, Filename &client_file) {
  _client_db = read_db(client_file);
  _client_db._filename = client_file;
  _server_db = read_db(server_file);
  _server_db._filename = server_file;

}

////////////////////////////////////////////////////////////////////
//     Function: DownloadDb::Constructor
//       Access: Public
//  Description: Primarily used for testing.
////////////////////////////////////////////////////////////////////
DownloadDb::
DownloadDb(void) {
  _client_db = Db();
  _server_db = Db();
}

////////////////////////////////////////////////////////////////////
//     Function: DownloadDb::Destructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
DownloadDb::
~DownloadDb(void) {
}


////////////////////////////////////////////////////////////////////
//     Function: DownloadDb::output
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
void DownloadDb::
output(ostream &out) const {
  out << "DownloadDb" << endl;
  out << "============================================================" << endl;
  out << "  Client DB file: " << _client_db._filename << endl;
  out << "============================================================" << endl;
  _client_db.output(out);
  out << endl;
  out << "============================================================" << endl;
  out << "  Server DB file: " << _server_db._filename << endl;
  out << "============================================================" << endl;
  _server_db.output(out);
  output_version_map(out);
  out << endl;
}


////////////////////////////////////////////////////////////////////
//     Function: DownloadDb::
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
bool DownloadDb::
write_client_db(Filename &file) {
  return write_db(file, _client_db);
}


////////////////////////////////////////////////////////////////////
//     Function: DownloadDb::
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
bool DownloadDb::
write_server_db(Filename &file) {
  return write_db(file, _server_db);
}

////////////////////////////////////////////////////////////////////
//     Function: DownloadDb::
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
bool DownloadDb::
client_db_current_version(void) const {
  return (_client_db._version == _server_db._version);
}

////////////////////////////////////////////////////////////////////
//     Function: DownloadDb::
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
bool DownloadDb::
client_multifile_exists(string mfname) const {
  return (_client_db.multifile_exists(mfname));
}

////////////////////////////////////////////////////////////////////
//     Function: DownloadDb::
//       Access: Public
//  Description: A multifile is complete when it is completely 
//               downloaded. Note: it may already be decompressed
//               or expanded and it is still complete
////////////////////////////////////////////////////////////////////
bool DownloadDb::
client_multifile_complete(string mfname) const {
  int client_status = _client_db.get_multifile_record_named(mfname)->_status;
  return (client_status >= Status_complete);
}

////////////////////////////////////////////////////////////////////
//     Function: DownloadDb::
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
bool DownloadDb::
client_multifile_decompressed(string mfname) const {
  int client_status = _client_db.get_multifile_record_named(mfname)->_status;
  return (client_status >= Status_decompressed);
}

////////////////////////////////////////////////////////////////////
//     Function: DownloadDb::
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
bool DownloadDb::
client_multifile_expanded(string mfname) const {
  int client_status = _client_db.get_multifile_record_named(mfname)->_status;
  return (client_status >= Status_expanded);  
}

////////////////////////////////////////////////////////////////////
//     Function: DownloadDb::
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
bool DownloadDb::
client_multifile_version_correct(string mfname) const {
  Version client_version = get_client_multifile_version(mfname);
  Version server_version = get_server_multifile_version(mfname);
  return (client_version == server_version);
}

////////////////////////////////////////////////////////////////////
//     Function: DownloadDb::
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
bool DownloadDb::
client_file_version_correct(string mfname, string filename) const {
  Version client_version = get_client_file_version(mfname, filename);
  Version server_version = get_server_file_version(mfname, filename);
  return (client_version == server_version);
}

////////////////////////////////////////////////////////////////////
//     Function: DownloadDb::
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
bool DownloadDb::
client_file_hash_correct(string mfname, string filename) const {
  Hash client_hash = get_client_file_hash(mfname, filename);
  Hash server_hash = get_server_file_hash(mfname, filename);
  return (client_hash == server_hash);
}

// Operations on multifiles

////////////////////////////////////////////////////////////////////
//     Function: DownloadDb::
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
void DownloadDb::
delete_client_multifile(string mfname) {
}

////////////////////////////////////////////////////////////////////
//     Function: DownloadDb::
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
void DownloadDb::
add_client_multifile(string server_mfname) {
  PT(MultifileRecord) server_mfr = _server_db.get_multifile_record_named(server_mfname);
  PT(MultifileRecord) client_mfr = new MultifileRecord;
  client_mfr->_name = server_mfr->_name;
  client_mfr->_phase = server_mfr->_phase;
  client_mfr->_version = server_mfr->_version;
  _client_db.add_multifile_record(client_mfr);
}


////////////////////////////////////////////////////////////////////
//     Function: DownloadDb::
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
void DownloadDb::
expand_client_multifile(string mfname) {
}


////////////////////////////////////////////////////////////////////
//     Function: DownloadDb::
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
DownloadDb::Db DownloadDb::
read_db(Filename &file) {
  // Open the multifile for reading
  ifstream read_stream;
  file.set_binary();
  
  Db db;

  if (!file.open_read(read_stream)) {
    downloader_cat.error()
      << "DownloadDb::read() - Failed to open input file: "
      << file << endl;
    return db;
  }

  if (!db.read(read_stream)) {
    downloader_cat.error()
      << "DownloadDb::read() - Read failed: "
      << file << endl;
    return db;
  }
  if (!read_version_map(read_stream)) {
    downloader_cat.error()
      << "DownloadDb::read() - read_version_map() failed: " 
      << file << endl;
  }

  return db;
}

////////////////////////////////////////////////////////////////////
//     Function: DownloadDb::
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
bool DownloadDb::
write_db(Filename &file, Db db) {
  ofstream write_stream;
  file.set_binary();
  if (!file.open_write(write_stream)) {
    downloader_cat.error()
      << "DownloadDb::write_db() - Failed to open output file: "
      << file << endl;
    return false;
  }
  
  downloader_cat.debug()
    << "Writing to file: " << file << endl;

  db.write(write_stream);
  write_version_map(write_stream);
  write_stream.close();
  return true;
}


////////////////////////////////////////////////////////////////////
//     Function: DownloadDb::create_new_server_db
//       Access: Public
//  Description: Used on the server side makefiles to create a 
//               new clean server db
////////////////////////////////////////////////////////////////////
void DownloadDb::
create_new_server_db(void) {
  _server_db = Db();
}


////////////////////////////////////////////////////////////////////
//     Function: DownloadDb::
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
void DownloadDb::
server_add_multifile(string mfname, Phase phase, Version version, int size, int status) {
  PT(MultifileRecord) mfr = new MultifileRecord(mfname, phase, version, size, status);
  _server_db.add_multifile_record(mfr);
}


////////////////////////////////////////////////////////////////////
//     Function: DownloadDb::
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
void DownloadDb::
server_add_file(string mfname, string fname, Version version, Hash hash) {
  // Make the new file record
  PT(FileRecord) fr = new FileRecord(fname, version, hash);

  // Find the multifile with mfname
  vector<PT(MultifileRecord)>::iterator i = _server_db._mfile_records.begin();
  for(; i != _server_db._mfile_records.end(); ++i) {
    if (mfname == (*i)->_name) {
      (*i)->add_file_record(fr);
      return;
    }
  }

  // Uh-oh, did not find it
  downloader_cat.error() << "Could not find record named " 
			 << mfname << " in database " << endl;
  return;
}


////////////////////////////////////////////////////////////////////
// Multifile methods
////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////
//     Function: DownloadDb::MultifileRecord::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
DownloadDb::MultifileRecord::
MultifileRecord(void) {
  _name = "";
  _phase = 0;
  _version = 0;
  _size = 0;
  _status = Status_incomplete;
}


////////////////////////////////////////////////////////////////////
//     Function: DownloadDb::MultifileRecord::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
DownloadDb::MultifileRecord::
MultifileRecord(string name, Phase phase, Version version, int size, int status) {
  _name = name;
  _phase = phase;
  _version = version;
  _size = size;
  _status = status;
}


////////////////////////////////////////////////////////////////////
//     Function: DownloadDb::MultifileRecord::output
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
void DownloadDb::MultifileRecord::
output(ostream &out) const {
  out << "==================================================" << endl;
  out << "MultifileRecord: " << _name    << endl
      << "    phase: " << _phase   << endl
      << "  version: " << _version << endl
      << "     size: " << _size    << endl
      << "   status: " << _status  << endl;
  out << "--------------------------------------------------" << endl;
  vector<PT(FileRecord)>::const_iterator i = _file_records.begin();
  for(; i != _file_records.end(); ++i) {
    (*i)->output(out);
  }
}



////////////////////////////////////////////////////////////////////
//     Function: DownloadDb::MultifileRecord::
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
int DownloadDb::MultifileRecord::
get_num_files(void) const {
  return _file_records.size();
}

////////////////////////////////////////////////////////////////////
//     Function: DownloadDb::MultifileRecord::
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
string DownloadDb::MultifileRecord::
get_file_name(int index) const {
  return _file_records[index]->_name;
}


////////////////////////////////////////////////////////////////////
//     Function: DownloadDb::MultifileRecord::
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
bool DownloadDb::MultifileRecord::
file_exists(string fname) const {
  vector<PT(FileRecord)>::const_iterator i = _file_records.begin();
  for(; i != _file_records.end(); ++i) {
    if (fname == (*i)->_name) {
      return true;
    }
  }
  return false;
}


////////////////////////////////////////////////////////////////////
//     Function: DownloadDb::MultifileRecord::
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
PT(DownloadDb::FileRecord) DownloadDb::MultifileRecord::
get_file_record_named(string fname) const {
  vector<PT(FileRecord)>::const_iterator i = _file_records.begin();
  for(; i != _file_records.end(); ++i) {
    if (fname == (*i)->_name) {
      return (*i);
    }
  }
  // Did not find it, just return an empty version
  downloader_cat.error() << "Could not find record named " 
			 << fname << " in multifile " << _name << endl;
  PT(FileRecord) foo = new FileRecord;
  return foo;
}


////////////////////////////////////////////////////////////////////
//     Function: DownloadDb::MultifileRecord::
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
void DownloadDb::MultifileRecord::
add_file_record(PT(FileRecord) fr) {
  _file_records.push_back(fr);
}



////////////////////////////////////////////////////////////////////
// Db methods
////////////////////////////////////////////////////////////////////




////////////////////////////////////////////////////////////////////
//     Function: DownloadDb::Db::constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
DownloadDb::Db::
Db(void) {
  // The head is a magic number and the number of multifiles in the db
  _header_length = sizeof(_magic_number) + sizeof(PN_int32);
}


////////////////////////////////////////////////////////////////////
//     Function: DownloadDb::Db::output
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
void DownloadDb::Db::
output(ostream &out) const {
  vector<PT(MultifileRecord)>::const_iterator i = _mfile_records.begin();
  for(; i != _mfile_records.end(); ++i) {
    (*i)->output(out);
  }
}


////////////////////////////////////////////////////////////////////
//     Function: DownloadDb::Db::
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
int DownloadDb::Db::
get_num_multifiles(void) const {
  return _mfile_records.size();
}

////////////////////////////////////////////////////////////////////
//     Function: DownloadDb::Db::
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
string DownloadDb::Db::
get_multifile_name(int index) const {
  return _mfile_records[index]->_name;
}

////////////////////////////////////////////////////////////////////
//     Function: DownloadDb::Db::
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
bool DownloadDb::Db::
multifile_exists(string mfname) const {
  vector<PT(MultifileRecord)>::const_iterator i = _mfile_records.begin();
  for(; i != _mfile_records.end(); ++i) {
    if (mfname == (*i)->_name) {
      return true;
    }
  }
  return false;
}

////////////////////////////////////////////////////////////////////
//     Function: DownloadDb::Db::
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
PT(DownloadDb::MultifileRecord) DownloadDb::Db::
get_multifile_record_named(string mfname) const {
  vector<PT(MultifileRecord)>::const_iterator i = _mfile_records.begin();
  for(; i != _mfile_records.end(); ++i) {
    if (mfname == (*i)->_name) {
      return (*i);
    }
  }
  // Did not find it, just return an empty version
  downloader_cat.error() << "Could not find record named " 
			 << mfname << " in database " << endl;
  PT(MultifileRecord) foo = new MultifileRecord;
  return foo;
}

////////////////////////////////////////////////////////////////////
//     Function: DownloadDb::Db::
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
void DownloadDb::Db::
add_multifile_record(PT(MultifileRecord) mfr) {
  _mfile_records.push_back(mfr);
}


////////////////////////////////////////////////////////////////////
//     Function: DownloadDb::Db::parse_header
//       Access: Private
//  Description: Verifies magic number, returns the number of 
//               multifiles or -1 if invalid
////////////////////////////////////////////////////////////////////
int DownloadDb::Db::
parse_header(uchar *start, int size) {
  _datagram.clear();
  _datagram.append_data(start, size);

  // Make sure we have a good header
  DatagramIterator di(_datagram);
  PN_uint32 magic_number = di.get_uint32();
  downloader_cat.debug()
    << "Parsed magic number: " << magic_number << endl;
  if (magic_number != _magic_number) {
    downloader_cat.error()
      << "DownloadDb::parse_header() - Invalid magic number: " 
      << magic_number << " expected: " << _magic_number << endl;
    return -1;
  }

  PN_int32 num_multifiles = di.get_int32();
  downloader_cat.debug()
    << "Parsed number of multifiles: " << num_multifiles << endl;

  // If we got all the way here, must be done
  return num_multifiles;
}



////////////////////////////////////////////////////////////////////
//     Function: DownloadDb::Db::parse_fr_header
//       Access: Private
//  Description: Parses a file record (fr) header and returns
//               the length of the next file record
////////////////////////////////////////////////////////////////////
int DownloadDb::Db::
parse_record_header(uchar *start, int size) {
  _datagram.clear();
  _datagram.append_data(start, size);

  DatagramIterator di(_datagram);
  PN_int32 record_length = di.get_int32();
  downloader_cat.debug()
    << "Parsed record header length: " << record_length << endl;

  // If we got all the way here, must be done
  return record_length;
}


////////////////////////////////////////////////////////////////////
//     Function: DownloadDb::Db::parse_mfr
//       Access: Private
//  Description: Parses a multifile record (mfr) and returns one
////////////////////////////////////////////////////////////////////
PT(DownloadDb::MultifileRecord) DownloadDb::Db::
parse_mfr(uchar *start, int size) {

  PT(DownloadDb::MultifileRecord) mfr = new DownloadDb::MultifileRecord;

  _datagram.clear();
  _datagram.append_data(start, size);

  DatagramIterator di(_datagram);
  PN_int32 mfr_name_length = di.get_int32();
  mfr->_name = di.extract_bytes(mfr_name_length);
  mfr->_phase = di.get_int32();
  mfr->_version = di.get_int32();
  mfr->_size = di.get_int32();
  mfr->_status = di.get_int32();
  mfr->_num_files = di.get_int32();
  
  downloader_cat.debug()
    << "Parsed multifile record: " << mfr->_name << " phase: " << mfr->_phase 
    << " version: " << mfr->_version << " size: " << mfr->_size
    << " status: " << mfr->_status << " num_files: " << mfr->_num_files << endl;

  // Return the new MultifileRecord
  return mfr;
}




////////////////////////////////////////////////////////////////////
//     Function: DownloadDb::Db::parse_fr
//       Access: Private
//  Description: Parses a file record (fr) and returns one
////////////////////////////////////////////////////////////////////
PT(DownloadDb::FileRecord) DownloadDb::Db::
parse_fr(uchar *start, int size) {

  PT(DownloadDb::FileRecord) fr = new DownloadDb::FileRecord;

  _datagram.clear();
  _datagram.append_data(start, size);

  DatagramIterator di(_datagram);
  PN_int32 fr_name_length = di.get_int32();
  fr->_name = di.extract_bytes(fr_name_length);
  fr->_version = di.get_int32();
  
  downloader_cat.debug()
    << "Parsed file record: " << fr->_name 
    << " version: " << fr->_version 
    << " hash: " << fr->_hash 
    << endl;

  // Return the new MultifileRecord
  return fr;
}




////////////////////////////////////////////////////////////////////
//     Function: DownloadDb::Db::read
//       Access: Private
//  Description: 
////////////////////////////////////////////////////////////////////
bool DownloadDb::Db::
read(ifstream &read_stream) {
  
  // Make a little buffer to read the header into
  uchar *header_buf = new uchar[_header_length];
  // Read the header
  read_stream.read((char *)header_buf, _header_length);

  // Parse the header
  int num_multifiles = parse_header(header_buf, _header_length);
  if (num_multifiles == -1) {
    delete header_buf;
    downloader_cat.error() << "DownloadDb::read() - Invalid header" << endl;
    return false;
  }

  delete header_buf;  

  // Now that we know how many multifiles this db has, we can iterate
  // reading them off one by one
  for (int i = 0; i < num_multifiles; i++) {
    // The multifile record header is just one int which
    // represents the size of the record
    int mfr_header_length = sizeof(PN_int32);

    // Make a little buffer to read the multifile record header into
    header_buf = new uchar[mfr_header_length];

    // Read the header
    read_stream.read((char *)header_buf, mfr_header_length);

    // Parse the header
    int mfr_length = parse_record_header(header_buf, mfr_header_length);
    delete header_buf;
    
    // Ok, now that we know the size of the mfr, read it in
    // Make a buffer to read the multifile record into
    header_buf = new uchar[mfr_length];

    // Read the mfr -- do not count the header length twice
    read_stream.read((char *)header_buf, (mfr_length - mfr_header_length));

    // Parse the mfr
    PT(DownloadDb::MultifileRecord) mfr = parse_mfr(header_buf, mfr_length);
    delete header_buf;

    // Read off all the file records this multifile has
    for (int j = 0; j<mfr->_num_files; j++) {
      // The file record header is just one int which
      // represents the size of the record
      int fr_header_length = sizeof(PN_int32);

      // Make a little buffer to read the file record header into
      header_buf = new uchar[fr_header_length];

      // Read the header
      read_stream.read((char *)header_buf, fr_header_length);

      // Parse the header
      int fr_length = parse_record_header(header_buf, fr_header_length);
      delete header_buf;
      
      // Ok, now that we know the size of the mfr, read it in
      // Make a buffer to read the file record into
      header_buf = new uchar[fr_length];

      // Read the file record -- do not count the header length twice
      read_stream.read((char *)header_buf, (fr_length - fr_header_length));

      // Parse the file recrod
      PT(DownloadDb::FileRecord) fr = parse_fr(header_buf, fr_length);

      // Add this file record to the current multifilerecord
      mfr->add_file_record(fr);
    }

    // Add the current multifilerecord to our database
    add_multifile_record(mfr);

  }

  return true;
}



////////////////////////////////////////////////////////////////////
//     Function: DownloadDb::Db::write
//       Access: Private
//  Description: 
////////////////////////////////////////////////////////////////////
bool DownloadDb::Db::
write(ofstream &write_stream) {
  write_header(write_stream);


  // Iterate over the multifiles writing them to the stream
  vector<PT(MultifileRecord)>::const_iterator i = _mfile_records.begin();
  for(; i != _mfile_records.end(); ++i) {
    _datagram.clear();
    
    // Cache some properties so we do not have to keep asking for them
    PN_int32 phase = (*i)->_phase;
    PN_int32 version = (*i)->_version;
    PN_int32 size = (*i)->_size;
    PN_int32 status = (*i)->_status;
    PN_int32 num_files = (*i)->get_num_files();
    PN_int32 name_length = (*i)->_name.length();

    // Compute the length of this datagram
    PN_int32 header_length;
    header_length = 
      sizeof(header_length) +  // Size of this header length
      sizeof(name_length) +    // Size of the size of the name string
      (*i)->_name.length() +      // Size of the name string
      sizeof(phase) + sizeof(version) + sizeof(size) + 
      sizeof(status) + sizeof(num_files);
      
    // Add the length of this entire datagram
    _datagram.add_int32(header_length);

    // Add the length of the name
    _datagram.add_int32(name_length);
    // Add the name
    _datagram.append_data((*i)->_name.c_str(), (*i)->_name.length());

    // Add all the properties
    _datagram.add_int32(phase);
    _datagram.add_int32(version);
    _datagram.add_int32(size);
    _datagram.add_int32(status);
    _datagram.add_int32(num_files);
    
    // Now put this datagram on the write stream
    string msg = _datagram.get_message();
    write_stream.write(msg.data(), msg.length());

    // Now iterate over this multifile's files writing them to the stream
    // Iterate over the multifiles writing them to the stream
    vector<PT(FileRecord)>::const_iterator j = (*i)->_file_records.begin();
    for(; j != (*i)->_file_records.end(); ++j) {
      // Clear the datagram before we jam a bunch of stuff on it
      _datagram.clear();

      version = (*j)->_version;
      name_length = (*j)->_name.length();

      // Compute the length of this datagram
      header_length = 
	sizeof(header_length) +  // Size of this header length
	sizeof(name_length) +    // Size of the size of the name string
	(*j)->_name.length() +      // Size of the name string
	sizeof(version);
      
      // Add the length of this entire datagram
      _datagram.add_int32(header_length);

      // Add the length of the name
      _datagram.add_int32(name_length);
      // Add the name
      _datagram.append_data((*j)->_name.c_str(), (*j)->_name.length());

      // Add all the properties
      _datagram.add_int32(version);

      // Now put this datagram on the write stream
      string msg = _datagram.get_message();
      write_stream.write(msg.data(), msg.length());
    }

  }

  return true;
}






////////////////////////////////////////////////////////////////////
//     Function: DownloadDb::Db::write_header
//       Access: Private
//  Description: Writes the header uncompressed with platform-
//               independent byte ordering
////////////////////////////////////////////////////////////////////
bool DownloadDb::Db::
write_header(ofstream &write_stream) {
  _datagram.clear();

  // Write the db magic number
  _datagram.add_uint32(_magic_number);

  // Write the number of multifiles
  _datagram.add_int32(get_num_multifiles());

  string msg = _datagram.get_message();
  write_stream.write(msg.data(), msg.length());
  return true;
}


////////////////////////////////////////////////////////////////////
// FileRecord methods
////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////
//     Function: DownloadDb::FileRecord::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
DownloadDb::FileRecord::
FileRecord(void) {
  _name = "";
  _version = 0;
  _hash = 0;
}


////////////////////////////////////////////////////////////////////
//     Function: DownloadDb::FileRecord::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
DownloadDb::FileRecord::
FileRecord(string name, Version version, Hash hash) {
  _name = name;
  _version = version;
  _hash = hash;
}

////////////////////////////////////////////////////////////////////
//     Function: DownloadDb::FileRecord::output
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
void DownloadDb::FileRecord::
output(ostream &out) const {
  out << " FileRecord: " << _name 
      << " version: " << _version 
      << " hash: " << _hash
      << endl;
}

////////////////////////////////////////////////////////////////////
//     Function: DownloadDb::add_version
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
void DownloadDb::
add_version(const Filename &name, Hash hash, Version version) {
  add_version(name.get_fullpath(), hash, version);
}

////////////////////////////////////////////////////////////////////
//     Function: DownloadDb::add_version
//       Access: Public
//  Description: Note: version numbers start at 1
////////////////////////////////////////////////////////////////////
void DownloadDb::
add_version(const string &name, Hash hash, Version version) {
  // Try to find this name in the map
  VersionMap::iterator i = _versions.find(name);
  nassertv(version >= 1);

  // If we did not find it, put a new vector_ulong at this name_code
  if (i == _versions.end()) {
    vector_ulong v;
    nassertv(version == 1);
    v.push_back(hash);
    _versions[name] = v;
  } else {
    int size = (*i).second.size();

    // Assert that this version is the next version in the list
    nassertv(version<=size);

    // If you are overwriting an old hash value, just insert the new value
    if (version-1 < size) {
      (*i).second[version-1] = hash;
    } else {
      //  add this hash at the end of the vector
      (*i).second.push_back(hash);
    }
  }
}

////////////////////////////////////////////////////////////////////
//     Function: DownloadDb::get_version
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
int DownloadDb::
get_version(const Filename &name, Hash hash) {
  return get_version(name.get_fullpath(), hash);
}

////////////////////////////////////////////////////////////////////
//     Function: DownloadDb::get_version
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
int DownloadDb::
get_version(const string &name, Hash hash) {
  VersionMap::const_iterator vmi = _versions.find(name);
  if (vmi == _versions.end()) {
    downloader_cat.debug()
      << "DownloadDb::get_version() - can't find: " << name << endl;
    return -1;
  }
  vector_ulong ulvec = (*vmi).second;
  vector_ulong::iterator i = find(ulvec.begin(), ulvec.end(), hash);
  if (i != ulvec.end())
    return (i - ulvec.begin() + 1);
  downloader_cat.debug()
    << "DownloadDb::get_version() - can't find hash: " << hash << endl;
  return -1;
}

////////////////////////////////////////////////////////////////////
//     Function: DownloadDb::write_version_map
//       Access: Protected
//  Description:
////////////////////////////////////////////////////////////////////
void DownloadDb::
write_version_map(ofstream &write_stream) {
  _master_datagram.clear();
  VersionMap::iterator vmi;
  vector_ulong::iterator i;
  _master_datagram.add_int32(_versions.size());
  for (vmi = _versions.begin(); vmi != _versions.end(); ++vmi) {
    string name = (*vmi).first;
    downloader_cat.debug()
      << "DownloadDb::write_version_map() - writing file: "
      << name << " of length: " << name.length() << endl;
    _master_datagram.add_int32(name.length());
    _master_datagram.append_data(name.c_str(), name.length());
    _master_datagram.add_int32((*vmi).second.size());
    for (i = (*vmi).second.begin(); i != (*vmi).second.end(); ++i)
      _master_datagram.add_uint64((*i));
  }
  string msg = _master_datagram.get_message();
  write_stream.write((char *)msg.data(), msg.length());
}

////////////////////////////////////////////////////////////////////
//     Function: DownloadDb::read_version_map
//       Access: Protected
//  Description:
////////////////////////////////////////////////////////////////////
bool DownloadDb::
read_version_map(ifstream &read_stream) {
  _master_datagram.clear();
  char *buffer = new char[sizeof(PN_uint64)];
  read_stream.read(buffer, sizeof(PN_int32));
  _master_datagram.append_data(buffer, sizeof(PN_int32));
  DatagramIterator di(_master_datagram);
  int num_entries = di.get_int32();

  for (int i = 0; i < num_entries; i++) {

    // Get the length of the file name
    _master_datagram.clear();
    read_stream.read(buffer, sizeof(PN_int32)); 
    _master_datagram.append_data(buffer, sizeof(PN_int32)); 
    DatagramIterator di2(_master_datagram);
    int name_length = di2.get_int32();
    downloader_cat.debug()
      << "DownloadDb::read_version_map() - name length: " << name_length
      << endl;

    // Get the file name
    _master_datagram.clear();
    char *namebuffer = new char[name_length];
    read_stream.read(namebuffer, name_length);
    _master_datagram.append_data(namebuffer, name_length);
    DatagramIterator di4(_master_datagram);
    string name = di4.extract_bytes(name_length);
    downloader_cat.debug()
      << "DownloadDb::read_version_map() - name: " << name << endl;

    // Get number of hash values for name
    _master_datagram.clear();
    read_stream.read(buffer, sizeof(PN_int32));
    _master_datagram.append_data(buffer, sizeof(PN_int32));
    DatagramIterator di5(_master_datagram);
    int length = di5.get_int32();   
    downloader_cat.debug()
      << "DownloadDb::read_version_map() - number of values: " << length
      << endl;

    for (int j = 0; j < length; j++) {
      _master_datagram.clear();
      read_stream.read(buffer, sizeof(PN_uint64));
      _master_datagram.append_data(buffer, sizeof(PN_uint64));
      DatagramIterator di3(_master_datagram);
      int hash = di3.get_uint64();
      add_version(name, hash, j + 1);
    }
    delete namebuffer;
  }
  delete buffer;
  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: DownloadDb::output_version_map
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
void DownloadDb::
output_version_map(ostream &out) const {
  out << " Version Map: " << endl;
  VersionMap::const_iterator vmi;
  vector_ulong::const_iterator i;
  for (vmi = _versions.begin(); vmi != _versions.end(); ++vmi) {
    out << "  Filename: " << (*vmi).first;
    for (i = (*vmi).second.begin(); i != (*vmi).second.end(); ++i)
      out << " " << (*i);
    out << endl;
  }
  out << endl;
}
