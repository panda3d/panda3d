// Filename: ipc_file.h
// Created by:  jason (05Jun00)
//

#ifndef __IPC_FILE_H__
#define __IPC_FILE_H__

#include <pandabase.h>
#include <datagramGenerator.h>
#include <datagramSink.h>
#include "ipc_traits.h"

class base_file {
public:
  enum file_mode {FILE_READ, FILE_WRITE, FILE_BOTH, FILE_APPEND};

  typedef ipc_traits traits;
  typedef traits::file_class file_class;
  
  static base_file* const Null;

  INLINE base_file(const string& file = "") : _file(traits::make_file(file)) {}
  virtual ~base_file(void) { delete _file; };
  INLINE void setFile(const string& file) {_file->setFile(file);}
  INLINE bool open(file_mode mode) {return _file->open(mode);}
  INLINE void close(void) {_file->close();}
  INLINE bool empty(void) { return _file->empty();}
  INLINE int readin(string& buffer, int numBytes) {return _file->readin(buffer, numBytes);}
  INLINE int writeout(const string& buffer) {return _file->writeout(buffer);}
private:
  file_class* _file;
};

typedef base_file file;
typedef file::file_mode file_mode;

#include "ipc_file.I"

#endif /* __IPC_FILE_H__ */

