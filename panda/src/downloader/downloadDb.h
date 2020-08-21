/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file downloadDb.h
 * @author shochet
 * @date 2000-09-06
 */

#ifndef DOWNLOADDB_H
#define DOWNLOADDB_H

#include "pandabase.h"
#include "pnotify.h"
#include "filename.h"
#include "multifile.h"
#include "datagram.h"
#include "datagramIterator.h"

#include "pvector.h"
#include "pointerTo.h"
#include "pmap.h"

#include "hashVal.h"

class StreamReader;
class StreamWriter;
typedef PN_stdfloat Phase;
class Ramfile;

/*
// Database Format
magic_number
number_of_multifiles
header_length multifile_name phase version size status num_files
  header_length file_name version hash
  header_length file_name version hash
header_length multifile_name phase version size status num_files
  header_length file_name version hash
  header_length file_name version hash
  ...
...


A Db is a Vector<MultifileRecord>
MultifileRecord is a Vector<FileRecord>
*/


/**
 * A listing of files within multifiles for management of client-side
 * synchronization with a server-provided set of files.
 *
 * This class manages one copy of the database for the client, representing
 * the files on the client system, and another copy for the server,
 * representing the files the server has available.
 */
class EXPCL_PANDA_DOWNLOADER DownloadDb {
PUBLISHED:
  // Status of a multifile is stored in this enum Note these values are in
  // increasing order of "doneness" So if you are decompressed, you are
  // complete If you are extracted, you are decompressed and complete
  enum Status {
    Status_incomplete = 0,
    Status_complete = 1,
    Status_decompressed = 2,
    Status_extracted = 3
  };

  DownloadDb();
  explicit DownloadDb(Ramfile &server_file, Filename &client_file);
  explicit DownloadDb(Filename &server_file, Filename &client_file);
  ~DownloadDb();

  void output(std::ostream &out) const;
  void write(std::ostream &out) const;
  void write_version_map(std::ostream &out) const;

  // Write a database file
  bool write_client_db(Filename &file);
  bool write_server_db(Filename &file);

  INLINE int get_client_num_multifiles() const;
  INLINE int get_server_num_multifiles() const;

  INLINE std::string get_client_multifile_name(int index) const;
  INLINE std::string get_server_multifile_name(int index) const;

  INLINE int get_client_multifile_size(std::string mfname) const;
  INLINE void set_client_multifile_size(std::string mfname, int size);
  INLINE int set_client_multifile_delta_size(std::string mfname, int size);
  INLINE int get_server_multifile_size(std::string mfname) const;
  INLINE void set_server_multifile_size(std::string mfname, int size);

  INLINE Phase get_client_multifile_phase(std::string mfname) const;
  INLINE Phase get_server_multifile_phase(std::string mfname) const;

  INLINE void set_client_multifile_incomplete(std::string mfname);
  INLINE void set_client_multifile_complete(std::string mfname);
  INLINE void set_client_multifile_decompressed(std::string mfname);
  INLINE void set_client_multifile_extracted(std::string mfname);

  INLINE int get_server_num_files(std::string mfname) const;
  INLINE std::string get_server_file_name(std::string mfname, int index) const;

  // Queries from the Launcher
  bool client_multifile_exists(std::string mfname) const;
  bool client_multifile_complete(std::string mfname) const;
  bool client_multifile_decompressed(std::string mfname) const;
  bool client_multifile_extracted(std::string mfname) const;

  // Ask what version (told with the hash) this multifile is
  HashVal get_client_multifile_hash(std::string mfname) const;
  void set_client_multifile_hash(std::string mfname, HashVal val);
  HashVal get_server_multifile_hash(std::string mfname) const;
  void set_server_multifile_hash(std::string mfname, HashVal val);

  // Operations on multifiles
  void delete_client_multifile(std::string mfname);
  void add_client_multifile(std::string server_mfname);
  void expand_client_multifile(std::string mfname);

  // Server side operations to create multifile records
  void create_new_server_db();
  void server_add_multifile(std::string mfname, Phase phase, int size, int status);
  void server_add_file(std::string mfname, std::string fname);

public:

  class EXPCL_PANDA_DOWNLOADER FileRecord : public ReferenceCount {
  public:
    FileRecord();
    FileRecord(std::string name);
    void write(std::ostream &out) const;
    std::string _name;
  };

  typedef pvector< PT(FileRecord) > FileRecords;

  class EXPCL_PANDA_DOWNLOADER MultifileRecord : public ReferenceCount {
  public:
    MultifileRecord();
    MultifileRecord(std::string name, Phase phase, int size, int status);
    void write(std::ostream &out) const;
    int get_num_files() const;
    std::string get_file_name(int index) const;
    bool file_exists(std::string fname) const;
    PT(FileRecord) get_file_record_named(std::string fname) const;
    void add_file_record(PT(FileRecord) fr);
    std::string _name;
    Phase _phase;
    int _size;
    int _status;
    HashVal _hash;
    int32_t _num_files;
    FileRecords _file_records;
  };

  typedef pvector< PT(MultifileRecord) > MultifileRecords;

  class EXPCL_PANDA_DOWNLOADER Db {
  public:
    Db();
    void write(std::ostream &out) const;
    int get_num_multifiles() const;
    std::string get_multifile_name(int index) const;
    bool multifile_exists(std::string mfname) const;
    PT(MultifileRecord) get_multifile_record_named(std::string mfname) const;
    void add_multifile_record(PT(MultifileRecord) mfr);
    int parse_header(Datagram dg);
    int parse_record_header(Datagram dg);
    PT(MultifileRecord) parse_mfr(Datagram dg);
    PT(FileRecord) parse_fr(Datagram dg);
    bool read(StreamReader &sr, bool want_server_info);
    bool write(StreamWriter &sw, bool want_server_info);
    Filename _filename;
    MultifileRecords _mfile_records;
    bool write_header(std::ostream &write_stream);
    bool write_bogus_header(StreamWriter &sw);
  private:
    int32_t _header_length;
  };

PUBLISHED:
  Db read_db(Filename &file, bool want_server_info);
  Db read_db(Ramfile &file, bool want_server_info);
  bool write_db(Filename &file, Db db, bool want_server_info);

public:
  // The download db stores two databases, one that represents the client's
  // state and one that represents the server state.
  Db _client_db;
  Db _server_db;

  // Magic number for knowing this is a download Db
  static uint32_t _magic_number;
  static uint32_t _bogus_magic_number;
  typedef pvector<HashVal> VectorHash;
  typedef pmap<Filename, VectorHash> VersionMap;

PUBLISHED:
  void add_version(const Filename &name, const HashVal &hash, int version);
  void insert_new_version(const Filename &name, const HashVal &hash);
  bool has_version(const Filename &name) const;
  int get_num_versions(const Filename &name) const;
  void set_num_versions(const Filename &name, int num_versions);

  int get_version(const Filename &name, const HashVal &hash) const;
  const HashVal &get_hash(const Filename &name, int version) const;

protected:
  void write_version_map(StreamWriter &sw);
  bool read_version_map(StreamReader &sr);
  VersionMap _versions;
};

INLINE std::ostream &operator << (std::ostream &out, const DownloadDb &dldb) {
  dldb.output(out);
  return out;
}


#include "downloadDb.I"

#endif
