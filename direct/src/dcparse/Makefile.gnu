OFILES = \
  dcParser.o dcLexer.o \
  dcAtomicField.o dcClass.o dcField.o dcFile.o dcMolecularField.o \
  dcSubatomicType.o indent.o \
  dcparse.o

# Cheesy dependencies.
HFILES = \
  dcAtomicField.h dcClass.h dcClassDescription.h dcField.h dcFile.h \
  dcLexerDefs.h dcMolecularField.h dcParser.h dcParserDefs.h	\
  dcSubatomicType.h dcbase.h indent.h

dcparse : $(OFILES)
	g++ -o $@ $(OFILES)

dcParser.cxx : dcParser.yxx
	bison -d --name-prefix=dcyy -o $@ $<
	mv $@.h dcParser.h

dcLexer.cxx : dcLexer.lxx
	flex -Pdcyy -ot.lex $<
	sed '/#include <unistd.h>/d' t.lex >$@
	rm -f t.lex

%.o: %.cxx $(HFILES)
	g++ -c -g -Wall -o $@ $<

clean:
	rm -f *.o dcparse

zip:
	rm -f dcparse.zip
	zip dcparse.zip Makefile.gnu *.cxx *.h *.lxx *.yxx *.dc \
             dc.dsp dc.dsw
