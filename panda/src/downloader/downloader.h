// Filename: downloader.h
// Created by:  mike (09Jan97)
//
////////////////////////////////////////////////////////////////////
//
#ifndef DOWNLOADER_H
#define DOWNLOADER_H
//
////////////////////////////////////////////////////////////////////
// Includes
////////////////////////////////////////////////////////////////////
#include <pandabase.h>
#include <notify.h>
#include <filename.h>
#include <tokenBoard.h>
#include <buffer.h>
#include "asyncUtility.h"

#if defined(WIN32_VC)
  #include <winsock2.h>
#else
  #include <sys/types.h>
  #include <sys/socket.h>
  #include <resolv.h>
#endif

class DownloaderToken;

////////////////////////////////////////////////////////////////////
//       Class : Downloader
// Description :
////////////////////////////////////////////////////////////////////
class EXPCL_PANDAEXPRESS Downloader : public AsyncUtility {
public:
  Downloader(void);
  Downloader(PT(Buffer) buffer);
  virtual ~Downloader(void);

  bool connect_to_server(const string &name, uint port=80);
  void disconnect_from_server(void);

  int request_download(const string &file_name, const Filename &file_dest,
			const string &event_name);
  int request_download(const string &file_name, const Filename &file_dest,
			const string &event_name, int first_byte,
			int last_byte, int total_bytes, 
			bool partial_content = true);

  INLINE void set_bandwidth(float bytes);
  INLINE float get_bandwidth(void) const;
  INLINE void enable_download(bool val);
  INLINE bool is_download_enabled(void) const;

private:
  class DownloadStatus {
  public:
    DownloadStatus(char *buffer, const string &event_name, int first_byte,
			int last_byte, int total_bytes, bool partial_content,
			uint id);
    void reset(void);

  public:
    bool _first_line_complete;
    bool _header_is_complete;
    bool _header_is_valid;
    char *_start;
    char *_next_in;
    int _bytes_in_buffer;
    string _event_name;
    int _total_bytes_written;
    int _first_byte;
    int _last_byte;
    int _total_bytes;
    bool _partial_content;
    uint _id;

  private:
    char *_buffer;
  };

  void init(PT(Buffer) buffer);
  bool download(const string &file_name, Filename file_dest, 
			const string &event_name, int first_byte,
			int last_byte, int total_bytes, bool partial_content,
			uint id);
  virtual bool process_request(void);
  bool parse_header(DownloadStatus &status);
  bool write_to_disk(DownloadStatus &status);
  bool connect_to_server(void);
  bool safe_send(int socket, const char *data, int length, long timeout);
  int safe_receive(int socket, char *data, int length, long timeout);

  typedef TokenBoard<DownloaderToken> DownloaderTokenBoard;
  DownloaderTokenBoard *_token_board;

  bool _connected;

#ifdef HAVE_IPC
  mutex _bandwidth_frequency_lock;
#endif

  int _socket;
  PT(Buffer) _buffer;
  float _bandwidth;
  bool _download_enabled;
  ofstream _dest_stream;

  string _server_name;
  struct sockaddr_in _sin;
};

#include "downloader.I"

#endif
