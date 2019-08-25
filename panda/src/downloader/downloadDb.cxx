/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file downloadDb.cxx
 * @author shochet
 * @date 2000-09-08
 */

#include "config_downloader.h"
#include "downloadDb.h"
#include "streamReader.h"
#include "streamWriter.h"
#include "ramfile.h"
#include "virtualFileSystem.h"

#include <algorithm>

using std::endl;
using std::istream;
using std::istringstream;
using std::move;
using std::ostream;
using std::string;

// Defines

// Written at the top of the file so we know this is a downloadDb
uint32_t DownloadDb::_magic_number = 0xfeedfeed;

// Written at the top of the file to signify we are not done writing to the
// file yet.  If you load a db with this magic number that means the previous
// time it got written out was probably interrupted in the middle of the
// write.
uint32_t DownloadDb::_bogus_magic_number = 0x11111111;


static string
back_to_front_slash(const string &str) {
  string result = str;
  string::iterator si;
  for (si = result.begin(); si != result.end(); ++si) {
    if ((*si) == '\\') {
      (*si) = '/';
    }
  }

  return result;
}


/**
 * Create a download db with these client and server dbs
 */
DownloadDb::
DownloadDb(Ramfile &server_file, Filename &client_file) {
  if (downloader_cat.is_debug()) {
    downloader_cat.debug()
      << "DownloadDb constructor called" << endl;
  }
  _client_db = read_db(client_file, 0);
  _client_db._filename = client_file;
  _server_db = read_db(server_file, 1);
}

/**
 * Create a download db with these client and server dbs
 */
DownloadDb::
DownloadDb(Filename &server_file, Filename &client_file) {
  if (downloader_cat.is_debug()) {
    downloader_cat.debug()
      << "DownloadDb constructor called" << endl;
  }
  _client_db = read_db(client_file, 0);
  _client_db._filename = client_file;
  _server_db = read_db(server_file, 1);
  _server_db._filename = server_file;
}

/**
 * Primarily used for testing.
 */
DownloadDb::
DownloadDb() {
  _client_db = Db();
  _server_db = Db();
}

/**
 *
 */
DownloadDb::
~DownloadDb() {
  if (downloader_cat.is_debug()) {
    downloader_cat.debug()
      << "DownloadDb destructor called" << endl;
  }
}


/**
 *
 */
void DownloadDb::
output(ostream &out) const {
  out << "[" << _server_db._filename << " " << _client_db._filename << "]";
}

/**
 *
 */
void DownloadDb::
write(ostream &out) const {
  out << "DownloadDb" << endl;
  out << "============================================================" << endl;
  out << "  Client DB file: " << _client_db._filename << endl;
  out << "============================================================" << endl;
  _client_db.write(out);
  out << endl;
  out << "============================================================" << endl;
  out << "  Server DB file: " << _server_db._filename << endl;
  out << "============================================================" << endl;
  _server_db.write(out);
  write_version_map(out);
  out << endl;
}


/**
 *
 */
bool DownloadDb::
write_client_db(Filename &file) {
  return write_db(file, _client_db, 0);
}


/**
 *
 */
bool DownloadDb::
write_server_db(Filename &file) {
  return write_db(file, _server_db, 1);
}

/**
 *
 */
bool DownloadDb::
client_multifile_exists(string mfname) const {
  return (_client_db.multifile_exists(mfname));
}

/**
 * A multifile is complete when it is completely downloaded.  Note: it may
 * already be decompressed or extracted and it is still complete
 */
bool DownloadDb::
client_multifile_complete(string mfname) const {
  int client_status = _client_db.get_multifile_record_named(mfname)->_status;
  return (client_status >= Status_complete);
}

/**
 *
 */
bool DownloadDb::
client_multifile_decompressed(string mfname) const {
  int client_status = _client_db.get_multifile_record_named(mfname)->_status;
  return (client_status >= Status_decompressed);
}

/**
 *
 */
bool DownloadDb::
client_multifile_extracted(string mfname) const {
  int client_status = _client_db.get_multifile_record_named(mfname)->_status;
  return (client_status >= Status_extracted);
}


/**
 * Return the hash value of the file we are working on
 */
HashVal DownloadDb::
get_client_multifile_hash(string mfname) const {
  return _client_db.get_multifile_record_named(mfname)->_hash;
}


/**
 * Return the hash value of the server file
 */
HashVal DownloadDb::
get_server_multifile_hash(string mfname) const {
  return _server_db.get_multifile_record_named(mfname)->_hash;
}


/**
 * Set the hash value of file we are working on
 */
void DownloadDb::
set_client_multifile_hash(string mfname, HashVal val) {
  _client_db.get_multifile_record_named(mfname)->_hash = val;
  write_client_db(_client_db._filename);
}


/**
 * Set the hash value of file we are working on
 */
void DownloadDb::
set_server_multifile_hash(string mfname, HashVal val) {
  _server_db.get_multifile_record_named(mfname)->_hash = val;
}

// Operations on multifiles

/**
 *
 */
void DownloadDb::
delete_client_multifile(string mfname) {
}

/**
 *
 */
void DownloadDb::
add_client_multifile(string server_mfname) {
  PT(MultifileRecord) server_mfr = _server_db.get_multifile_record_named(server_mfname);
  PT(MultifileRecord) client_mfr = new MultifileRecord;
  client_mfr->_name = server_mfr->_name;
  client_mfr->_phase = server_mfr->_phase;
  _client_db.add_multifile_record(client_mfr);
}


/**
 *
 */
void DownloadDb::
expand_client_multifile(string mfname) {
}


/**
 *
 */
DownloadDb::Db DownloadDb::
read_db(Filename &file, bool want_server_info) {
  VirtualFileSystem *vfs = VirtualFileSystem::get_global_ptr();
  Db db;

  // Open the multifile for reading
  file.set_binary();
  istream *read_stream = vfs->open_read_file(file, true);

  if (read_stream == nullptr) {
    downloader_cat.error()
      << "failed to open input file: "
      << file << endl;
    return db;
  }

  StreamReader sr(*read_stream);
  if (!db.read(sr, want_server_info)) {
    downloader_cat.error()
      << "read failed: "
      << file << endl;
    vfs->close_read_file(read_stream);
    return db;
  }

  if (want_server_info) {
    if (!read_version_map(sr)) {
      downloader_cat.error()
        << "read_version_map() failed: "
        << file << endl;
    }
  }

  vfs->close_read_file(read_stream);

  return db;
}

/**
 *
 */
DownloadDb::Db DownloadDb::
read_db(Ramfile &file, bool want_server_info) {
  // Open the multifile for reading
  istringstream read_stream(file._data);
  Db db;

  StreamReader sr(read_stream);

  if (!db.read(sr, want_server_info)) {
    downloader_cat.error()
      << "read failed" << endl;
    return db;
  }
  if (want_server_info) {
    if (!read_version_map(sr)) {
      downloader_cat.error()
        << "read_version_map() failed" << endl;
    }
  }

  return db;
}


/**
 *
 */
bool DownloadDb::
write_db(Filename &file, Db db, bool want_server_info) {
  pofstream write_stream;
  file.set_binary();
  if (!file.open_write(write_stream)) {
    downloader_cat.error()
      << "DownloadDb::write_db() - Failed to open output file: "
      << file << endl;
    return false;
  }

  if (downloader_cat.is_spam()) {
    downloader_cat.spam()
      << "Writing to file: " << file << endl;
  }

  StreamWriter sw(write_stream);

  // Initialize the write stream with a bogus header
  db.write_bogus_header(sw);
  db.write(sw, want_server_info);
  if (want_server_info) {
    write_version_map(sw);
  }
  // Now write the real header
  db.write_header(write_stream);
  write_stream.close();
  return true;
}


/**
 * Used on the server side makefiles to create a new clean server db
 */
void DownloadDb::
create_new_server_db() {
  _server_db = Db();
}


/**
 *
 */
void DownloadDb::
server_add_multifile(string mfname, Phase phase, int size, int status) {
  PT(MultifileRecord) mfr = new MultifileRecord(mfname, phase, size, status);
  _server_db.add_multifile_record(mfr);
}


/**
 *
 */
void DownloadDb::
server_add_file(string mfname, string fname) {
  // Make the new file record
  PT(FileRecord) fr = new FileRecord(fname);

  // Find the multifile with mfname
  pvector< PT(MultifileRecord) >::iterator i = _server_db._mfile_records.begin();
  for (; i != _server_db._mfile_records.end(); ++i) {
    if (mfname == (*i)->_name) {
      (*i)->add_file_record(fr);
      return;
    }
  }

  // Uh-oh, did not find it
  downloader_cat.error() << "Could not find record named "
                         << mfname << " in database " << endl;
  nassertv(false);
  return;
}


// Multifile methods


/**
 *
 */
DownloadDb::MultifileRecord::
MultifileRecord() {
  _name = "";
  _phase = 0;
  _size = 0;
  _status = Status_incomplete;
}


/**
 *
 */
DownloadDb::MultifileRecord::
MultifileRecord(string name, Phase phase, int size, int status) {
  _name = name;
  _phase = phase;
  _size = size;
  _status = status;
}


/**
 *
 */
void DownloadDb::MultifileRecord::
write(ostream &out) const {
  out << "==================================================" << endl;
  out << "MultifileRecord: " << _name    << endl
      << "    phase: " << _phase   << endl
      << "     size: " << _size    << endl
      << "   status: " << _status  << endl
      << "     hash: " << _hash.as_dec() << endl;
  out << "--------------------------------------------------" << endl;
  pvector< PT(FileRecord) >::const_iterator i = _file_records.begin();
  for(; i != _file_records.end(); ++i) {
    (*i)->write(out);
  }
}



/**
 *
 */
int DownloadDb::MultifileRecord::
get_num_files() const {
  return _file_records.size();
}

/**
 *
 */
string DownloadDb::MultifileRecord::
get_file_name(int index) const {
  return _file_records[index]->_name;
}


/**
 *
 */
bool DownloadDb::MultifileRecord::
file_exists(string fname) const {
  pvector< PT(FileRecord) >::const_iterator i = _file_records.begin();
  for(; i != _file_records.end(); ++i) {
    if (fname == (*i)->_name) {
      return true;
    }
  }
  return false;
}


/**
 *
 */
PT(DownloadDb::FileRecord) DownloadDb::MultifileRecord::
get_file_record_named(string fname) const {
  pvector< PT(FileRecord) >::const_iterator i = _file_records.begin();
  for(; i != _file_records.end(); ++i) {
    if (fname == (*i)->_name) {
      return (*i);
    }
  }
  // Did not find it, just return an empty version
  downloader_cat.error() << "Could not find record named "
                         << fname << " in multifile " << _name << endl;
  PT(FileRecord) foo = new FileRecord;
  nassertr(false, foo);
  return foo;
}


/**
 *
 */
void DownloadDb::MultifileRecord::
add_file_record(PT(FileRecord) fr) {
  _file_records.push_back(fr);
}



// Db methods




/**
 *
 */
DownloadDb::Db::
Db() {
  // The head is a magic number and the number of multifiles in the db
  _header_length = sizeof(_magic_number) + sizeof(int32_t);
}


/**
 *
 */
void DownloadDb::Db::
write(ostream &out) const {
  pvector< PT(MultifileRecord) >::const_iterator i = _mfile_records.begin();
  for(; i != _mfile_records.end(); ++i) {
    (*i)->write(out);
  }
}


/**
 *
 */
int DownloadDb::Db::
get_num_multifiles() const {
  return _mfile_records.size();
}

/**
 *
 */
string DownloadDb::Db::
get_multifile_name(int index) const {
  return _mfile_records[index]->_name;
}

/**
 *
 */
bool DownloadDb::Db::
multifile_exists(string mfname) const {
  pvector< PT(MultifileRecord) >::const_iterator i = _mfile_records.begin();
  for(; i != _mfile_records.end(); ++i) {
    if (mfname == (*i)->_name) {
      return true;
    }
  }
  return false;
}

/**
 *
 */
PT(DownloadDb::MultifileRecord) DownloadDb::Db::
get_multifile_record_named(string mfname) const {
  pvector< PT(MultifileRecord) >::const_iterator i = _mfile_records.begin();
  for(; i != _mfile_records.end(); ++i) {
    if (mfname == (*i)->_name) {
      return (*i);
    }
  }
  // Did not find it, just return an empty version
  downloader_cat.error() << "Could not find record named "
                         << mfname << " in database " << endl;
  PT(MultifileRecord) foo = new MultifileRecord;
  nassertr(false, foo);
  return foo;
}

/**
 *
 */
void DownloadDb::Db::
add_multifile_record(PT(MultifileRecord) mfr) {
  _mfile_records.push_back(mfr);
}


/**
 * Verifies magic number, returns the number of multifiles or -1 if invalid
 */
int DownloadDb::Db::
parse_header(Datagram dg) {
  // Make sure we have a good header
  DatagramIterator di(dg);
  uint32_t magic_number = di.get_uint32();
  if (downloader_cat.is_debug()) {
    downloader_cat.debug()
      << "Parsed magic number: " << magic_number << endl;
  }
  // If the magic number is equal to the bogus magic number it signifies that
  // the previous write was interrupted
  if (magic_number == _bogus_magic_number) {
    downloader_cat.error()
      << "DownloadDb::parse_header() - "
      << "Bogus magic number, previous write incomplete: "
      << magic_number << " expected: " << _magic_number << endl;
    return -1;
  }
  // If the magic number does not match at all, something is really wrong
  else if (magic_number != _magic_number) {
    downloader_cat.error()
      << "DownloadDb::parse_header() - Invalid magic number: "
      << magic_number << " expected: " << _magic_number << endl;
    return -1;
  }

  int32_t num_multifiles = di.get_int32();
  if (downloader_cat.is_debug()) {
    downloader_cat.debug()
      << "Parsed number of multifiles: " << num_multifiles << endl;
  }

  // If we got all the way here, must be done
  return num_multifiles;
}



/**
 * Parses a file record (fr) header and returns the length of the next file
 * record
 */
int DownloadDb::Db::
parse_record_header(Datagram dg) {
  DatagramIterator di(dg);
  int32_t record_length = di.get_int32();
  if (downloader_cat.is_spam()) {
    downloader_cat.spam()
      << "Parsed record header length: " << record_length << endl;
  }

  // If we got all the way here, must be done
  return record_length;
}


/**
 * Parses a multifile record (mfr) and returns one
 */
PT(DownloadDb::MultifileRecord) DownloadDb::Db::
parse_mfr(Datagram dg) {

  PT(DownloadDb::MultifileRecord) mfr = new DownloadDb::MultifileRecord;

  DatagramIterator di(dg);
  mfr->_name = di.get_string32();
  mfr->_phase = di.get_float64();
  mfr->_size = di.get_int32();
  mfr->_status = di.get_int32();
  mfr->_num_files = di.get_int32();

  // At one time, we stored files in the database with a backslash separator.
  // Nowadays we use a forward slash, but we should make sure we properly
  // convert any old records we might read.
  mfr->_name = back_to_front_slash(mfr->_name);

  // Read the hash value
  mfr->_hash.read_datagram(di);

  if (downloader_cat.is_debug()) {
    downloader_cat.debug()
      << "Parsed multifile record: " << mfr->_name << " phase: " << mfr->_phase
      << " size: " << mfr->_size << " status: " << mfr->_status
      << " num_files: " << mfr->_num_files << endl;
  }

  // Return the new MultifileRecord
  return mfr;
}




/**
 * Parses a file record (fr) and returns one
 */
PT(DownloadDb::FileRecord) DownloadDb::Db::
parse_fr(Datagram dg) {

  PT(DownloadDb::FileRecord) fr = new DownloadDb::FileRecord;

  DatagramIterator di(dg);
  fr->_name = di.get_string32();

  // At one time, we stored files in the database with a backslash separator.
  // Nowadays we use a forward slash, but we should make sure we properly
  // convert any old records we might read.
  fr->_name = back_to_front_slash(fr->_name);

  if (downloader_cat.is_spam()) {
    downloader_cat.spam()
      << "Parsed file record: " << fr->_name << endl;
  }

  // Return the new MultifileRecord
  return fr;
}




/**
 *
 */
bool DownloadDb::Db::
read(StreamReader &sr, bool want_server_info) {
  // Read the header
  vector_uchar header = sr.extract_bytes(_header_length);
  if (header.size() != (size_t)_header_length) {
    downloader_cat.error() << "truncated db file" << endl;
    return false;
  }

  // Parse the header
  int num_multifiles = parse_header(Datagram(move(header)));
  if (num_multifiles < 0) {
    downloader_cat.error() << "invalid db header" << endl;
    return false;
  }

  // Now that we know how many multifiles this db has, we can iterate reading
  // them off one by one
  for (int i = 0; i < num_multifiles; i++) {
    // The multifile record header is just one int which represents the size
    // of the record
    int mfr_header_length = sizeof(int32_t);

    vector_uchar mfr_header = sr.extract_bytes(mfr_header_length);
    if (mfr_header.size() != (size_t)mfr_header_length) {
      downloader_cat.error() << "invalid mfr header" << endl;
      return false;
    }

    // Parse the header
    int mfr_length = parse_record_header(Datagram(move(mfr_header)));

    // Ok, now that we know the size of the mfr, read it in Make a buffer to
    // read the multifile record into do not count the header length twice
    int read_length = (mfr_length - mfr_header_length);
    vector_uchar mfr_record = sr.extract_bytes(read_length);
    if (mfr_record.size() != (size_t)read_length) {
      downloader_cat.error() << "invalid mfr record" << endl;
      return false;
    }

    // Parse the mfr
    PT(DownloadDb::MultifileRecord) mfr = parse_mfr(Datagram(move(mfr_record)));

    // Only read in the individual file info if you are the server
    if (want_server_info) {

      // Read off all the file records this multifile has
      for (int j = 0; j < mfr->_num_files; j++) {
        // The file record header is just one int which represents the size of
        // the record
        int fr_header_length = sizeof(int32_t);

        // Read the header
        vector_uchar fr_header = sr.extract_bytes(fr_header_length);
        if (fr_header.size() != (size_t)fr_header_length) {
          downloader_cat.error() << "invalid fr header" << endl;
          return false;
        }

        // Parse the header
        int fr_length = parse_record_header(Datagram(move(fr_header)));

        // Ok, now that we know the size of the mfr, read it in do not count
        // the header length twice
        int read_length = (fr_length - fr_header_length);

        vector_uchar fr_record = sr.extract_bytes(read_length);
        if (fr_record.size() != (size_t)read_length) {
          downloader_cat.error() << "invalid fr record" << endl;
          return false;
        }

        // Parse the file record
        PT(DownloadDb::FileRecord) fr = parse_fr(Datagram(move(fr_record)));

        // Add this file record to the current multifilerecord
        mfr->add_file_record(fr);
      }
    }

    // Add the current multifilerecord to our database
    add_multifile_record(mfr);
  }

  return true;
}



/**
 *
 */
bool DownloadDb::Db::
write(StreamWriter &sw, bool want_server_info) {
  // Declare these outside the loop so we do not keep creating and deleting
  // them
  PN_float64 phase;
  int32_t size;
  int32_t status;
  int32_t num_files;
  int32_t name_length;
  int32_t header_length;

  // Iterate over the multifiles writing them to the stream
  pvector< PT(MultifileRecord) >::const_iterator i = _mfile_records.begin();
  for(; i != _mfile_records.end(); ++i) {
    // Cache some properties so we do not have to keep asking for them
    phase = (*i)->_phase;
    size = (*i)->_size;
    status = (*i)->_status;
    num_files = (*i)->get_num_files();
    name_length = (*i)->_name.length();

    // Compute the length of this datagram
    header_length =
      sizeof(header_length) +  // Size of this header length
      sizeof(name_length) +    // Size of the size of the name string
      (*i)->_name.length() +      // Size of the name string
      sizeof(phase) + sizeof(size) +
      sizeof(status) + sizeof(num_files) +
      sizeof(uint32_t)*4; // Size of hash value

    // Add the length of this entire datagram
    sw.add_int32(header_length);

    // Add the length of the name
    sw.add_int32(name_length);
    // Add the name
    sw.append_data((*i)->_name);

    // Add all the properties
    sw.add_float64(phase);
    sw.add_int32(size);
    sw.add_int32(status);
    sw.add_int32(num_files);

    (*i)->_hash.write_stream(sw);

    // Only write out the file information if you are the server
    if (want_server_info) {
      // Now iterate over this multifile's files writing them to the stream
      // Iterate over the multifiles writing them to the stream
      pvector< PT(FileRecord) >::const_iterator j = (*i)->_file_records.begin();
      for(; j != (*i)->_file_records.end(); ++j) {
        name_length = (*j)->_name.length();

        // Compute the length of this datagram
        header_length =
          sizeof(header_length) +  // Size of this header length
          sizeof(name_length) +    // Size of the size of the name string
          (*j)->_name.length();    // Size of the name string

        // Add the length of this entire datagram
        sw.add_int32(header_length);

        // Add the length of the name
        sw.add_int32(name_length);
        // Add the name
        sw.append_data((*j)->_name);
      }
    }
  }

  return true;
}

/**
 * Writes the bogus header uncompressed with platform- independent byte
 * ordering.  This header will get overwritten with the real magic number as
 * the last step in the write
 */
bool DownloadDb::Db::
write_bogus_header(StreamWriter &sw) {
  // Write the db magic number
  sw.add_uint32(_bogus_magic_number);

  // Write the number of multifiles
  sw.add_int32(get_num_multifiles());

  return true;
}

/**
 * Writes the header uncompressed with platform- independent byte ordering
 */
bool DownloadDb::Db::
write_header(ostream &write_stream) {
  Datagram dg;

  // Write the db magic number
  dg.add_uint32(_magic_number);

  // Write the number of multifiles
  dg.add_int32(get_num_multifiles());

  // Seek back to the beginning of the write stream
  write_stream.seekp(0);
  // Overwrite the old bogus header with the real header
  write_stream.write((const char *)dg.get_data(), dg.get_length());
  return true;
}



// FileRecord methods


/**
 *
 */
DownloadDb::FileRecord::
FileRecord() {
  _name = "";
}


/**
 *
 */
DownloadDb::FileRecord::
FileRecord(string name) {
  _name = name;
}

/**
 *
 */
void DownloadDb::FileRecord::
write(ostream &out) const {
  out << " FileRecord: " << _name << endl;
}

/**
 * Appends a new version of the file onto the end of the list, or changes the
 * hash associated with a version previously added.
 *
 * Note: version numbers start at 1
 */
void DownloadDb::
add_version(const Filename &name, const HashVal &hash, int version) {
  nassertv(version >= 1);

  VectorHash &vhash = _versions[name];
  int size = vhash.size();

  // We should not skip over versions as we add them.
  nassertv(version <= size+1);

  if (version-1 < size) {
    // If you are overwriting an old hash value, just rewrite the value
    vhash[version-1] = hash;

  } else {
    // Otherwise, extend the vector.
    vhash.push_back(hash);
  }
}

/**
 * Inserts a new version 1 copy of the file, sliding all the other versions up
 * by one.
 */
void DownloadDb::
insert_new_version(const Filename &name, const HashVal &hash) {
  VectorHash &vhash = _versions[name];
  vhash.insert(vhash.begin(), hash);
}

/**
 * Returns true if the indicated file has version information, false
 * otherwise.  Some files recorded in the database may not bother to track
 * versions.
 */
bool DownloadDb::
has_version(const Filename &name) const {
  return (_versions.find(name) != _versions.end());
}

/**
 * Returns the number of versions stored for the indicated file.
 */
int DownloadDb::
get_num_versions(const Filename &name) const {
  VersionMap::const_iterator vmi = _versions.find(name);
  if (vmi == _versions.end()) {
    return 0;
  }

  return (int)(*vmi).second.size();
}

/**
 * Reduces the number of versions of a particular file stored in the ddb by
 * throwing away all versions higher than the indicated index.
 */
void DownloadDb::
set_num_versions(const Filename &name, int num_versions) {
  VersionMap::iterator vmi = _versions.find(name);
  if (vmi == _versions.end()) {
    nassertv(num_versions == 0);
    return;
  }

  VectorHash &vhash = (*vmi).second;

  nassertv(num_versions <= (int)vhash.size());
  vhash.erase(vhash.begin() + num_versions, vhash.end());
}

/**
 * Returns the version number of this particular file, determined by looking
 * up the hash generated from the file.  Returns -1 if the version number
 * cannot be determined.
 */
int DownloadDb::
get_version(const Filename &name, const HashVal &hash) const {
  VersionMap::const_iterator vmi = _versions.find(name);
  if (vmi == _versions.end()) {
    if (downloader_cat.is_debug()) {
      downloader_cat.debug()
        << "DownloadDb::get_version() - can't find: " << name << endl;
    }
    return -1;
  }
  const VectorHash &vhash = (*vmi).second;
  VectorHash::const_iterator i = find(vhash.begin(), vhash.end(), hash);
  if (i != vhash.end()) {
    return (i - vhash.begin() + 1);
  }
  if (downloader_cat.is_debug()) {
    downloader_cat.debug()
      << "DownloadDb::get_version() - can't find hash: " << hash << endl;
  }
  return -1;
}

/**
 * Returns the MD5 hash associated with the indicated version of the indicated
 * file.
 */
const HashVal &DownloadDb::
get_hash(const Filename &name, int version) const {
  static HashVal bogus_hash;

  VersionMap::const_iterator vmi = _versions.find(name);
  if (vmi == _versions.end()) {
    downloader_cat.error()
      << "DownloadDb::get_hash() - can't find: " << name << endl;
    return bogus_hash;
  }

  const VectorHash &vhash = (*vmi).second;
  if (version < 1 || version > (int)vhash.size()) {
    downloader_cat.error()
      << "DownloadDb::get_hash() - no version " << version
      << " for " << name << endl;
    return bogus_hash;
  }
  return vhash[version - 1];
}

/**
 *
 */
void DownloadDb::
write_version_map(StreamWriter &sw) {
  VersionMap::iterator vmi;
  VectorHash::iterator i;
  string name;

  sw.add_int32(_versions.size());
  for (vmi = _versions.begin(); vmi != _versions.end(); ++vmi) {
    name = (*vmi).first;
    if (downloader_cat.is_spam()) {
      downloader_cat.spam()
        << "DownloadDb::write_version_map() - writing file: "
        << name << " of length: " << name.length() << endl;
    }
    sw.add_int32(name.length());
    sw.append_data(name);
    sw.add_int32((*vmi).second.size());
    for (i = (*vmi).second.begin(); i != (*vmi).second.end(); ++i) {
      // *i will point to a HashVal
      (*i).write_stream(sw);
    }
  }
}

/**
 *
 */
bool DownloadDb::
read_version_map(StreamReader &sr) {
  int num_entries = sr.get_int32();
  if (sr.get_istream()->fail()) {
    return false;
  }

  for (int i = 0; i < num_entries; i++) {

    // Get the file name
    string name = sr.get_string32();
    if (downloader_cat.is_spam()) {
      downloader_cat.spam()
        << "DownloadDb::read_version_map() - name: " << name << endl;
    }

    // Get number of hash values for name
    int length = sr.get_int32();
    if (sr.get_istream()->fail()) {
      return false;
    }
    if (downloader_cat.is_spam()) {
      downloader_cat.spam()
        << "DownloadDb::read_version_map() - number of values: " << length
        << endl;
    }

    for (int j = 0; j < length; j++) {
      HashVal hash;
      hash.read_stream(sr);
      if (sr.get_istream()->fail()) {
        return false;
      }
      add_version(name, hash, j + 1);
    }
  }
  return true;
}

/**
 *
 */
void DownloadDb::
write_version_map(ostream &out) const {
  out << "Version Map: " << endl;
  VersionMap::const_iterator vmi;
  VectorHash::const_iterator i;
  for (vmi = _versions.begin(); vmi != _versions.end(); ++vmi) {
    out << "  " << (*vmi).first << endl;
    for (i = (*vmi).second.begin(); i != (*vmi).second.end(); ++i) {
      HashVal hash = *i;
      out << "    " << hash.as_dec() << endl;
    }
  }
  out << endl;
}
