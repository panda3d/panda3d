// Filename: pfstream.h
// Created by:  cary (27Aug98)
//
////////////////////////////////////////////////////////////////////

#ifndef __PFSTREAM_H__
#define __PFSTREAM_H__

#include <dtoolbase.h>

#include <string>
#include <stdio.h>

#ifdef WIN32_VC
#define popen _popen
#define pclose _pclose
#endif

class EXPCL_DTOOL ipfstream {
   private:
      ifstream *ifs;
      FILE *fd;
      bool ok;

      ipfstream() {}
      ipfstream(const ipfstream&) {}
   public:
      ipfstream(std::string cmd, int mode = ios::in, int = 0664) : ok(false) {
#ifndef PENV_PS2
	 fd = popen(cmd.c_str(), (mode & ios::in)?"r":"w");
	 if (fd != (FILE *)0l) {
// this can't work under windows until this is re-written
#ifdef WIN32_VC 
           cerr << "ipfstream: this class needs to be rewritten for windows!"
		<< endl;
#else
	    ifs = new ifstream(fileno(fd));
#endif // WIN32_VC 
	    ok = true;
	 }
#endif // PENV_PS2
      }

      ~ipfstream() {
#ifndef PENV_PS2
	 if (ok) {
	    pclose(fd);
	    delete ifs;
	 }
#endif // PENV_PS2
      }

      INLINE operator istream&() {
	 return *ifs;
      }
};

class EXPCL_DTOOL opfstream {
   private:
      ofstream *ofs;
      FILE *fd;
      bool ok;

      opfstream() {}
      opfstream(const opfstream&) {}
   public:
      opfstream(std::string cmd, int mode = ios::out, int = 0664) : ok(false) {
#ifndef PENV_PS2
	 fd = popen(cmd.c_str(), (mode & ios::out)?"w":"r");
	 if (fd != (FILE *)0L) {
#ifndef WIN32_VC 
	    ofs = new ofstream(fileno(fd));
#endif
	    ok = true;
	 }
#endif // PENV_PS2
      }
      ~opfstream() {
#ifndef PENV_PS2
	 if (ok) {
	    pclose(fd);
	    delete ofs;
	 }
#endif
      }

      INLINE operator ostream&() {
	 return *ofs;
      }
};

#endif /* __PFSTREAM_H__ */


