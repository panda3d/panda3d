// Filename: pnmWriterTypes.h
// Created by:  drose (14Jun00)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://www.panda3d.org/license.txt .
//
// To contact the maintainers of this program write to
// panda3d@yahoogroups.com .
//
////////////////////////////////////////////////////////////////////

#ifndef PNMWRITERTYPES_H
#define PNMWRITERTYPES_H

#include <pandabase.h>

#include "pnmWriter.h"

class EXPCL_PANDA PNMWriterPNM : public PNMWriter {
public:
  PNMWriterPNM(FILE *file) : PNMWriter(file) {}

  virtual FileType Type() const {
    return PNMImageHeader::PNM;
  }

  virtual bool WriteHeader();
  virtual bool WriteRow(xel *row_data, xelval *alpha_data);

  int pnm_format;
};

class EXPCL_PANDA PNMWriterSGI : public PNMWriter {
public:
  PNMWriterSGI(FILE *file) : PNMWriter(file) {}
  ~PNMWriterSGI();

  virtual FileType Type() const {
    return PNMImageHeader::SGI;
  }

  virtual bool WriteHeader();
  virtual bool WriteRow(xel *row_data, xelval *alpha_data);


protected:

  typedef struct {
    long start;     /* offset in file */
    long length;    /* length of compressed scanline */
  } TabEntry;

  typedef short ScanElem;
  typedef struct {
    ScanElem *  data;
    long        length;
  } ScanLine;

  TabEntry &Table(int chan) {
    return table[chan * rows + current_row];
  }

  void write_header(char *imagename);
  void WriteTable();
  void WriteChannels(ScanLine channel[], void (*put)(FILE *, short));
  void BuildScanline(ScanLine output[], xel *row_data, xelval *alpha_data);
  ScanElem *Compress(ScanElem *temp, ScanLine &output);
  int RLECompress(ScanElem *inbuf, int size);

  TabEntry *table;
  long table_start;
  int current_row;
  int zsize, bpc;
  int channels, dimensions;
  int new_maxval;

  ScanElem *rletemp;
};

class EXPCL_PANDA PNMWriterAlias : public PNMWriter {
public:
  PNMWriterAlias(FILE *file) : PNMWriter(file) {}

  virtual FileType Type() const {
    return PNMImageHeader::Alias;
  }

  virtual bool WriteHeader();
  virtual bool WriteRow(xel *row_data, xelval *alpha_data);
};

class EXPCL_PANDA PNMWriterRadiance : public PNMWriter {
public:
  PNMWriterRadiance(FILE *file) : PNMWriter(file) {}

  virtual FileType Type() const {
    return PNMImageHeader::Radiance;
  }

  virtual bool WriteHeader();
  virtual bool WriteRow(xel *row_data, xelval *alpha_data);
};

#ifndef WIN32_VC
class EXPCL_PANDA PNMWriterTIFF : public PNMWriter {
public:
  PNMWriterTIFF(FILE *file) : PNMWriter(file) {}

  virtual FileType Type() const {
    return PNMImageHeader::TIFF;
  }

  virtual int WriteData(xel *array, xelval *alpha);
  virtual bool SupportsStreaming() const {
    return false;
  }
};
#endif

class EXPCL_PANDA PNMWriterIMG : public PNMWriter {
public:
  PNMWriterIMG(FILE *file) : PNMWriter(file) {}

  virtual FileType Type() const {
    return PNMImageHeader::IMG;
  }

  virtual bool WriteHeader();
  virtual bool WriteRow(xel *row_data, xelval *alpha_data);
};

class EXPCL_PANDA PNMWriterSoftImage : public PNMWriter {
public:
  PNMWriterSoftImage(FILE *file) : PNMWriter(file) {}

  virtual FileType Type() const {
    return PNMImageHeader::SoftImage;
  }

  virtual bool WriteHeader();
  virtual bool WriteRow(xel *row_data, xelval *alpha_data);
};

class EXPCL_PANDA PNMWriterYUV : public PNMWriter {
public:
  PNMWriterYUV(FILE *file) : PNMWriter(file) {
    yuvbuf = NULL;
  }

  virtual FileType Type() const {
    return PNMImageHeader::YUV;
  }

  virtual bool WriteHeader();
  virtual bool WriteRow(xel *row_data, xelval *alpha_data);
  ~PNMWriterYUV();

  unsigned char *yuvbuf;
};

class EXPCL_PANDA PNMWriterBMP : public PNMWriter {
public:
  PNMWriterBMP(FILE *file) : PNMWriter(file) {}

  virtual FileType Type() const {
    return PNMImageHeader::BMP;
  }

  virtual int WriteData(xel *array, xelval *alpha);
  virtual bool SupportsStreaming() const {
    return false;
  }
};

#endif


