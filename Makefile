pkg_name = tsvtree
pkg_version = 1.0.0
pkg_revision = 0
tarball_name = $(pkg_name)-$(pkg_version)-$(pkg_revision)
tarball_dir = $(pkg_name)-$(pkg_version)
prefix = /usr
datarootdir = $(prefix)/share
datadir = $(datarootdir)
docdir = $(datadir)/doc/$(pkg_name)
bindir = $(prefix)/bin
srcdir = .

bin_final_dir = $(DESTDIR)$(bindir)
doc_final_dir = $(DESTDIR)$(docdir)

CPPFLAGS += -std=c++17 -Wall -Werror
CPPFLAGS += -I. -I$./src
CPPFLAGS += $(CXXFLAGS)
CPPFLAGS += -g

VPATH = ./src

exes =
exes += tsvtree
exes += treesim

exe_objs = $(addsuffix .o, $(exes))

srcs =
srcs += tree.hpp tree.cpp
srcs += tsv.hpp tsv.cpp
srcs += utils.hpp utils.cpp
srcs += $(addsuffix .cpp, $(exes))

aux = Makefile

all: $(exes)

Makefile.dep:
	-$(CXX) -MM ./src/*.cpp > $@

-include Makefile.dep

tsvtree: % : %.o tree.o utils.o tsv.o
	$(CXX) -o $@ $^ $(CPPFLAGS) -lboost_program_options

treesim: % : %.o
	$(CXX) -o $@ $^ $(CPPFLAGS)

install: all
	install -D tsvtree --target-directory $(bin_final_dir)

uninstall:
	rm -f $(bin_final_dir)/tsvtree
	rmdir $(DESDIR)$(docdir)

.PHONY: clean
clean:
	rm -f $(exes) $(exe_objs) utils.o tree.o tsv.o $(tarball_name).tar.gz Makefile.dep Makefile.dep
	rm -rf tmp

$(tarball_name).tar.gz:
	git archive --format=tar.gz --prefix=$(tarball_dir)/ HEAD > $(tarball_name).tar.gz

.PHONY: dist
dist: $(tarball_name).tar.gz

.PHONY: deb
deb: dist
	rm -rf tmp; mkdir tmp; mv $(tarball_name).tar.gz tmp; cd tmp; \
	ln $(tarball_name).tar.gz tsvtree_1.0.0.orig.tar.gz; \
	tar -xvvzf $(tarball_name).tar.gz; \
	cd $(tarball_dir)/debian; debuild --no-sign -j1

backup_emails = foo@bar.de

.PHONY: backup
backup: $(tarball_name).tar.gz
	echo "Backup" | mutt -s "Backup" -a $< -- $(backup_emails)

