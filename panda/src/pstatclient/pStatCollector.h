// Filename: pStatCollector.h
// Created by:  drose (10Jul00)
// 
////////////////////////////////////////////////////////////////////

#ifndef PSTATCOLLECTOR_H
#define PSTATCOLLECTOR_H

#include <pandabase.h>

#include "pStatThread.h"
#include "pStatClient.h"

#include <luse.h>

////////////////////////////////////////////////////////////////////
// 	 Class : PStatCollector
// Description : A lightweight class that represents a single element
//               that may be timed via stats.  Bracket the code to be
//               timed with calls to start() and stop().
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA PStatCollector {
#ifdef DO_PSTATS

private:
  INLINE PStatCollector();
  INLINE PStatCollector(PStatClient *client, int index);

public:
  INLINE PStatCollector(const string &name, 
			const RGBColorf &suggested_color = RGBColorf::zero(),
			int sort = -1,
			PStatClient *client = NULL);
  INLINE PStatCollector(const PStatCollector &parent,
			const string &name,
			const RGBColorf &suggested_color = RGBColorf::zero(),
			int sort = -1);

  INLINE PStatCollector(const PStatCollector &copy);
  INLINE void operator = (const PStatCollector &copy);

  INLINE void start();
  INLINE void start(const PStatThread &thread);
  INLINE void start(const PStatThread &thread, double as_of);

  INLINE void stop();
  INLINE void stop(const PStatThread &thread);
  INLINE void stop(const PStatThread &thread, double as_of);

private:
  PStatClient *_client;
  int _index;

friend class PStatClient;

#else  // DO_PSTATS
public:
  INLINE PStatCollector(const string &, 
			const RGBColorf & = RGBColorf::zero(),
			int = -1,
			PStatClient * = NULL) { }
  INLINE PStatCollector(const PStatCollector &,
			const string &,
			const RGBColorf & = RGBColorf::zero(),
			int = -1) { }

  INLINE void start() { }
  INLINE void start(const PStatThread &) { }
  INLINE void start(const PStatThread &, double) { }

  INLINE void stop() { }
  INLINE void stop(const PStatThread &) { }
  INLINE void stop(const PStatThread &, double) { }

#endif  // DO_PSTATS
};

#include "pStatCollector.I"

#endif

