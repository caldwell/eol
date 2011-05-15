CC = gcc
CPPFLAGS = -Wall -g -MMD
LN = ln -s

p=)
os:=$(shell case `uname -s` in \
                  Linux$p echo linux;; \
                  CYGWIN32_95$p echo  dos;; \
                  OpenStep$p echo next;; \
            esac)#next#linux#aspi

eol_objs = eol.o
getopt = #getopt/getopt.o getopt/getopt1.o

prefix = /usr/local
bindir = $(prefix)/bin
mandir = $(prefix)/man/man$(mansection)
p = to

ifeq ($(os),next)
 CPPFLAGS += -Igetopt
 eol_objs += $(getopt)
%.o : %.c
	$(CC) $(CFLAGS) -c -o $@ $<
endif

all: MyVersion
version = $(shell ./MyVersion)

ifeq ($(os),dos)
 version = no-auto-version-with-dos
 p = 
 e = .exe
 CPPFLAGS += -Igetopt
 eol_objs += $(getopt) #libwnaspi32.a
 LN = cp
 prefix = //rewind/source/eol
endif

links = $(p)mac$e $(p)dos$e $(p)unix$e $(p)win$e

prog = eol$e

mansection = 1
manpage = $(prog).$(mansection)
manlinks = $(patsubst %,%.$(mansection),$(links))

all: $(prog) $(links) $(manlinks)

$(prog): $(eol_objs) $(aspi_lib)
	$(CC) $(CPPFLAGS) -o $(prog) $(eol_objs) $(libs)

$(links) : $(prog)
	$(LN) $< $@

$(manlinks) : $(manpage)
	$(LN) $< $@

install : $(prog) $(manpage)
	-mkdir -p $(bindir)
	( cd $(bindir) ; \
	  for f in $(links) ; do \
		rm -f $$f; \
	   	ln -s $(prog) $$f ; \
	  done)
ifeq ($(os),next)
	install -s -o 0 -g 0 $(prog) $(bindir)/$(prog)
else
	install -s -o0 -g0 -m u+rwx,g+rwx,o+rx $(prog) $(bindir)/$(prog)
endif
	-mkdir -p $(mandir)
	( cd $(mandir) ; \
	  for f in $(manlinks) ; do \
		rm -f $$f; \
	   	ln -s $(manpage) $$f ; \
	  done)
ifeq ($(os),next)
	install -o 0 -g 0 $(manpage) $(mandir)/$(manpage)
else
	install -o0 -g0 -m u+rwx,g+rwx,o+rx $(manpage) $(mandir)/$(manpage)
endif

install-log:
	perl -e 'map { print "$(bindir)/$$_\n" } @ARGV' $(prog) $(links) > $@

MyVersion : version.h
	gcc -x c version.h -DVERSION_STRING_NOW -o $@

#scsi-$(version): tarball

tarball : MyVersion $(prog)-$(version).tar.gz

$(prog)-$(version).tar.gz : MyVersion makefile
	rm -rf $(prog)-$(version)
	mkdir $(prog)-$(version)
	-cp -a *.c *.cc *.h *.pl makefile makefile.rpm *.in $(manpage) ChangeLog $(prog)-$(version)
#	mkdir $(prog)-$(version)/getopt
#	cp -a getopt/*.[hc] $(prog)-$(version)/getopt
	tar czf $(prog)-$(version).tar.gz $(prog)-$(version)
	rm -rf $(prog)-$(version)

# --- This is the RPM section:
$(prog).spec : $(prog).spec.in MyVersion
	perl -pe 's/version .*/version $(version)/' $< > $@

rpmproj = $(prog)
copy-tarball=yes
include makefile.rpm
# --- this concludes the RPM section.

# cheat and use the handmade def instead of makeing our own
#%.def : %.dll
#	@echo grepping through $< for symbols to make $@
#	-touch $@

lib%.a : %.def
	dlltool --output-lib $@ --dllname $*.dll --def $< #--output-exp $*.exp 

suid: $(prog)
ifeq ($(os),linux)
	chown root $<
	chmod u+s $<
endif

bin = /shcope/bin
strips = $(prog)

ifeq ($(os),aspi)
 strips += $(links)
 bin = //s/aspi/bin
endif

strip:
	strip $(strips)

#install: $(prog) $(links) strip
#	cp $(prog) $(links) $(bin)

clean:
	rm -f $(prog) $(links) $(eol_objs) *~ *.d MyVersion

-include *.d
