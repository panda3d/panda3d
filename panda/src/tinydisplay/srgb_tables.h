/* 8-bit sRGB to 16-bit linear */
extern const unsigned short decode_sRGB[256];

/* 12-bit linear to 8-bit sRGB.  I used 12-bit because it can
   represent all possible 8-bit sRGB values. */
extern const unsigned char encode_sRGB[4096];
