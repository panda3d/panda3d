#include "mpg123.h"
#include "common.h"

#if 0
static void check_buffer_range(int size)
{
    int pos = (bsi->wordpointer-bsbuf) + (size>>3);

    if( pos >= fsizeold) {
        fprintf(stderr,"Pointer out of range (%d,%d)!\n",pos,fsizeold);
    }
}
#endif

void backbits(struct bitstream_info *bsi,int number_of_bits)
{
  bsi->bitindex    -= number_of_bits;
  bsi->wordpointer += (bsi->bitindex>>3);
  bsi->bitindex    &= 0x7;
}

int getbitoffset(struct bitstream_info *bsi) 
{
  return (-bsi->bitindex)&0x7;
}

int getbyte(struct bitstream_info *bsi)
{
#ifdef DEBUG_GETBITS
  if(bsi->bitindex) 
    fprintf(stderr,"getbyte called unsynched!\n");
#endif
  return *bsi->wordpointer++;
}

unsigned int getbits(struct bitstream_info *bsi,int number_of_bits)
{
  unsigned long rval;

#ifdef DEBUG_GETBITS
fprintf(stderr,"g%d",number_of_bits);
#endif

  if(!number_of_bits)
    return 0;

#if 0
   check_buffer_range(number_of_bits+bsi->bitindex);
#endif

  {
    rval = bsi->wordpointer[0];
    rval <<= 8;
    rval |= bsi->wordpointer[1];
    rval <<= 8;
    rval |= bsi->wordpointer[2];

    rval <<= bsi->bitindex;
    rval &= 0xffffff;

    bsi->bitindex += number_of_bits;

    rval >>= (24-number_of_bits);

    bsi->wordpointer += (bsi->bitindex>>3);
    bsi->bitindex &= 7;
  }

#ifdef DEBUG_GETBITS
fprintf(stderr,":%x ",rval);
#endif

  return rval;
}

unsigned int getbits_fast(struct bitstream_info *bsi,int number_of_bits)
{
  unsigned int rval;
#ifdef DEBUG_GETBITS
fprintf(stderr,"g%d",number_of_bits);
#endif

#if 0
   check_buffer_range(number_of_bits+bsi->bitindex);
#endif

  rval =  (unsigned char) (bsi->wordpointer[0] << bsi->bitindex);
  rval |= ((unsigned int) bsi->wordpointer[1]<<bsi->bitindex)>>8;
  rval <<= number_of_bits;
  rval >>= 8;

  bsi->bitindex += number_of_bits;

  bsi->wordpointer += (bsi->bitindex>>3);
  bsi->bitindex &= 7;

#ifdef DEBUG_GETBITS
fprintf(stderr,":%x ",rval);
#endif
  return rval;
}

unsigned int get1bit(struct bitstream_info *bsi)
{
  unsigned char rval;

#ifdef DEBUG_GETBITS
fprintf(stderr,"g%d",1);
#endif

#if 0
   check_buffer_range(1+bsi->bitindex);
#endif

  rval = *(bsi->wordpointer) << bsi->bitindex;

  bsi->bitindex++;
  bsi->wordpointer += (bsi->bitindex>>3);
  bsi->bitindex &= 7;

#ifdef DEBUG_GETBITS
fprintf(stderr,":%d ",rval>>7);
#endif

  return rval>>7;
}

