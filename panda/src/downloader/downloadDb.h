// Filename: downloadDb.h
// Created by:  shochet (06Sep00)
//
////////////////////////////////////////////////////////////////////
//
#ifndef DOWNLOADDB_H
#define DOWNLOADDB_H
//
////////////////////////////////////////////////////////////////////
// Includes
////////////////////////////////////////////////////////////////////
#include <pandabase.h>
#include <notify.h>
#include <filename.h>
#include <multifile.h>

#include <vector>
#include <string>
#include <pointerTo.h>
#include <map>


/*
//////////////////////////////////////////////////
//  Database Format
//////////////////////////////////////////////////
magic_number
number_of_multifiles
header_length multifile_name phase version size status num_files
  header_length file_name version
  header_length file_name version
header_length multifile_name phase version size status num_files
  header_length file_name version
  header_length file_name version
  ...
...


A Db is a Vector<MultifileRecord>
MultifileRecord is a Vector<FileRecord>
*/

typedef int Version;
typedef int Phase;

class EXPCL_PANDAEXPRESS DownloadDb {
public:

  // Status of a multifile is stored in this enum
  // Note these values are in increasing order of "doneness"
  // So if you are decompressed, you are complete
  // If you are expanded, you are decompressed and complete
  enum Status {
    Status_incomplete = 0,
    Status_complete = 1,
    Status_decompressed = 2,
    Status_expanded = 3
  };

  DownloadDb(void);
  DownloadDb(Filename &server_file, Filename &client_file);
  ~DownloadDb(void);

  void output(ostream &out) const;

  // Write a database file
  bool write_client_db(Filename &file);
  bool write_server_db(Filename &file);
  
  INLINE int get_client_num_multifiles(void) const;
  INLINE int get_server_num_multifiles(void) const;

  INLINE string get_client_multifile_name(int index) const;
  INLINE string get_server_multifile_name(int index) const;

  INLINE Version get_client_multifile_version(string mfname) const;
  INLINE void set_client_multifile_version(string mfname, Version version);
  INLINE Version get_server_multifile_version(string mfname) const;

  INLINE int get_client_multifile_size(string mfname) const;
  INLINE void set_client_multifile_size(string mfname, int size);
  INLINE void set_client_multifile_delta_size(string mfname, int size);
  INLINE int get_server_multifile_size(string mfname) const;

  INLINE int get_client_multifile_phase(string mfname) const;
  INLINE int get_server_multifile_phase(string mfname) const;

  INLINE void set_client_multifile_complete(string mfname);
  INLINE void set_client_multifile_decompressed(string mfname);
  INLINE void set_client_multifile_expanded(string mfname);

  INLINE int get_client_num_files(string mfname) const;
  INLINE int get_server_num_files(string mfname) const;

  INLINE string get_client_file_name(string mfname, int index) const;
  INLINE string get_server_file_name(string mfname, int index) const;

  INLINE Version get_client_file_version(string mfname, string fname) const;
  INLINE Version get_server_file_version(string mfname, string fname) const;
  INLINE void set_client_file_version(string mfname, string fname, Version version);

  // Check client db against server db
  bool client_db_current_version(void) const;

  // Queries from the Launcher
  bool client_multifile_exists(string mfname) const;
  bool client_multifile_complete(string mfname) const;
  bool client_multifile_decompressed(string mfname) const;
  bool client_multifile_expanded(string mfname) const;
  bool client_multifile_version_correct(string mfname) const;
  bool client_file_version_correct(string mfname, string filename) const;
  bool client_file_crc_correct(string mfname, string filename) const;

  // Operations on multifiles
  void delete_client_multifile(string mfname);
  void add_client_multifile(string server_mfname);
  void expand_client_multifile(string mfname);

  // Server side operations to create multifile records
  void create_new_server_db();
  void server_add_multifile(string mfname, Phase phase, Version version, int size, int status);
  void server_add_file(string mfname, string fname, Version version);

public:

  class EXPCL_PANDAEXPRESS FileRecord : public ReferenceCount {
  public:
    FileRecord(void);
    FileRecord(string name, Version version);
    void output(ostream &out) const;
    string _name;
    Version _version;
  };

  typedef vector<PT(FileRecord)> FileRecords;

  class EXPCL_PANDAEXPRESS MultifileRecord : public ReferenceCount {
  public:
    MultifileRecord(void);
    MultifileRecord(string name, Phase phase, Version version, int size, int status);
    void output(ostream &out) const;
    int get_num_files(void) const;
    string get_file_name(int index) const;
    bool file_exists(string fname) const;
    PT(FileRecord) get_file_record_named(string fname) const;
    void add_file_record(PT(FileRecord) fr);
    string _name;
    Phase _phase;
    Version _version;
    int _size;
    int _status;
    PN_int32 _num_files;
    FileRecords _file_records;
  };

  typedef vector<PT(MultifileRecord)> MultifileRecords;

  class EXPCL_PANDAEXPRESS Db {
  public:
    Db(void);
    void output(ostream &out) const;
    int get_num_multifiles(void) const;
    string get_multifile_name(int index) const;
    bool multifile_exists(string mfname) const;
    PT(MultifileRecord) get_multifile_record_named(string mfname) const;
    void add_multifile_record(PT(MultifileRecord) mfr);
    int parse_header(uchar *start, int size);
    int parse_record_header(uchar *start, int size);
    PT(MultifileRecord) parse_mfr(uchar *start, int size);
    PT(FileRecord) parse_fr(uchar *start, int size);
    bool read(ifstream &read_stream);
    bool write(ofstream &write_stream);
    Version _version;
    Filename _filename;
    MultifileRecords _mfile_records;
  private:
    PN_int32 _header_length;

    // Datagram used for reading and writing to disk
    Datagram _datagram;

    bool write_header(ofstream &write_stream);
  };

  Db read_db(Filename &file);
  bool write_db(Filename &file, Db db);

  // The doenload db stores two databases, one that represents the client's state
  // and one that represents the server state
  Db _client_db;
  Db _server_db;
  
public:
  // Magic number for knowing this is a download Db
  static PN_uint32 _magic_number;  

  typedef vector<unsigned long> vector_ulong;
  typedef map<int, vector_ulong> VersionMap;
  void add_version(const Filename &name, ulong hash, int version);
  void add_version(int name, ulong hash, int version);
  int get_version(const Filename &name, ulong hash);

protected:
  void write_version_map(ofstream &write_stream);
  bool read_version_map(ifstream &read_stream);
  VersionMap _versions;
  Datagram _master_datagram;
};

INLINE ostream &operator << (ostream &out, const DownloadDb &dldb) {
  dldb.output(out);
  return out;
}


#include "downloadDb.I"

#endif
