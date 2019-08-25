/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file httpDate.cxx
 * @author drose
 * @date 2003-01-28
 */

#include "httpDate.h"

#include <ctype.h>

using std::setfill;
using std::setw;
using std::string;

static const int num_weekdays = 7;
static const char * const weekdays[num_weekdays] = {
  "Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"
};

static const int num_months = 12;
static const char * const months[num_months] = {
  "Jan", "Feb", "Mar", "Apr", "May", "Jun",
  "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"
};


/**
 * Decodes the string into a sensible date.  Returns 0 (!is_valid()) if the
 * string cannot be correctly decoded.
 */
HTTPDate::
HTTPDate(const string &format) {
  _time = (time_t)(-1);

  struct tm t;
  memset(&t, 0, sizeof(t));

  bool got_weekday = false;
  bool got_month = false;
  bool got_day = false;
  bool got_year = false;
  bool got_hour = false;
  bool got_minute = false;
  bool got_second = false;

  enum ExpectNext {
    EN_none,
    EN_second,
    EN_year
  };
  ExpectNext expect_next = EN_none;

  size_t pos = 0;
  string token = get_token(format, pos);
  while (!token.empty()) {
    ExpectNext expected = expect_next;
    expect_next = EN_none;

    if (isdigit(token[0])) {
      // Here's a number.
      int value = atoi(token.c_str());
      if (token[token.length() - 1] == ':') {
        // If it ends in a colon, it must be hh or mm.
        if (!got_hour) {
          t.tm_hour = value;
          got_hour = true;

        } else if (!got_minute) {
          t.tm_min = value;
          got_minute = true;
          expect_next = EN_second;

        } else {
          return;
        }

      } else if (token[token.length() - 1] == '/') {
        // If it ends in a colon, it must be mmdd.
        if (!got_month) {
          t.tm_mon = value - 1;
          got_month = true;

        } else if (!got_day) {
          t.tm_mday = value;
          got_day = true;
          expect_next = EN_year;

        } else {
          return;
        }

      } else {
        if (expected == EN_second) {
          // The first number following hh:mm: is always the seconds.
          t.tm_sec = value;
          got_second = true;

        } else if (expected == EN_year) {
          // The first number following mmdd is always the year.
          t.tm_year = value;
          got_year = true;

        } else if (!got_day) {
          // Assume it's a day.
          t.tm_mday = value;
          got_day = true;

        } else if (!got_year) {
          // It must be the year.
          t.tm_year = value;
          got_year = true;

        } else if (!got_hour) {
          t.tm_hour = value;
          got_hour = true;

        } else if (!got_minute) {
          t.tm_min = value;
          got_minute = true;

        } else if (!got_second) {
          t.tm_sec = value;
          got_second = true;

        } else {
          // Huh, an unexpected numeric value.
          return;
        }
      }

    } else {
      // This is a string token.  It should be either a month name or a day
      // name, or a timezone name--but the only timezone name we expect to see
      // is "GMT".
      bool matched = false;
      int i;

      for (i = 0; i < num_weekdays && !matched; i++) {
        if (token == weekdays[i]) {
          if (got_weekday) {
            return;
          }
          matched = true;
          got_weekday = true;
          t.tm_wday = i;
        }
      }

      for (i = 0; i < num_months && !matched; i++) {
        if (token == months[i]) {
          if (got_month) {
            return;
          }
          matched = true;
          got_month = true;
          t.tm_mon = i;
        }
      }

      if (!matched && token == "Gmt") {
        matched = true;
      }

      if (!matched) {
        // Couldn't figure this one out.
        return;
      }
    }

    token = get_token(format, pos);
  }

  // Now check that we got the minimum expected tokens.
  if (!(got_month && got_day && got_year && got_hour && got_minute)) {
    return;
  }

  // Also validate the tokens we did get.
  if (t.tm_year < 100) {
    // Two-digit year.  Assume it's in the same century, unless that
    // assumption puts it more than 50 years in the future.
    time_t now = time(nullptr);
    struct tm *tp = gmtime(&now);
    t.tm_year += 100 * (tp->tm_year / 100);
    if (t.tm_year - tp->tm_year > 50) {
      t.tm_year -= 100;
    }

  } else if (t.tm_year < 1900) {
    // Invalid three- or four-digit year.  Give up.
    return;

  } else {
    t.tm_year -= 1900;
  }

  if (!((t.tm_mon >= 0 && t.tm_mon < num_months) &&
        (t.tm_mday >= 1 && t.tm_mday <= 31) &&
        (t.tm_hour >= 0 && t.tm_hour < 60) &&
        (t.tm_min >= 0 && t.tm_min < 60) &&
        (t.tm_sec >= 0 && t.tm_sec < 62) /* maybe leap seconds */)) {
    return;
  }

  // Everything checks out; convert the date.  rdb made this an if 0 check as
  // timegm is a nonstandard extension so it fails in some situations even if
  // the compiler defines __GNUC__
#if 0

  _time = timegm(&t);

#else  // __GNUC__
  // Without the GNU extension timegm, we have to use mktime() instead.
  _time = mktime(&t);

  if (_time != (time_t)-1) {
    // Unfortunately, mktime() assumes local time; convert this back to GMT.
#if defined(IS_FREEBSD)
    time_t now = time(nullptr);
    struct tm *tp = localtime(&now);
    _time -= tp->tm_gmtoff;
#elif defined(_WIN32)
    long int timezone;
    _get_timezone(&timezone);
    _time -= timezone;
#else
    extern long int timezone;
    _time -= timezone;
#endif
  }
#endif  // __GNUC__
}

/**
 *
 */
string HTTPDate::
get_string() const {
  if (!is_valid()) {
    return "Invalid Date";
  }

  struct tm *tp = gmtime(&_time);

  std::ostringstream result;
  result
    << weekdays[tp->tm_wday] << ", "
    << setw(2) << setfill('0') << tp->tm_mday << " "
    << months[tp->tm_mon] << " "
    << setw(4) << setfill('0') << tp->tm_year + 1900 << " "
    << setw(2) << setfill('0') << tp->tm_hour << ":"
    << setw(2) << setfill('0') << tp->tm_min << ":"
    << setw(2) << setfill('0') << tp->tm_sec << " GMT";

  return result.str();
}


/**
 *
 */
bool HTTPDate::
input(std::istream &in) {
  (*this) = HTTPDate();

  // Extract out the quoted date string.
  char ch;
  in >> ch;
  if (ch != '"') {
    return false;
  }

  string date;
  ch = in.get();
  while (!in.fail() && ch != '"') {
    date += ch;
    ch = in.get();
  }

  if (ch != '"') {
    return false;
  }

  // Visual C++ has problems with "(*this) = HTTPDate(date)".
  HTTPDate new_date(date);
  (*this) = new_date;
  return is_valid();
}

/**
 *
 */
void HTTPDate::
output(std::ostream &out) const {
  // We put quotes around the string on output, so we can reliably detect the
  // end of the date string on input, above.
  out << '"' << get_string() << '"';
}

/**
 * Extracts the next token from the string starting at the indicated position.
 * Returns the token and updates pos.  When the last token has been extracted,
 * returns empty string.
 *
 * A token is defined as a contiguous sequence of digits or letters.  If it is
 * a sequence of letters, the function quietly truncates it to three letters
 * before returning, and forces the first letter to capital and the second two
 * to lowercase.  If it is a sequence of digits, the function also returns the
 * next character following the last digit (unless it is a letter).
 */
string HTTPDate::
get_token(const string &str, size_t &pos) {
  // Start by scanning for the first alphanumeric character.
  size_t start = pos;
  while (start < str.length() && !isalnum(str[start])) {
    start++;
  }

  if (start >= str.length()) {
    // End of the line.
    pos = string::npos;
    return string();
  }

  string token;

  if (isalpha(str[start])) {
    // A string of letters.
    token = toupper(str[start]);
    pos = start + 1;
    while (pos < str.length() && isalpha(str[pos])) {
      if (token.length() < 3) {
        token += tolower(str[pos]);
      }
      pos++;
    }

  } else {
    // A string of digits.
    pos = start + 1;
    while (pos < str.length() && isdigit(str[pos])) {
      pos++;
    }
    // Get one more character, so we can identify things like hh:
    if (pos < str.length() && !isalpha(str[pos])) {
      pos++;
    }
    token = str.substr(start, pos - start);
  }

  return token;
}
