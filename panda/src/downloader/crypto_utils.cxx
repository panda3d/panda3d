// Filename: crypto_utils.cxx
// Created by:  drose (07Nov00)
//
////////////////////////////////////////////////////////////////////

// This file is compiled only if we have crypto++ installed.

#include "crypto_utils.h"

#include <md5.h>
#include <files.h>

#include <string>

USING_NAMESPACE(CryptoPP);
USING_NAMESPACE(std);

static uint 
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
  ostringstream os;
  MD5 md5;

  string fs = name.to_os_specific();
  FileSource f(fs.c_str(), true, new HashFilter(md5, new FileSink(os)));

  istringstream is(os.str());

  ret.hv[0] = read32(is);
  ret.hv[1] = read32(is);
  ret.hv[2] = read32(is);
  ret.hv[3] = read32(is);
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
  ret.hv[0] = (outb[0] << 24) | (outb[1] << 16) | (outb[2] << 8) | outb[3];
  ret.hv[1] = (outb[4] << 24) | (outb[5] << 16) | (outb[6] << 8) | outb[7];
  ret.hv[2] = (outb[8] << 24) | (outb[9] << 16) | (outb[10] << 8) | outb[11];
  ret.hv[3] = (outb[12] << 24) | (outb[13] << 16) | (outb[14] << 8) | outb[15];
  delete[] outb;
}

