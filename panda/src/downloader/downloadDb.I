/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file downloadDb.I
 * @author shochet
 * @date 2000-09-08
 */

/**

 */
INLINE int DownloadDb::
get_client_num_multifiles() const {
  return _client_db.get_num_multifiles();
}

/**

 */
INLINE int DownloadDb::
get_server_num_multifiles() const {
  return _server_db.get_num_multifiles();
}

/**

 */
INLINE string DownloadDb::
get_client_multifile_name(int index) const {
  return _client_db.get_multifile_name(index);
}

/**

 */
INLINE string DownloadDb::
get_server_multifile_name(int index) const {
  return _server_db.get_multifile_name(index);
}


/**

 */
INLINE Phase DownloadDb::
get_client_multifile_phase(string mfname) const {
  return (_client_db.get_multifile_record_named(mfname))->_phase;
}

/**

 */
INLINE Phase DownloadDb::
get_server_multifile_phase(string mfname) const {
  return (_server_db.get_multifile_record_named(mfname))->_phase;
}



/**

 */
INLINE int DownloadDb::
get_client_multifile_size(string mfname) const {
  return (_client_db.get_multifile_record_named(mfname))->_size;
}

/**

 */
INLINE void DownloadDb::
set_client_multifile_size(string mfname, int size) {
  (_client_db.get_multifile_record_named(mfname))->_size = size;
  write_client_db(_client_db._filename);
}


/**

 */
INLINE int DownloadDb::
set_client_multifile_delta_size(string mfname, int size) {
  (_client_db.get_multifile_record_named(mfname))->_size += size;
  write_client_db(_client_db._filename);
  // Return the new total
  return (_client_db.get_multifile_record_named(mfname))->_size;
}



/**

 */
INLINE int DownloadDb::
get_server_multifile_size(string mfname) const {
  return (_server_db.get_multifile_record_named(mfname))->_size;
}


/**

 */
INLINE void DownloadDb::
set_server_multifile_size(string mfname, int size) {
  (_server_db.get_multifile_record_named(mfname))->_size = size;
}


/**

 */
INLINE void DownloadDb::
set_client_multifile_incomplete(string mfname) {
  (_client_db.get_multifile_record_named(mfname))->_status = Status_incomplete;
  write_client_db(_client_db._filename);
}

/**

 */
INLINE void DownloadDb::
set_client_multifile_complete(string mfname) {
  (_client_db.get_multifile_record_named(mfname))->_status = Status_complete;
  write_client_db(_client_db._filename);
}

/**

 */
INLINE void DownloadDb::
set_client_multifile_decompressed(string mfname) {
  (_client_db.get_multifile_record_named(mfname))->_status = Status_decompressed;
  write_client_db(_client_db._filename);
}

/**

 */
INLINE void DownloadDb::
set_client_multifile_extracted(string mfname) {
  (_client_db.get_multifile_record_named(mfname))->_status = Status_extracted;
  write_client_db(_client_db._filename);
}

/**

 */
INLINE int DownloadDb::
get_server_num_files(string mfname) const {
  return (_server_db.get_multifile_record_named(mfname))->get_num_files();
}

/**

 */
INLINE string DownloadDb::
get_server_file_name(string mfname, int index) const {
  return (_server_db.get_multifile_record_named(mfname))->get_file_name(index);
}
