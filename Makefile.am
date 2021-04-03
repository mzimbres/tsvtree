ACLOCAL_AMFLAGS = -I m4
AUTOMAKE_OPTIONS = subdir-objects

AM_COLOR_TESTS = always

Makefile.dep:
	-$(CXX) -MM ./src/*.cpp > $@

-include Makefile.dep

noinst_PROGRAMS = tsvsim
bin_PROGRAMS = tsvtree

tsvtree_SOURCES =
tsvtree_SOURCES += $(top_srcdir)/src/tree.cpp
tsvtree_SOURCES += $(top_srcdir)/src/tree.hpp
tsvtree_SOURCES += $(top_srcdir)/src/tsv.cpp
tsvtree_SOURCES += $(top_srcdir)/src/tsv.hpp
tsvtree_SOURCES += $(top_srcdir)/src/utils.cpp
tsvtree_SOURCES += $(top_srcdir)/src/utils.hpp
tsvtree_SOURCES += $(top_srcdir)/src/tsvtree.cpp

tsvtree_CPPFLAGS =
tsvtree_CPPFLAGS += $(BOOST_CPPFLAGS)
tsvtree_CPPFLAGS += -I$(top_srcdir)/src

tsvtree_LDFLAGS =
tsvtree_LDFLAGS += $(BOOST_LDFLAGS)

tsvtree_LDADD =
tsvtree_LDADD += -lfmt 
tsvtree_LDADD += -lboost_program_options

tsvsim_SOURCES =
tsvsim_SOURCES += $(top_srcdir)/src/tsvsim.cpp


EXTRA_DIST =
EXTRA_DIST += $(top_srcdir)/examples/cities.tsv
EXTRA_DIST += $(top_srcdir)/debian
EXTRA_DIST += $(top_srcdir)/src/check.sh

dist_man_MANS = $(top_srcdir)/doc/tsvtree.1

CLEANFILES =
#CLEANFILES += tsvtree
#CLEANFILES += tsvsim
#CLEANFILES += tsvsim.o
CLEANFILES += Makefile.dep

.PHONY: deb
deb: dist
	rm -rf tmp;\
	mkdir tmp;\
	mv $(distdir).tar.gz $(distdir)-1.tar.gz; \
	mv $(distdir)-1.tar.gz tmp;\
	cd tmp;\
	ln $(distdir)-1.tar.gz $(PACKAGE)_$(VERSION).orig.tar.gz; \
	tar -xvvzf $(distdir)-1.tar.gz; \
	cd $(distdir)/debian; debuild --no-sign -j1

#backup_emails = foo@bar.de
.PHONY: backup
backup: $(tarball_dir).tar.gz
	echo "Backup" | mutt -s "Backup" -a $< -- $(backup_emails)

