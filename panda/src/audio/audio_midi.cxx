// Filename: audio_midi.cxx
// Created by:  cary (26Sep00)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://www.panda3d.org/license.txt .
//
// To contact the maintainers of this program write to
// panda3d@yahoogroups.com .
//
////////////////////////////////////////////////////////////////////

#include "audio_midi.h"
#include "config_audio.h"
#include <pandabase.h>    // get iostream and fstream

#define SHORT short
#define LONG long

#define RIFF 0x52494646
#define CTMF 0x43544d46
#define MThd 0x4d546864
#define MTrk 0x4d54726b

// take data off the supplimental stream until it's empty
inline static unsigned char read8(istream& is, istream& supp) {
  unsigned char b;
  if (supp.eof()) {
    is >> b;
  } else {
    supp >> b;
  }
  return b;
}

inline static unsigned SHORT read16(istream& is, istream& supp) {
  unsigned char b1, b2;
  b1 = read8(is, supp);
  b2 = read8(is, supp);
  unsigned SHORT ret = (b1 << 8) | (b2);
  return ret;
}

inline static unsigned LONG read32(istream& is, istream& supp) {
  unsigned char b1, b2, b3, b4;
  b1 = read8(is, supp);
  b2 = read8(is, supp);
  b3 = read8(is, supp);
  b4 = read8(is, supp);
  unsigned int LONG ret = (b1 << 24) | (b2 << 16) | (b3 << 8) | (b4);
  return ret;
}

inline static void scroll8(unsigned LONG& prev, unsigned LONG& curr,
               istream& is, istream& supp) {
  unsigned char b1 = ((curr >> 24) & 0xff);
  prev = ((prev << 8) & 0xffffff00) | (b1);
  b1 = read8(is, supp);
  curr = ((curr << 8) & 0xffffff00) | (b1);
}

AudioMidi::AudioMidi(Filename filename, int header_idx) {
  filename.set_binary();
  ifstream in;
  if (!filename.open_read(in)) {
    cerr << "ACK, cannot read '" << filename << "'" << endl;
    return;
  }

  istringstream dummy("");
  unsigned LONG prev = 0x0;
  unsigned LONG curr = read32(in, dummy);
  int count = 0;
  bool done = false;
  do {
    if (curr == MThd) {
      ++count;
      if (count == header_idx)
    done = true;
      else {
    scroll8(prev, curr, in, dummy);
    scroll8(prev, curr, in, dummy);
    scroll8(prev, curr, in, dummy);
    scroll8(prev, curr, in, dummy);
      }
    } else {
      scroll8(prev, curr, in, dummy);
    }
    if (in.eof())
      done = true;
  } while (!done);
  if (in.eof()) {
    cerr << "fewer then " << header_idx << " headers in file (" << count
     << ")" << endl;
    return;
  }
  if (prev == RIFF) {
    if (audio_cat.is_debug())
      audio_cat->debug() << "it's a RIFF file" << endl;
    curr = read32(in, dummy);
    curr = read32(in, dummy);
    curr = read32(in, dummy);
    curr = read32(in, dummy);
  }
  unsigned LONG tracklen;
  unsigned SHORT format;
  unsigned SHORT numtracks;
  unsigned SHORT division;
  if (curr == MThd) {
    if (audio_cat.is_debug())
      audio_cat->debug() << "easy header" << endl;
    tracklen = read32(in, dummy);
    format = read16(in, dummy);
    numtracks = read16(in, dummy);
    division = read16(in, dummy);
  } else if (curr == CTMF) {
    // Creative Labs CMF file.  We're not supporting this yet
    cerr << "don't support Creative Labs CMF files yet" << endl;
    return;
  } else {
    if (audio_cat.is_debug())
      audio_cat->debug() << "hard header" << endl;
    done = false;
    do {
      if (curr == MThd)
    done = true;
      else
    scroll8(prev, curr, in, dummy);
      if (in.eof())
    done = true;
    } while (!done);
    if (in.eof()) {
      cerr << "truncated file!" << endl;
      return;
    }
    tracklen = read32(in, dummy);
    format = read16(in, dummy);
    numtracks = read16(in, dummy);
    division = read16(in, dummy);
  }
  if (audio_cat.is_debug())
    audio_cat->debug() << "Read header.  tracklen = " << tracklen
               << "  format = " << format << "  numtracks = "
               << numtracks << "  division = " << division << endl;
  for (int currtrack = 0; currtrack < numtracks; ++currtrack) {
    string fudge;
    curr = read32(in, dummy);
    if (curr != MTrk) {
      if (audio_cat.is_debug())
    audio_cat->debug() << "having to seach for track #" << currtrack
               << endl;
      if (curr == MThd) {
    if (audio_cat.is_debug())
      audio_cat->debug() << "hit a header instead, skipping track" << endl;
    continue;
      } else {
    done = false;
    if (currtrack > 0) {
      string stmp = (*(_seq.rbegin()));
      int i = stmp.rfind("MTrk");
      if (i != string::npos) {
        fudge = stmp.substr(i+4, string::npos);
        unsigned char b;
        b = ((curr >> 24) & 0xff);
        fudge += b;
        b = ((curr >> 16) & 0xff);
        fudge += b;
        b = ((curr >> 8) & 0xff);
        fudge += b;
        b = (curr & 0xff);
        fudge += b;
        done = true;
      }
    }
    if (!done) {
      do {
        if (curr == MTrk)
          done = true;
        else
          scroll8(prev, curr, in, dummy);
        if (in.eof())
          done = true;
      } while (!done);
      if (in.eof()) {
        cerr << "truncated file" << endl;
        return;
      }
    }
      }
    }
    if (audio_cat.is_debug())
      audio_cat->debug() << "fudge = '" << fudge << "'" << endl;
    istringstream fudges(fudge);
    if (fudge.empty()) {
      // force EOF
      unsigned char b;
      fudges >> b;
    }
    unsigned LONG thislen = read32(in, fudges);
    if (audio_cat.is_debug())
      audio_cat->debug() << "found track #" << currtrack << " with length = "
             << thislen << endl;
    {
      ostringstream os;
      int i;
      for (i=0; i<thislen; ++i) {
    unsigned char b;
    b = read8(in, fudges);
    os << b;
    if (in.eof() && ((i+1) < thislen))
      break;
      }
      if (in.eof() && (i != thislen)) {
    cerr << "truncated file" << endl;
      }
      string s = os.str();
      _seq.push_back(s);
      if (audio_cat.is_debug())
    audio_cat->debug() << "track data (" << s.length() << "): '" << s
               << "'" << endl;
    }
  }
  if ((_seq.size() != numtracks) && audio_cat.is_debug())
    audio_cat->debug()
      << "actual number of tracks read does not match header. ("
      << _seq.size() << " != " << numtracks << ")" << endl;
}

AudioMidi::AudioMidi(const AudioMidi& c) : _seq(c._seq) {}

AudioMidi::~AudioMidi(void) {
}

AudioMidi& AudioMidi::operator=(const AudioMidi&) {
  return *this;
}

bool AudioMidi::operator==(const AudioMidi&) const {
  return false;
}
