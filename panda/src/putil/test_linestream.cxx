// Filename: test_linestream.cxx
// Created by:  drose (26Feb00)
// 
////////////////////////////////////////////////////////////////////

#include "lineStream.h"

#include <notify.h>

int
main(int argc, char *argv[]) {
  LineStream ls;

  int i = 0;
  while (ls) {
    if ((i % 5) == 0) {
      if (ls.is_text_available()) {
	nout << "Got line: '" << ls.get_line() << "'";
	if (ls.has_newline()) {
	  nout << " (nl)";
	}
	nout << "\n";
      }
    }

    ls << "123456789 ";
    if ((i % 6)==0) {
      ls << "\n";
    }
    i++;
  }

  return 0;
}

  
