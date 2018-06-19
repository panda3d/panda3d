/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file notifySeverity.h
 * @author drose
 * @date 2000-02-29
 */

#ifndef NOTIFYSEVERITY_H
#define NOTIFYSEVERITY_H

#include "dtoolbase.h"

BEGIN_PUBLISH
enum NotifySeverity {
  NS_unspecified,  // Never used, a special case internally.
  NS_spam,
  NS_debug,
  NS_info,
  NS_warning,
  NS_error,        // Specifically, a recoverable error.
  NS_fatal         // A nonrecoverable error--expect abort() or core dump.
};
END_PUBLISH

EXPCL_DTOOL_PRC std::ostream &operator << (std::ostream &out, NotifySeverity severity);
EXPCL_DTOOL_PRC std::istream &operator >> (std::istream &in, NotifySeverity &severity);


#endif
