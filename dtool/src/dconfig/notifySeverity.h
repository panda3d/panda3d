// Filename: notifySeverity.h
// Created by:  drose (29Feb00)
// 
////////////////////////////////////////////////////////////////////

#ifndef NOTIFYSEVERITY_H
#define NOTIFYSEVERITY_H

#include <dtoolbase.h>

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

EXPCL_DTOOL ostream &operator << (ostream &out, NotifySeverity severity);

#endif
