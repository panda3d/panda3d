// Filename: pnmReaderTypes.h
// Created by:  drose (14Jun00)
// 
////////////////////////////////////////////////////////////////////

#ifndef PNMREADERTYPES_H
#define PNMREADERTYPES_H

#include <pandabase.h>

#include "pnmReader.h"

class EXPCL_PANDA PNMReaderPNM : public PNMReader {
public:
  PNMReaderPNM(FILE *file, int already_read_magic);

  virtual FileType Type() const {
    return PNMImageHeader::PNM;
  }

  virtual bool ReadRow(xel *row_data, xelval *alpha_data);

  int ftype;
};

class EXPCL_PANDA PNMReaderSGI : public PNMReader {
public:
  PNMReaderSGI(FILE *file, int already_read_magic);

  virtual FileType Type() const {
    return PNMImageHeader::SGI;
  }

  virtual bool ReadRow(xel *row_data, xelval *alpha_data);
  ~PNMReaderSGI();

  typedef struct {
    long start;     /* offset in file */
    long length;    /* length of compressed scanline */
  } TabEntry;
 
  TabEntry *table;
  long table_start;
  int current_row;
  int zsize, bpc;
};

class EXPCL_PANDA PNMReaderAlias : public PNMReader {
public:
  PNMReaderAlias(FILE *file, int already_read_magic);

  virtual FileType Type() const {
    return PNMImageHeader::Alias;
  }

  virtual bool ReadRow(xel *row_data, xelval *alpha_data);
};

class EXPCL_PANDA PNMReaderRadiance : public PNMReader {
public:
  PNMReaderRadiance(FILE *file, int already_read_magic);

  virtual FileType Type() const {
    return PNMImageHeader::Radiance;
  }

  virtual bool ReadRow(xel *row_data, xelval *alpha_data);
};

#ifndef WIN32_VC
class EXPCL_PANDA PNMReaderTIFF : public PNMReader {
public:
  PNMReaderTIFF(FILE *file, int already_read_magic);

  virtual FileType Type() const {
    return PNMImageHeader::TIFF;
  }

  virtual bool ReadRow(xel *row_data, xelval *alpha_data);
  ~PNMReaderTIFF();

  unsigned short photomet;
  unsigned short bps, spp;
  xel colormap[1024];  // == MAXCOLORS in pnm-image-tiff.cxx

  int current_row;
  struct tiff *tif;
};
#endif

class EXPCL_PANDA PNMReaderIMG : public PNMReader {
public:
  PNMReaderIMG(FILE *file, int already_read_magic);

  virtual FileType Type() const {
    return PNMImageHeader::IMG;
  }

  virtual bool ReadRow(xel *row_data, xelval *alpha_data);
};

class EXPCL_PANDA PNMReaderSoftImage : public PNMReader {
public:
  PNMReaderSoftImage(FILE *file, int already_read_magic);

  virtual FileType Type() const {
    return PNMImageHeader::SoftImage;
  }

  virtual bool ReadRow(xel *row_data, xelval *alpha_data);

  enum { unknown, rgb, rgba, rgb_a } soft_color;
  int rgb_ctype, alpha_ctype;
};

class EXPCL_PANDA PNMReaderYUV : public PNMReader {
public:
  PNMReaderYUV(FILE *file, int already_read_magic);

  virtual FileType Type() const {
    return PNMImageHeader::YUV;
  }

  virtual bool ReadRow(xel *row_data, xelval *alpha_data);
  ~PNMReaderYUV();

  long *yuvbuf;
};

class EXPCL_PANDA PNMReaderBMP : public PNMReader {
public:
  PNMReaderBMP(FILE *file, int already_read_magic);

  virtual FileType Type() const {
    return PNMImageHeader::BMP;
  }

  virtual int ReadData(xel *array, xelval *alpha);

  virtual bool SupportsStreaming() const {
    return false;
  }

  unsigned long	pos;
 
  unsigned long offBits;
 
  unsigned short  cBitCount;
  int             indexed;
  int             classv;

  pixval R[256];	/* reds */
  pixval G[256];	/* greens */
  pixval B[256];	/* blues */
};

#endif


