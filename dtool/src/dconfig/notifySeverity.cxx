// Filename: notifySeverity.C
// Created by:  drose (29Feb00)
// 
////////////////////////////////////////////////////////////////////

#include "notifySeverity.h"

ostream &operator << (ostream &out, NotifySeverity severity) {
  switch (severity) {
  case NS_spam: 
    return out << "spam";

  case NS_debug: 
    return out << "debug";

  case NS_info:
    return out << "info";

  case NS_warning:
    return out << "warning";
    
  case NS_error:
    return out << "error";

  case NS_fatal:
    return out << "fatal";

  case NS_unspecified:
    return out << "unspecified";
  }

  return out << "**invalid severity**";
}
