// Filename: txaFile.cxx
// Created by:  drose (30Nov00)
// 
////////////////////////////////////////////////////////////////////

#include "txaFile.h"
#include "string_utils.h"
#include "palettizer.h"
#include "paletteGroup.h"
#include "textureImage.h"

#include <notify.h>

////////////////////////////////////////////////////////////////////
//     Function: TxaFile::Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
TxaFile::
TxaFile() {
}

////////////////////////////////////////////////////////////////////
//     Function: TxaFile::read
//       Access: Public
//  Description: Reads the indicated .txa filename, and returns true
//               if successful, or false if there is an error.
////////////////////////////////////////////////////////////////////
bool TxaFile::
read(Filename filename) {
  filename.set_text();
  ifstream in;
  if (!filename.open_read(in)) {
    nout << "Unable to open " << filename << "\n";
    return false;
  }

  string line;
  getline(in, line);
  int line_number = 1;

  while (!in.eof() && !in.fail()) {
    bool okflag = true;

    // Strip off the comment.
    size_t hash = line.find('#');
    if (hash != string::npos) {
      line = line.substr(0, hash);
    }
    line = trim_left(line);
    if (line.empty()) {
      // Empty lines are ignored.

    } else if (line[0] == ':') {
      // This is a keyword line.
      vector_string words;
      extract_words(line, words);
      if (words[0] == ":group") {
	okflag = parse_group_line(words);

      } else {
	nout << "Invalid keyword: " << line << "\n";
	okflag = false;
      }

    } else {
      _lines.push_back(TxaLine());
      TxaLine &txa_line = _lines.back();

      okflag = txa_line.parse(line);
    }

    if (!okflag) {
      nout << "Error on line " << line_number << " of " << filename << "\n";
      return false;
    }

    getline(in, line);
    line_number++;
  }

  if (!in.eof()) {
    nout << "I/O error reading " << filename << "\n";
    return false;
  }

  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: TxaFile::match_egg
//       Access: Public
//  Description: Searches for a matching line in the .txa file for the
//               given egg file and applies its specifications.  If a
//               match is found, returns true; otherwise, returns
//               false.  Also returns false if all the matching lines
//               for the egg file include the keyword "cont".
////////////////////////////////////////////////////////////////////
bool TxaFile::
match_egg(EggFile *egg_file) const {
  Lines::const_iterator li;
  for (li = _lines.begin(); li != _lines.end(); ++li) {
    if ((*li).match_egg(egg_file)) {
      return true;
    }
  }

  return false;
}

////////////////////////////////////////////////////////////////////
//     Function: TxaFile::match_texture
//       Access: Public
//  Description: Searches for a matching line in the .txa file for the
//               given texture and applies its specifications.  If a
//               match is found, returns true; otherwise, returns
//               false.  Also returns false if all the matching lines
//               for the texture include the keyword "cont".
////////////////////////////////////////////////////////////////////
bool TxaFile::
match_texture(TextureImage *texture) const {
  Lines::const_iterator li;
  for (li = _lines.begin(); li != _lines.end(); ++li) {
    if ((*li).match_texture(texture)) {
      return true;
    }
  }

  return false;
}

////////////////////////////////////////////////////////////////////
//     Function: TxaFile::write
//       Access: Public
//  Description: Outputs a representation of the lines that were read
//               in to the indicated output stream.  This is primarily
//               useful for debugging.
////////////////////////////////////////////////////////////////////
void TxaFile::
write(ostream &out) const {
  Lines::const_iterator li;
  for (li = _lines.begin(); li != _lines.end(); ++li) {
    out << (*li) << "\n";
  }
}


////////////////////////////////////////////////////////////////////
//     Function: TxaFile::parse_group_line
//       Access: Private
//  Description: Handles the line in a .txa file that begins with the
//               keyword ":group" and indicates the relationships
//               between one or more groups.
////////////////////////////////////////////////////////////////////
bool TxaFile::
parse_group_line(const vector_string &words) {
  vector_string::const_iterator wi;
  wi = words.begin();
  assert (wi != words.end());
  ++wi;

  const string &group_name = (*wi);
  PaletteGroup *group = pal->get_palette_group(group_name);
  ++wi;

  enum State {
    S_none,
    S_with,
    S_dir,
  };
  State state = S_none;

  while (wi != words.end()) {
    const string &word = (*wi);
    if (word == "with") {
      state = S_with;

    } else if (word == "dir") {
      state = S_dir;

    } else {
      switch (state) {
      case S_none:
	nout << "Invalid keyword: " << word << "\n";
	return false;

      case S_with:
	group->group_with(pal->get_palette_group(word));
	break;

      case S_dir:
	group->set_dirname(word);
	state = S_none;
	break;
      }
    }

    ++wi;
  }

  return true;
}
