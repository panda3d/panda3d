// Filename: iffId.h
// Created by:  drose (23Apr01)
// 
////////////////////////////////////////////////////////////////////

#ifndef IFFID_H
#define IFFID_H

#include <pandatoolbase.h>

#include <numeric_types.h>

////////////////////////////////////////////////////////////////////
// 	 Class : IffId
// Description : A four-byte chunk ID appearing in an "IFF" file.
//               This is used to identify the meaning of each chunk,
//               and can be treated either as a concrete object or as
//               a string, something like a TypeHandle.
////////////////////////////////////////////////////////////////////
class IffId {
public:
  INLINE IffId();
  INLINE IffId(const char id[4]);
  INLINE IffId(const IffId &copy);
  INLINE void operator = (const IffId &copy);

  INLINE bool operator == (const IffId &other) const;
  INLINE bool operator != (const IffId &other) const;
  INLINE bool operator < (const IffId &other) const;

  INLINE string get_name() const;
  
private:
  union {
    PN_uint32 _n;
    char _c[4];
  } _id;
};

#include "iffId.I"

INLINE ostream &operator << (ostream &out, const IffId &id) {
  return out << id.get_name();
}
 
#endif
