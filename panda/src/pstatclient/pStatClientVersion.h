// Filename: pStatClientVersion.h
// Created by:  drose (21May01)
// 
////////////////////////////////////////////////////////////////////

#ifndef PSTATCLIENTVERSION_H
#define PSTATCLIENTVERSION_H

#include <pandabase.h>

#include <referenceCount.h>
#include <pointerTo.h>

////////////////////////////////////////////////////////////////////
//       Class : PStatClientVersion
// Description : Records the version number of a particular client.
//               Normally this will be the same as
//               get_current_pstat_major/minor_version().
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA PStatClientVersion : public ReferenceCount {
public:
  PStatClientVersion();

  INLINE int get_major_version() const;
  INLINE int get_minor_version() const;

  INLINE void set_version(int major, int minor);

  INLINE bool is_at_least(int major, int minor) const;

private:
  int _major_version;
  int _minor_version;
};

#include "pStatClientVersion.I"

#endif

