#define INSTALL_LIBS \
    Makefile.a.template Makefile.bin.template Makefile.foreign.template \
    Makefile.install.template Makefile.meta.template Makefile.o.include \
    Makefile.o.template Makefile.project.template Makefile.so.template \
    Makefile.ss.template Makefile.uxb.template

#define INSTALL_HEADERS \
    Makefile.a.rules Makefile.bin.rules Makefile.bin.vars \
    Makefile.foreign.rules Makefile.foreign.vars Makefile.install.rules \
    Makefile.install.vars Makefile.meta.rules Makefile.o.rules \
    Makefile.o.vars Makefile.project.vars Makefile.so.rules \
    Makefile.ss.rules Makefile.ss.vars Makefile.uxb.rules \
    Makefile.penv.vars \
    ctinstmake.pl ctproj.pl ctutils.pl

#define INSTALL_SCRIPTS \
    ctaddpkg ctaddtgt ctinitproj ctproj ctpathadjust

#define EXTRA_DIST \
    initialize
