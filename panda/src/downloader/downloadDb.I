// Filename: downloadDb.I
// Created by:  shochet (08Sep00)
//
////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////
//     Function: DownloadDb::
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE int DownloadDb::
get_client_num_multifiles(void) const {
  return _client_db.get_num_multifiles();
}

////////////////////////////////////////////////////////////////////
//     Function: DownloadDb::
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE int DownloadDb::
get_server_num_multifiles(void) const {
  return _server_db.get_num_multifiles();
}

////////////////////////////////////////////////////////////////////
//     Function: DownloadDb::
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE string DownloadDb::
get_client_multifile_name(int index) const {
  return _client_db.get_multifile_name(index);
}

////////////////////////////////////////////////////////////////////
//     Function: DownloadDb::
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE string DownloadDb::
get_server_multifile_name(int index) const {
  return _server_db.get_multifile_name(index);
}


////////////////////////////////////////////////////////////////////
//     Function: DownloadDb::
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE Version DownloadDb::
get_client_multifile_phase(string mfname) const {
  return (_client_db.get_multifile_record_named(mfname))->_phase;
}

////////////////////////////////////////////////////////////////////
//     Function: DownloadDb::
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE Version DownloadDb::
get_server_multifile_phase(string mfname) const {
  return (_server_db.get_multifile_record_named(mfname))->_phase;
}



////////////////////////////////////////////////////////////////////
//     Function: DownloadDb::
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE int DownloadDb::
get_client_multifile_size(string mfname) const {
  return (_client_db.get_multifile_record_named(mfname))->_size;
}

////////////////////////////////////////////////////////////////////
//     Function: DownloadDb::
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE void DownloadDb::
set_client_multifile_size(string mfname, int size) {
  (_client_db.get_multifile_record_named(mfname))->_size = size;
  write_db(_client_db._filename, _client_db);
}


////////////////////////////////////////////////////////////////////
//     Function: DownloadDb::
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE int DownloadDb::
set_client_multifile_delta_size(string mfname, int size) {
  (_client_db.get_multifile_record_named(mfname))->_size += size;
  write_db(_client_db._filename, _client_db);
  // Return the new total
  return (_client_db.get_multifile_record_named(mfname))->_size;
}



////////////////////////////////////////////////////////////////////
//     Function: DownloadDb::
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE int DownloadDb::
get_server_multifile_size(string mfname) const {
  return (_server_db.get_multifile_record_named(mfname))->_size;
}


////////////////////////////////////////////////////////////////////
//     Function: DownloadDb::
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE void DownloadDb::
set_server_multifile_size(string mfname, int size) {
  (_server_db.get_multifile_record_named(mfname))->_size = size;
}


////////////////////////////////////////////////////////////////////
//     Function: DownloadDb::
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE void DownloadDb::
set_client_multifile_incomplete(string mfname) {
  (_client_db.get_multifile_record_named(mfname))->_status = Status_incomplete;
  write_db(_client_db._filename, _client_db);
}

////////////////////////////////////////////////////////////////////
//     Function: DownloadDb::
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE void DownloadDb::
set_client_multifile_complete(string mfname) {
  (_client_db.get_multifile_record_named(mfname))->_status = Status_complete;
  write_db(_client_db._filename, _client_db);
}

////////////////////////////////////////////////////////////////////
//     Function: DownloadDb::
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE void DownloadDb::
set_client_multifile_decompressed(string mfname) {
  (_client_db.get_multifile_record_named(mfname))->_status = Status_decompressed;
  write_db(_client_db._filename, _client_db);
}

////////////////////////////////////////////////////////////////////
//     Function: DownloadDb::
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE void DownloadDb::
set_client_multifile_extracted(string mfname) {
  (_client_db.get_multifile_record_named(mfname))->_status = Status_extracted;
  write_db(_client_db._filename, _client_db);
}


////////////////////////////////////////////////////////////////////
//     Function: DownloadDb::
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE int DownloadDb::
get_client_num_files(string mfname) const {
  return _client_db.get_multifile_record_named(mfname)->get_num_files();
}
////////////////////////////////////////////////////////////////////
//     Function: DownloadDb::
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE int DownloadDb::
get_server_num_files(string mfname) const {
  return (_server_db.get_multifile_record_named(mfname))->get_num_files();
}

////////////////////////////////////////////////////////////////////
//     Function: DownloadDb::
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE string DownloadDb::
get_client_file_name(string mfname, int index) const {
  return (_client_db.get_multifile_record_named(mfname))->get_file_name(index);
}

////////////////////////////////////////////////////////////////////
//     Function: DownloadDb::
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE string DownloadDb::
get_server_file_name(string mfname, int index) const {
  return (_server_db.get_multifile_record_named(mfname))->get_file_name(index);  
}
