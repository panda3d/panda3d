// Filename: download_utils.cxx
// Created by:  mike (18Jan99)
// 
////////////////////////////////////////////////////////////////////

// This file is compiled only if we have zlib installed.

#include "download_utils.h"
#include "config_downloader.h"
#include <zlib.h>

ulong
check_crc(Filename name) {
  ifstream read_stream;
  name.set_binary();
  if (!name.open_read(read_stream)) {
    downloader_cat.error()
      << "check_crc() - Failed to open input file: " << name << endl;
    return 0;
  }

  // Determine the length of the file and read it into the buffer
  read_stream.seekg(0, ios::end);
  int buffer_length = read_stream.tellg();
  char *buffer = new char[buffer_length];
  read_stream.seekg(0, ios::beg);
  read_stream.read(buffer, buffer_length);

  // Compute the crc
  ulong crc = crc32(0L, Z_NULL, 0);
  crc = crc32(crc, (uchar *)buffer, buffer_length);
  
  delete buffer; 

  return crc;
}

ulong
check_adler(Filename name) {
  ifstream read_stream;
  name.set_binary();
  if (!name.open_read(read_stream)) {
    downloader_cat.error()
      << "check_adler() - Failed to open input file: " << name << endl;
    return 0;
  }

  // Determine the length of the file and read it into the buffer
  read_stream.seekg(0, ios::end);
  int buffer_length = read_stream.tellg();
  char *buffer = new char[buffer_length];
  read_stream.seekg(0, ios::beg);
  read_stream.read(buffer, buffer_length);

  // Compute the adler checksum 
  ulong adler = adler32(0L, Z_NULL, 0);
  adler = adler32(adler, (uchar *)buffer, buffer_length);
  
  delete buffer; 

  return adler;
}

#include <md5.h>
#include <files.h>

#include <iostream>
#include <string>
#include <strstream>

USING_NAMESPACE(CryptoPP);
USING_NAMESPACE(std);

uint 
read32(istream& is) {
  unsigned int ret = 0x0;
  unsigned char b1, b2, b3, b4;
  is >> b1;
  is >> b2;
  is >> b3;
  is >> b4;
  ret = (b1 << 24) | (b2 << 16) | (b3 << 8) | b4;
  return ret;
}

void 
md5_a_file(const Filename &name, HashVal &ret) {
  ostrstream os;
  MD5 md5;

  string fs = name.get_fullpath();
  FileSource f(fs.c_str(), true, new HashFilter(md5, new FileSink(os)));

  istrstream is(os.str());

  ret[0] = read32(is);
  ret[1] = read32(is);
  ret[2] = read32(is);
  ret[3] = read32(is);
}

void 
md5_a_buffer(unsigned char* buf, unsigned long len, HashVal& ret) {
  MD5 md5;

  HashFilter hash(md5);
  hash.Put((byte*)buf, len);
  hash.Close();

  unsigned char* outb;
  unsigned long outl = hash.MaxRetrieveable();
  outb = new uchar[outl];
  hash.Get((byte*)outb, outl);
  ret[0] = (outb[0] << 24) | (outb[1] << 16) | (outb[2] << 8) | outb[3];
  ret[1] = (outb[4] << 24) | (outb[5] << 16) | (outb[6] << 8) | outb[7];
  ret[2] = (outb[8] << 24) | (outb[9] << 16) | (outb[10] << 8) | outb[11];
  ret[3] = (outb[12] << 24) | (outb[13] << 16) | (outb[14] << 8) | outb[15];
  delete outb;
}

