// pnm-image-writesgi.cc
//
// PNMImage::WriteSGI() and supporting functions.



// Much code in this file originally came from from Netpbm,
// specifically pnmtosgi.c.  It has since been fairly heavily
// modified.

/* pnmtosgi.c - convert portable anymap to SGI image
**
** Copyright (C) 1994 by Ingo Wilken (Ingo.Wilken@informatik.uni-oldenburg.de)
**
** Based on the SGI image description v0.9 by Paul Haeberli (paul@sgi.comp)
** Available via ftp from sgi.com:graphics/SGIIMAGESPEC
**
** Permission to use, copy, modify, and distribute this software and its
** documentation for any purpose and without fee is hereby granted, provided
** that the above copyright notice appear in all copies and that both that
** copyright notice and this permission notice appear in supporting
** documentation.  This software is provided "as is" without express or
** implied warranty.
**
** 29Jan94: first version
*/

#include "pnmImage.h"
#include "pnmWriter.h"
#include "pnmWriterTypes.h"
#include "../pnm/sgi.h"



#define WORSTCOMPR(x)   (2*(x) + 2)


#define MAXVAL_BYTE     255
#define MAXVAL_WORD     65535

static char storage = STORAGE_RLE;

inline void 
put_byte(FILE *out_file, unsigned char b) {
  putc(b, out_file);
}

static void
put_big_short(FILE *out_file, short s) {
    if ( pm_writebigshort( out_file, s ) == -1 )
        pm_error( "write error" );
}


static void
put_big_long(FILE *out_file, long l) {
    if ( pm_writebiglong( out_file, l ) == -1 )
        pm_error( "write error" );
}


static void
put_short_as_byte(FILE *out_file, short s) {
    put_byte(out_file, (unsigned char)s);
}

PNMWriterSGI::
~PNMWriterSGI() {
  if (table!=NULL) {
    // Rewrite the table with the correct values in it.
    fseek(file, table_start, SEEK_SET);
    WriteTable();
    delete[] table;
  }
}

bool PNMWriterSGI::
WriteHeader() {
  table = NULL;

  switch (color_type) {
  case PNMImage::Grayscale:
    dimensions = 2; channels = 1;
    break;

  case PNMImage::TwoChannel:
    dimensions = 2; channels = 2;
    break;

  case PNMImage::Color:
    dimensions = 3; channels = 3;
    break;

  case PNMImage::FourChannel:
    dimensions = 3; channels = 4;
    break;
  }

  // For some reason, we have problems with SGI image files whose pixmax value
  // is not 255 or 65535.  So, we'll round up when writing.
  if( maxval <= MAXVAL_BYTE ) {
    bpc = 1;
    new_maxval = MAXVAL_BYTE;
  } else if( maxval <= MAXVAL_WORD ) {
    bpc = 2;
    new_maxval = MAXVAL_WORD;
  } else {
    return false;
  }
  
  if( storage != STORAGE_VERBATIM ) {
    table = new TabEntry[channels * rows];
    memset(table, 0, channels * rows * sizeof(TabEntry));
  }

  write_header("no name");
  if (table!=NULL) {
    table_start = ftell(file);

    // The first time we write the table, it has zeroes.  We'll correct
    // this later.
    WriteTable();
  }

  current_row = rows-1;
  return true;
}


bool PNMWriterSGI::
WriteRow(xel *row_data, xelval *alpha_data) {
  ScanLine channel[4];

  BuildScanline(channel, row_data, alpha_data);

  if( bpc == 1 )
    WriteChannels(channel, put_short_as_byte);
  else
    WriteChannels(channel, put_big_short);
  
  for (int i = 0; i < channels; i++) {
    delete[] channel[i].data;
  }

  current_row--;
  return true;
}


void PNMWriterSGI::
write_header(char *imagename) {
    int i;

    put_big_short(file, SGI_MAGIC);
    put_byte(file, storage);
    put_byte(file, (char)bpc);
    put_big_short(file, dimensions);
    put_big_short(file, cols);
    put_big_short(file, rows);
    put_big_short(file, channels);
    put_big_long(file, 0);                /* PIXMIN */
    put_big_long(file, maxval);           /* PIXMAX */
    for( i = 0; i < 4; i++ )
        put_byte(file, 0);
    for( i = 0; i < 79 && imagename[i] != '\0'; i++ )
        put_byte(file, imagename[i]);
    for(; i < 80; i++ )
        put_byte(file, 0);
    put_big_long(file, CMAP_NORMAL);
    for( i = 0; i < 404; i++ )
        put_byte(file, 0);
}


void PNMWriterSGI::
WriteTable() {
    int i;
    int tabsize = rows*channels;

    for( i = 0; i < tabsize; i++ ) {
        put_big_long(file, table[i].start);
    }
    for( i = 0; i < tabsize; i++ )
        put_big_long(file, table[i].length);
}


void PNMWriterSGI::
WriteChannels(ScanLine channel[], void (*put)(FILE *, short)) {
  int i, col;
  
  for( i = 0; i < channels; i++ ) {
    Table(i).start = ftell(file);
    Table(i).length = channel[i].length * bpc;

    for( col = 0; col < channel[i].length; col++ ) {
      (*put)(file, channel[i].data[col]);
    }
  }
}


void PNMWriterSGI::
BuildScanline(ScanLine output[], xel *row_data, xelval *alpha_data) {
  int col;
  ScanElem *temp;
  
  if( storage != STORAGE_VERBATIM ) {
    rletemp = (ScanElem *)alloca(WORSTCOMPR(cols) * sizeof(ScanElem));
  }
  temp = new ScanElem[cols];
  
  if( channels <= 2 ) {
    for( col = 0; col < cols; col++ )
      temp[col] = (ScanElem)
	(new_maxval * PPM_GETB(row_data[col]) / maxval);
    temp = Compress(temp, output[0]);

    if (channels == 2) {
      for( col = 0; col < cols; col++ )
	temp[col] = (ScanElem)
	  (new_maxval * alpha_data[col] / maxval);
      temp = Compress(temp, output[1]);
    }

  } else {
    for( col = 0; col < cols; col++ )
      temp[col] = (ScanElem)
	(new_maxval * PPM_GETR(row_data[col]) / maxval);
    temp = Compress(temp, output[0]);
    for( col = 0; col < cols; col++ )
      temp[col] = (ScanElem)
	(new_maxval * PPM_GETG(row_data[col]) / maxval);
    temp = Compress(temp, output[1]);
    for( col = 0; col < cols; col++ )
      temp[col] = (ScanElem)
	(new_maxval * PPM_GETB(row_data[col]) / maxval);
    temp = Compress(temp, output[2]);
    if (channels == 4) {
      for( col = 0; col < cols; col++ )
	temp[col] = (ScanElem)
	  (new_maxval * alpha_data[col] / maxval);
      temp = Compress(temp, output[3]);
    }
  }

  delete[] temp;
}


PNMWriterSGI::ScanElem *PNMWriterSGI::
Compress(ScanElem *temp, ScanLine &output) {
    int len;

    switch( storage ) {
        case STORAGE_VERBATIM:
            output.length = cols;
            output.data = temp;
            temp = new ScanElem[cols];
            break;
        case STORAGE_RLE:
            len = RLECompress(temp, cols);    /* writes result into rletemp */
            output.length = len;
            output.data = new ScanElem[len];
	    memcpy(output.data, rletemp, len * sizeof(ScanElem));
            break;
        default:
            pm_error("unknown storage type - can\'t happen");
    }
    return temp;
}


/*
slightly modified RLE algorithm from ppmtoilbm.c
written by Robert A. Knop (rknop@mop.caltech.edu)
*/
int PNMWriterSGI::
RLECompress(ScanElem *inbuf, int size) {
    int in, out, hold, count;
    ScanElem *outbuf = rletemp;

    in=out=0;
    while( in<size ) {
        if( (in<size-1) && (inbuf[in]==inbuf[in+1]) ) {     /*Begin replicate run*/
            for( count=0,hold=in; in<size && inbuf[in]==inbuf[hold] && count<127; in++,count++)
                ;
            outbuf[out++]=(ScanElem)(count);
            outbuf[out++]=inbuf[hold];
        }
        else {  /*Do a literal run*/
            hold=out; out++; count=0;
            while( ((in>=size-2)&&(in<size)) || ((in<size-2) && ((inbuf[in]!=inbuf[in+1])||(inbuf[in]!=inbuf[in+2]))) ) {
                outbuf[out++]=inbuf[in++];
                if( ++count>=127 )
                    break;
            }
            outbuf[hold]=(ScanElem)(count | 0x80);
        }
    }
    outbuf[out++] = (ScanElem)0;     /* terminator */
    return(out);
}

