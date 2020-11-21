pkg_name = tsvtree
pkg_version = 1.0.5
pkg_revision = 1
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
man_final_dir = $(DESTDIR)$(datarootdir)/man

VPATH = ./src

CPPFLAGS += -std=c++17 -Wall -Werror
CPPFLAGS += -I$./src
CPPFLAGS += -g $(CXXFLAGS)

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
	rm -f tsvtree tsvsim tsvsim.o $(objs) $(tarball_dir).tar.gz Makefile.dep
	rm -rf tmp

$(tarball_name).tar.gz:
	git archive --format=tar.gz --prefix=$(tarball_dir)/ HEAD > $(tarball_name).tar.gz

.PHONY: dist
dist: $(tarball_name).tar.gz

.PHONY: deb
deb: dist
	rm -rf tmp; mkdir tmp; mv $(tarball_name).tar.gz tmp; cd tmp; \
	ln $(tarball_name).tar.gz $(pkg_name)_$(pkg_version).orig.tar.gz; \
	tar -xvvzf $(tarball_name).tar.gz; \
	cd $(tarball_dir)/debian; debuild --no-sign -j1

backup_emails = foo@bar.de

.PHONY: backup
backup: $(tarball_dir).tar.gz
	echo "Backup" | mutt -s "Backup" -a $< -- $(backup_emails)

