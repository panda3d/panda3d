// Filename: dcPackerInterface.I
// Created by:  drose (18Jun04)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001 - 2004, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://etc.cmu.edu/panda3d/docs/license/ .
//
// To contact the maintainers of this program write to
// panda3d-general@lists.sourceforge.net .
//
////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////
//     Function: DCPackerInterface::do_pack_int8
//       Access: Public, Static
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE void DCPackerInterface::
do_pack_int8(char *buffer, int value) {
  buffer[0] = (char)(value & 0xff);
}

////////////////////////////////////////////////////////////////////
//     Function: DCPackerInterface::do_pack_int16
//       Access: Public, Static
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE void DCPackerInterface::
do_pack_int16(char *buffer, int value) {
  buffer[0] = (char)(value & 0xff);
  buffer[1] = (char)((value >> 8) & 0xff);
}

////////////////////////////////////////////////////////////////////
//     Function: DCPackerInterface::do_pack_int32
//       Access: Public, Static
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE void DCPackerInterface::
do_pack_int32(char *buffer, int value) {
  buffer[0] = (char)(value & 0xff);
  buffer[1] = (char)((value >> 8) & 0xff);
  buffer[2] = (char)((value >> 16) & 0xff);
  buffer[3] = (char)((value >> 24) & 0xff);
}

////////////////////////////////////////////////////////////////////
//     Function: DCPackerInterface::do_pack_int64
//       Access: Public, Static
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE void DCPackerInterface::
do_pack_int64(char *buffer, PN_int64 value) {
  buffer[0] = (char)(value & 0xff);
  buffer[1] = (char)((value >> 8) & 0xff);
  buffer[2] = (char)((value >> 16) & 0xff);
  buffer[3] = (char)((value >> 24) & 0xff);
  buffer[4] = (char)((value >> 32) & 0xff);
  buffer[5] = (char)((value >> 40) & 0xff);
  buffer[6] = (char)((value >> 48) & 0xff);
  buffer[7] = (char)((value >> 56) & 0xff);
}

////////////////////////////////////////////////////////////////////
//     Function: DCPackerInterface::do_pack_uint8
//       Access: Public, Static
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE void DCPackerInterface::
do_pack_uint8(char *buffer, unsigned int value) {
  buffer[0] = (char)(value & 0xff);
}

////////////////////////////////////////////////////////////////////
//     Function: DCPackerInterface::do_pack_uint16
//       Access: Public, Static
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE void DCPackerInterface::
do_pack_uint16(char *buffer, unsigned int value) {
  buffer[0] = (char)(value & 0xff);
  buffer[1] = (char)((value >> 8) & 0xff);
}

////////////////////////////////////////////////////////////////////
//     Function: DCPackerInterface::do_pack_uint32
//       Access: Public, Static
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE void DCPackerInterface::
do_pack_uint32(char *buffer, unsigned int value) {
  buffer[0] = (char)(value & 0xff);
  buffer[1] = (char)((value >> 8) & 0xff);
  buffer[2] = (char)((value >> 16) & 0xff);
  buffer[3] = (char)((value >> 24) & 0xff);
}

////////////////////////////////////////////////////////////////////
//     Function: DCPackerInterface::do_pack_uint64
//       Access: Public, Static
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE void DCPackerInterface::
do_pack_uint64(char *buffer, PN_uint64 value) {
  buffer[0] = (char)(value & 0xff);
  buffer[1] = (char)((value >> 8) & 0xff);
  buffer[2] = (char)((value >> 16) & 0xff);
  buffer[3] = (char)((value >> 24) & 0xff);
  buffer[4] = (char)((value >> 32) & 0xff);
  buffer[5] = (char)((value >> 40) & 0xff);
  buffer[6] = (char)((value >> 48) & 0xff);
  buffer[7] = (char)((value >> 56) & 0xff);
}

////////////////////////////////////////////////////////////////////
//     Function: DCPackerInterface::do_pack_float64
//       Access: Public, Static
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE void DCPackerInterface::
do_pack_float64(char *buffer, double value) {
#ifdef WORDS_BIGENDIAN
  // Reverse the byte ordering for big-endian machines.
  char *p = (char *)value;
  for (size_t i = 0; i < 8; i++) {
    buffer[i] = p[7 - i];
  }
#else
  memcpy(buffer, &value, 8);
#endif
}


////////////////////////////////////////////////////////////////////
//     Function: DCPackerInterface::do_unpack_int8
//       Access: Public, Static
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE int DCPackerInterface::
do_unpack_int8(const char *buffer) {
  return (int)(signed char)buffer[0];
}

////////////////////////////////////////////////////////////////////
//     Function: DCPackerInterface::do_unpack_int16
//       Access: Public, Static
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE int DCPackerInterface::
do_unpack_int16(const char *buffer) {
  return (int)((unsigned int)(unsigned char)buffer[0] |
               ((int)(signed char)buffer[1] << 8));
}

////////////////////////////////////////////////////////////////////
//     Function: DCPackerInterface::do_unpack_int32
//       Access: Public, Static
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE int DCPackerInterface::
do_unpack_int32(const char *buffer) {
  return (int)((unsigned int)(unsigned char)buffer[0] |
               ((unsigned int)(unsigned char)buffer[1] << 8) |
               ((unsigned int)(unsigned char)buffer[2] << 16) |
               ((int)(signed char)buffer[3] << 24));
}

////////////////////////////////////////////////////////////////////
//     Function: DCPackerInterface::do_unpack_int64
//       Access: Public, Static
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE PN_int64 DCPackerInterface::
do_unpack_int64(const char *buffer) {
  return (PN_int64)((PN_uint64)(unsigned char)buffer[0] |
                    ((PN_uint64)(unsigned char)buffer[1] << 8) |
                    ((PN_uint64)(unsigned char)buffer[2] << 16) |
                    ((PN_uint64)(unsigned char)buffer[3] << 24) |
                    ((PN_uint64)(unsigned char)buffer[4] << 32) |
                    ((PN_uint64)(unsigned char)buffer[5] << 40) |
                    ((PN_uint64)(unsigned char)buffer[6] << 48) |
                    ((PN_int64)(signed char)buffer[7] << 54));
}
////////////////////////////////////////////////////////////////////
//     Function: DCPackerInterface::do_unpack_uint8
//       Access: Public, Static
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE unsigned int DCPackerInterface::
do_unpack_uint8(const char *buffer) {
  return (unsigned int)(unsigned char)buffer[0];
}

////////////////////////////////////////////////////////////////////
//     Function: DCPackerInterface::do_unpack_uint16
//       Access: Public, Static
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE unsigned int DCPackerInterface::
do_unpack_uint16(const char *buffer) {
  return ((unsigned int)(unsigned char)buffer[0] |
          ((unsigned int)(unsigned char)buffer[1] << 8));
}

////////////////////////////////////////////////////////////////////
//     Function: DCPackerInterface::do_unpack_uint32
//       Access: Public, Static
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE unsigned int DCPackerInterface::
do_unpack_uint32(const char *buffer) {
  return ((unsigned int)(unsigned char)buffer[0] |
          ((unsigned int)(unsigned char)buffer[1] << 8) |
          ((unsigned int)(unsigned char)buffer[2] << 16) |
          ((unsigned int)(unsigned char)buffer[3] << 24));
}

////////////////////////////////////////////////////////////////////
//     Function: DCPackerInterface::do_unpack_uint64
//       Access: Public, Static
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE PN_uint64 DCPackerInterface::
do_unpack_uint64(const char *buffer) {
  return ((PN_uint64)(unsigned char)buffer[0] |
          ((PN_uint64)(unsigned char)buffer[1] << 8) |
          ((PN_uint64)(unsigned char)buffer[2] << 16) |
          ((PN_uint64)(unsigned char)buffer[3] << 24) |
          ((PN_uint64)(unsigned char)buffer[4] << 32) |
          ((PN_uint64)(unsigned char)buffer[5] << 40) |
          ((PN_uint64)(unsigned char)buffer[6] << 48) |
          ((PN_int64)(signed char)buffer[7] << 54));
}


////////////////////////////////////////////////////////////////////
//     Function: DCPackerInterface::do_unpack_float64
//       Access: Public, Static
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE double DCPackerInterface::
do_unpack_float64(const char *buffer) {
#ifdef WORDS_BIGENDIAN
  char reverse[8];
  
  // Reverse the byte ordering for big-endian machines.
  for (size_t i = 0; i < 8; i++) {
    reverse[i] = buffer[7 - i];
  }
  return *(double *)reverse;
#else
  return *(double *)buffer;
#endif  // WORDS_BIGENDIAN 
}
