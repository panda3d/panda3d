// Filename: ppremake.cxx
// Created by:  drose (25Sep00)
// 
////////////////////////////////////////////////////////////////////

#include "ppremake.h"
#include "ppMain.h"
#include "ppScope.h"

#ifdef HAVE_GETOPT
#include <getopt.h>
#else
#include <gnu_getopt.h>
#endif

static void
usage() {
  cerr <<
    "\n"
    "ppremake [opts] subdir-name [subdir-name..]\n"
    "ppremake\n"
    "\n"
    "This is Panda pre-make: a script preprocessor that scans the directory\n"
    "hierarchy beginning at root-directory, looking for directories that\n"
    "contain a file called " SOURCE_FILENAME ".  At the top of the directory\n"
    "tree must be a file called " PACKAGE_FILENAME ", which should define\n"
    "key variable definitions for processing, as well as pointing out the\n"
    "locations of further config files.\n\n"

    "The package file is read and interpreted, followed by each source file\n"
    "in turn; after each source file is read, the template file (specified in\n"
    "the config file) is read.  The template file  contains the actual statements\n"
    "to be output and will typically be set up to generate Makefiles or whatever\n"
    "is equivalent and appropriate to the particular build environment in use.\n\n"

    "The parameters are the names of the subdirectories (their local names, not\n"
    "the relative or full paths to them) that are to be processed.  All\n"
    "subdirectories (that contain a file named " SOURCE_FILENAME ") will be\n"
    "scanned, but only the named subdirectories will have output files\n"
    "generated.  If no parameter is given, then all directories will be\n"
    "processed.\n\n"

    "Options:\n\n"

    "  -h           Display this page.\n"
    "  -V           Report the version of ppremake, and exit.\n"
    "  -P           Report the current platform name, and exit.\n\n"
    "  -p platform  Build as if for the indicated platform name.\n\n";
}

static void
report_version() {
  cerr << "This is " << PACKAGE << " version " << VERSION << ".\n";
}

static void
report_platform() {
  cerr << "ppremake built for platform " << PLATFORM << ".\n";
}

int
main(int argc, char *argv[]) {
  extern char *optarg;
  extern int optind;
  const char *optstr = "hVPp:";

  string platform = PLATFORM;
  int flag = getopt(argc, argv, optstr);

  while (flag != EOF) {
    switch (flag) {
    case 'h':
      usage();
      exit(0);

    case 'V':
      report_version();
      exit(0);
      break;

    case 'P':
      report_platform();
      exit(0);
      break;

    case 'p':
      platform = optarg;
      break;

    default:
      exit(1);
    }
    flag = getopt(argc, argv, optstr);
  }

  argc -= (optind-1);
  argv += (optind-1);

  PPScope global_scope((PPNamedScopes *)NULL);
  global_scope.define_variable("PROGRAM", PACKAGE);
  global_scope.define_variable("PROGVER", VERSION);
  global_scope.define_variable("PLATFORM", platform);

  PPMain ppmain(&global_scope);
  if (!ppmain.read_source(".")) {
    exit(1);
  }

  if (argc < 2) {
    if (!ppmain.process_all()) {
      exit(1);
    }
  } else {
    for (int i = 1; i < argc; i++) {
      if (!ppmain.process(argv[i])) {
	cerr << "Unable to process " << argv[i] << ".\n";
	exit(1);
      }
    }
  }

  cerr << "No errors.\n";
  return (0);
}
