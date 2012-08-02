BEE_VERSION = 1.1.99

CC=gcc
CFLAGS=-Wall -g
LDFLAGS=

PREFIX     = /usr
EPREFIX    = ${PREFIX}
SBINDIR    = ${EPREFIX}/sbin
BINDIR     = ${EPREFIX}/bin
LIBDIR     = ${EPREFIX}/lib
LIBEXECDIR = ${EPREFIX}/libexec
DATADIR    = ${PREFIX}/share
MANDIR     = ${DATADIR}/man
SYSCONFDIR = ${PREFIX}/etc

# set sysconfdir /etc if prefix /usr
ifeq (${PREFIX},/usr)
    SYSCONFDIR = /etc
endif

# set sysconfdir /etc if prefix /usr/local
ifeq (${PREFIX},/usr/local)
    SYSCONFDIR = /etc
endif

# strip /bee from LIBEXECDIR if set
ifeq ($(notdir ${LIBEXECDIR}),bee)
    override LIBEXECDIR := $(patsubst %/,%,$(dir ${LIBEXECDIR}))
endif

# default config directory
DEFCONFDIR=${SYSCONFDIR}/default

DESTDIR=

quiet-command = $(if ${V},${1},$(if ${2},@echo ${2} && ${1}, @${1}))
quiet-install = $(call quiet-command,install -m ${1} ${2} ${3},"INSTALL	${3}")
quiet-installdir = $(call quiet-command,install -m ${1} -d ${2},"MKDIR	${2}")

sed-rules = -e 's,@PREFIX@,${PREFIX},g' \
	    -e 's,@EPREFIX@,${EPREFIX},g' \
	    -e 's,@BINDIR@,${BINDIR},g' \
	    -e 's,@SBINDIR@,${SBINDIR},g' \
	    -e 's,@LIBDIR@,${LIBDIR},g' \
	    -e 's,@SYSCONFDIR@,${SYSCONFDIR},g' \
	    -e 's,@DEFCONFDIR@,${DEFCONFDIR},g' \
	    -e 's,@LIBEXECDIR@,${LIBEXECDIR},g' \
	    -e 's,@BEE_VERSION@,${BEE_VERSION},g' \
	    -e 's,@DATADIR@,${DATADIR},g'

PROGRAMS_C+=beecut
PROGRAMS_C+=beeflock
PROGRAMS_C+=beegetopt
PROGRAMS_C+=beesep
PROGRAMS_C+=beesort
PROGRAMS_C+=beeuniq
PROGRAMS_C+=beeversion

PROGRAMS_SHELL+=bee
PROGRAMS_SHELL+=beefind
PROGRAMS_SHELL+=beesh

HELPER_BEE_SHELL+=bee-cache
HELPER_BEE_SHELL+=bee-check
HELPER_BEE_SHELL+=bee-download
HELPER_BEE_SHELL+=bee-init
HELPER_BEE_SHELL+=bee-install
HELPER_BEE_SHELL+=bee-list
HELPER_BEE_SHELL+=bee-query
HELPER_BEE_SHELL+=bee-remove
HELPER_BEE_SHELL+=bee-update

HELPER_C+=bee-cache-inventory

HELPER_SHELL+=compat-filesfile2contentfile
HELPER_SHELL+=compat-fixmetadir
HELPER_SHELL+=content2filelist
HELPER_SHELL+=filelist2content
HELPER_SHELL+=bee-cache-update

LIBRARY_SHELL+=beelib.config.sh

BUILDTYPES+=autogen
BUILDTYPES+=cmake
BUILDTYPES+=configure
BUILDTYPES+=jb
BUILDTYPES+=make
BUILDTYPES+=perl-module
BUILDTYPES+=perl-module-makemaker
BUILDTYPES+=python-module

HELPER_HOOKS_SHELL+=update-mime-database
HELPER_HOOKS_SHELL+=glib-compile-schemas
HELPER_HOOKS_SHELL+=mkfontdir-mkfontscale
HELPER_HOOKS_SHELL+=gtk-update-icon-cache
HELPER_HOOKS_SHELL+=ldconfig
HELPER_HOOKS_SHELL+=update-desktop-database
HELPER_HOOKS_SHELL+=gdk-pixbuf-query-loaders
HELPER_HOOKS_SHELL+=mandb
HELPER_HOOKS_SHELL+=systemd-tmpfiles
HELPER_HOOKS_SHELL+=gconf-install-schemas

MANPAGES+=bee.1
MANPAGES+=bee-check.1
MANPAGES+=bee-download.1
MANPAGES+=bee-list.1
MANPAGES+=bee-init.1
MANPAGES+=bee-install.1
MANPAGES+=bee-query.1
MANPAGES+=bee-remove.1
MANPAGES+=bee-update.1

CONFIG_TEMPLATES+=fallback

CONFIG_FILES+=skiplist
CONFIG_FILES+=beerc

.SUFFIXES: .in .sh .sh.in

all: build

build: shellscripts buildtypes cprograms manpages

SHELLSCRIPTS=$(PROGRAMS_SHELL) $(HELPER_BEE_SHELL) $(HELPER_SHELL)

BEEVERSION_OBJECTS=beeversion.o bee_version_parse.o bee_version_compare.o bee_version_output.o
BEESEP_OBJECTS=beesep.o
BEECUT_OBJECTS=beecut.o
BEEUNIQ_OBJECTS=beeuniq.o
BEESORT_OBJECTS=bee_tree.o bee_version_compare.o bee_version_output.o bee_version_parse.o bee_getopt.o beesort.o
BEEGETOPT_OBJECTS=bee_getopt.o beegetopt.o
BEEFLOCK_OBJECTS=bee_getopt.o beeflock.o
BEECACHEINVENTORY_OBJECTS=bee-cache-inventory.o bee_getopt.o

bee_MANPAGES=$(addprefix manpages/,${MANPAGES})
bee_BUILDTYPES=$(addsuffix .sh,$(addprefix buildtypes/,$(BUILDTYPES)))

shellscripts: $(addsuffix .sh,$(SHELLSCRIPTS)) $(LIBRARY_SHELL)
cprograms:    $(PROGRAMS_C) ${HELPER_C}
manpages:     ${bee_MANPAGES}
buildtypes:   ${bee_BUILDTYPES}

beesep: $(addprefix src/, ${BEESEP_OBJECTS})
	$(call quiet-command,${CC} ${LDFLAGS} -o $@ $^,"LD	$@")

beeversion: $(addprefix  src/, ${BEEVERSION_OBJECTS})
	$(call quiet-command,${CC} ${LDFLAGS} -o $@ $^,"LD	$@")

beecut: $(addprefix src/, ${BEECUT_OBJECTS})
	$(call quiet-command,${CC} ${LDFLAGS} -o $@ $^,"LD	$@")

beeuniq: $(addprefix src/, ${BEEUNIQ_OBJECTS})
	$(call quiet-command,${CC} ${LDFLAGS} -o $@ $^,"LD	$@")

beesort: $(addprefix src/, ${BEESORT_OBJECTS})
	$(call quiet-command,${CC} ${LDFLAGS} -o $@ $^,"LD	$@")

beegetopt: $(addprefix src/, ${BEEGETOPT_OBJECTS})
	$(call quiet-command,${CC} ${LDFLAGS} -o $@ $^,"LD	$@")

beeflock: $(addprefix src/, ${BEEFLOCK_OBJECTS})
	$(call quiet-command,${CC} ${LDFLAGS} -o $@ $^,"LD	$@")

bee-cache-inventory: $(addprefix src/, ${BEECACHEINVENTORY_OBJECTS})
	$(call quiet-command,${CC} ${LDFLAGS} -o $@ $^,"LD	$@")

%.o: %.c
	$(call quiet-command,${CC} ${CFLAGS} -o $@ -c $^,"CC	$@")

%.sh: src/%.sh.in
	$(call quiet-command,sed ${sed-rules} $< >$@,"SED	$@")

%.1: %.1.in
	$(call quiet-command,sed ${sed-rules} $< >$@,"SED	$@")

%.sh: %.sh.in
	$(call quiet-command,sed ${sed-rules} $< >$@,"SED	$@")

clean:
	$(call quiet-command,rm -f $(addsuffix .sh,${SHELLSCRIPTS}) $(LIBRARY_SHELL) $(HELPER_SHELL),"CLEAN	<various>.sh")
	$(call quiet-command,rm -f ${PROGRAMS_C},"CLEAN	${PROGRAMS_C}")
	$(call quiet-command,rm -f ${HELPER_C},"CLEAN	${HELPER_C}")
	$(call quiet-command,rm -f src/*.o,"CLEAN	c object files")
	$(call quiet-command,rm -f ${bee_MANPAGES},"CLEAN	manpages")
	$(call quiet-command,rm -f ${bee_BUILDTYPES},"CLEAN	buildtypes")

install: install-core install-config

install-core: build install-man install-hooks install-buildtypes install-beeshlib install-tools install-helper install-bin

install-bin: $(addprefix ${DESTDIR}${BINDIR}/,${PROGRAMS_C} ${PROGRAMS_SHELL})

install-dir-bindir:
	$(call quiet-installdir,0755,${DESTDIR}${BINDIR})

${DESTDIR}${BINDIR}/%: % install-dir-bindir
	$(call quiet-install,0755,$<,$@)

${DESTDIR}${BINDIR}/%: %.sh install-dir-bindir
	$(call quiet-install,0755,$<,$@)

install-tools: $(addprefix ${DESTDIR}${LIBEXECDIR}/bee/bee.d/,${HELPER_BEE_SHELL})

install-dir-tools:
	$(call quiet-installdir,0755,${DESTDIR}${LIBEXECDIR}/bee/bee.d)

${DESTDIR}${LIBEXECDIR}/bee/bee.d/%: %.sh install-dir-tools
	$(call quiet-install,0755,$<,$@)

${DESTDIR}${LIBEXECDIR}/bee/bee.d/%: % install-dir-tools
	$(call quiet-install,0755,$<,$@)

install-helper: $(addprefix ${DESTDIR}${LIBEXECDIR}/bee/,${HELPER_SHELL} ${HELPER_C})

install-dir-helper:
	$(call quiet-installdir,0755,${DESTDIR}${LIBEXECDIR}/bee)

${DESTDIR}${LIBEXECDIR}/bee/%: %.sh install-dir-helper
	$(call quiet-install,0755,$<,$@)

${DESTDIR}${LIBEXECDIR}/bee/%: % install-dir-helper
	$(call quiet-install,0755,$<,$@)

install-beeshlib: $(addprefix ${DESTDIR}${LIBEXECDIR}/bee/,${LIBRARY_SHELL})

install-dir-beeshlib:
	$(call quiet-installdir,0755,${DESTDIR}${LIBEXECDIR}/bee)

${DESTDIR}${LIBEXECDIR}/bee/%: % install-dir-beeshlib
	$(call quiet-install,0755,$<,$@)

install-buildtypes: $(addsuffix .sh,$(addprefix ${DESTDIR}${LIBEXECDIR}/bee/beesh.d/,${BUILDTYPES}))

install-dir-buildtypes:
	$(call quiet-installdir,0755,${DESTDIR}${LIBEXECDIR}/bee/beesh.d)

${DESTDIR}${LIBEXECDIR}/bee/beesh.d/%.sh: buildtypes/%.sh install-dir-buildtypes
	$(call quiet-install,0755,$<,$@)

install-hooks: $(addprefix ${DESTDIR}${LIBEXECDIR}/bee/hooks.d/,${HELPER_HOOKS_SHELL})

install-dir-hookdir:
	$(call quiet-installdir,0755,${DESTDIR}${LIBEXECDIR}/bee/hooks.d)

${DESTDIR}${LIBEXECDIR}/bee/hooks.d/%: hooks/%.sh install-dir-hookdir
	$(call quiet-install,0755,$<,$@)

install-man: $(addprefix ${DESTDIR}${MANDIR}/man1/,${MANPAGES})

install-dir-mandir:
	$(call quiet-installdir,0755,${DESTDIR}${MANDIR}/man1)

${DESTDIR}${MANDIR}/man1/%.1: manpages/%.1 install-dir-mandir
	$(call quiet-install,0644,$<,$@)

install-dir-config:
	$(call quiet-installdir,0755,${DESTDIR}${DEFCONFDIR}/bee/templates)

install-config: install-config-defaults install-config-templates

install-config-defaults: $(addprefix ${DESTDIR}${DEFCONFDIR}/bee/,${CONFIG_FILES})

install-config-templates: $(addprefix ${DESTDIR}${DEFCONFDIR}/bee/templates/,${CONFIG_TEMPLATES})

${DESTDIR}${DEFCONFDIR}/bee/%: conf/% install-dir-config
	$(call quiet-install,0444,$<,$@)
