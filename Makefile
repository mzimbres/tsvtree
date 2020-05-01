pkg_name = tsvtree
pkg_version = 1.0.3
tarball = $(pkg_name)-$(pkg_version)
prefix = /usr
datarootdir = $(prefix)/share
datadir = $(datarootdir)
docdir = $(datadir)/doc/$(pkg_name)
bindir = $(prefix)/bin
srcdir = .

bin_final_dir = $(DESTDIR)$(bindir)
doc_final_dir = $(DESTDIR)$(docdir)
man_final_dir = $(DESTDIR)$(datarootdir)/man

VPATH = ./src

CPPFLAGS += -std=c++17 -Wall -Werror
CPPFLAGS += -I$./src
CPPFLAGS += $(CXXFLAGS)

objs = tree.o tsv.o utils.o tsvtree.o
aux = Makefile

all: tsvtree tsvsim

Makefile.dep:
	-$(CXX) -MM ./src/*.cpp > $@

-include Makefile.dep

.PHONY: version
version:
	echo "#pragma once\nchar const* version = \"$(pkg_version)\";" > src/version.hpp

tsvtree: $(objs)
	$(CXX) -o $@ $(objs) $(CPPFLAGS) -lfmt -lboost_program_options $(LDFLAGS)

tsvsim: % : %.o
	$(CXX) -o $@ $^ $(CPPFLAGS) $(LDFLAGS)

.PHONY: check
check:
	./src/check.sh

install: all
	install -D tsvtree --target-directory $(bin_final_dir)
	install --mode=664 -D examples/cities.tsv --target-directory $(doc_final_dir)/examples
	install --mode=444 -D doc/$(pkg_name).1 --target-directory $(man_final_dir)/man1

uninstall:
	rm -f $(bin_final_dir)/tsvtree
	rm -f $(man_final_dir)/man1/$(pkg_name).1
	rm -rf $(doc_final_dir)

.PHONY: clean
clean:
	rm -f tsvtree tsvsim tsvsim.o $(objs) $(tarball).tar.gz Makefile.dep Makefile.dep
	rm -rf tmp


.PHONY: dist
dist:
	git archive --format=tar.gz --prefix=$(tarball)/ HEAD > $(tarball).tar.gz

backup_emails = foo@bar.de

.PHONY: backup
backup: $(tarball).tar.gz
	echo "Backup" | mutt -s "Backup" -a $< -- $(backup_emails)

