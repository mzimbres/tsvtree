pkg_name = tsvtree
pkg_version = 1.0.0
debian_version = 1
tarball_name = $(pkg_name)-$(pkg_version)-$(debian_version)
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

objs = tree.o tsv.o utils.o tsvtree.o
aux = Makefile

all: tsvtree treesim

Makefile.dep:
	-$(CXX) -MM ./src/*.cpp > $@

-include Makefile.dep

.PHONY: version
version:
	echo "#pragma once\nchar const* version = \"$(pkg_version)\";" > src/version.hpp

tsvtree: $(objs) version
	$(CXX) -o $@ $(objs) $(CPPFLAGS) -lboost_program_options

treesim: % : %.o
	$(CXX) -o $@ $^ $(CPPFLAGS)

install: all
	install -D tsvtree --target-directory $(bin_final_dir)

uninstall:
	rm -f $(bin_final_dir)/tsvtree
	rmdir $(doc_final_dir)

.PHONY: clean
clean:
	rm -f tsvtree treesim treesim.o $(objs) $(tarball_name).tar.gz Makefile.dep Makefile.dep
	rm -rf tmp

$(tarball_name).tar.gz:
	git archive --format=tar.gz --prefix=$(tarball_dir)/ HEAD > $(tarball_name).tar.gz

.PHONY: dist
dist: $(tarball_name).tar.gz

# Add --no-sign to avoid signing the the package.
.PHONY: deb
deb: dist
	rm -rf tmp; mkdir tmp; mv $(tarball_name).tar.gz tmp; cd tmp; \
	ln $(tarball_name).tar.gz tsvtree_1.0.0.orig.tar.gz; \
	tar -xvvzf $(tarball_name).tar.gz; \
	cd $(tarball_dir)/debian; debuild -j1 

backup_emails = foo@bar.de

.PHONY: backup
backup: $(tarball_name).tar.gz
	echo "Backup" | mutt -s "Backup" -a $< -- $(backup_emails)

