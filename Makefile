BEE_VERSION = 1.0_rc29

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

sed-compat-bashlt4 = -e 'sx\$${\([a-zA-Z_]*\),,}x\$$(tr A-Z a-z <<<\$${\1})xg' \
                     -e 'sx\$${\([a-zA-Z_]*\)^^}x\$$(tr a-z A-Z <<<\$${\1})xg'


PROGRAMS_C=beeversion beesep beecut beeuniq beesort beegetopt
PROGRAMS_SHELL=bee beesh
PROGRAMS_PERL=beefind.pl

HELPER_BEE_SHELL=bee-init bee-check bee-remove bee-install bee-list bee-query bee-download
HELPER_BEE_C=bee-dep

LIBRARY_SHELL=beelib.config.sh

HELPER_BEESH_SHELL=configure cmake autogen perl-module perl-module-makemaker make python-module

HELPER_HOOKS_SHELL=update-mime-database glib-compile-schemas mkfontdir-mkfontscale gtk-update-icon-cache \
                   ldconfig update-desktop-database gdk-pixbuf-query-loaders mandb systemd-tmpfiles

BEE_MANPAGES=bee.1 bee-check.1 bee-init.1 bee-install.1 bee-list.1 bee-query.1 bee-remove.1 bee-dep.1

CONFIG_TEMPLATES=fallback
CONFIG_FILES=skiplist beerc

COMPAT_BASHLT4=buildtypes/autogen.sh buildtypes/configure.sh buildtypes/make.sh \
               src/beesh.sh.in src/bee-check.sh.in

.SUFFIXES: .in .sh .sh.in .pl

all: build

build: shellscripts perlscripts cprograms manpages

compat: compat-bashlt4

compat-bashlt4:
	$(call quiet-command, sed ${sed-compat-bashlt4} -i ${COMPAT_BASHLT4}, "SED	$@" )

SHELLSCRIPTS=$(PROGRAMS_SHELL) $(HELPER_BEE_SHELL)

BEEVERSION_OBJECTS=beeversion.o bee_version_parse.o bee_version_compare.o bee_version_output.o
BEESEP_OBJECTS=beesep.o
BEECUT_OBJECTS=beecut.o
BEEUNIQ_OBJECTS=beeuniq.o
BEESORT_OBJECTS=bee_tree.o bee_version_compare.o bee_version_output.o bee_version_parse.o bee_getopt.o beesort.o
BEEDEP_OBJECTS=bee-dep.o graph.o hash.o beedep_tree.o node.o
BEEGETOPT_OBJECTS=bee_getopt.o beegetopt.o

shellscripts: $(addsuffix .sh,$(SHELLSCRIPTS)) $(LIBRARY_SHELL)
perlscripts:  $(PROGRAMS_PERL)
cprograms:    $(PROGRAMS_C) ${HELPER_BEE_C}
manpages:     ${BEE_MANPAGES}

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

bee-dep: $(addprefix src/, ${BEEDEP_OBJECTS})
	$(call quiet-command,${CC} ${LDFLAGS} -o $@ $^,"LD	$@")

beegetopt: $(addprefix src/, ${BEEGETOPT_OBJECTS})
	$(call quiet-command,${CC} ${LDFLAGS} -o $@ $^,"LD	$@")

%.o: %.c
	$(call quiet-command,${CC} ${CFLAGS} -o $@ -c $^,"CC	$@")

%.sh: src/%.sh.in
	$(call quiet-command,sed ${sed-rules} $< >$@,"SED	$@")

%.pl: src/%.pl
	$(call quiet-command,cp $< $@,"CP	$@")

%.1: manpages/%.1.in
	$(call quiet-command,sed ${sed-rules} $< >$@,"SED	$@")

clean:
	$(call quiet-command,rm -f $(addsuffix .sh,${SHELLSCRIPTS}) $(LIBRARY_SHELL),"RM	<various>.sh")
	$(call quiet-command,rm -f ${PROGRAMS_PERL},"RM	${PROGRAMS_PERL}")
	$(call quiet-command,rm -f ${PROGRAMS_C},"RM	${PROGRAMS_C}")
	$(call quiet-command,rm -f ${HELPER_BEE_C},"RM	${HELPER_BEE_C}")
	$(call quiet-command,rm -f src/*.o,"RM	src/*.o")
	$(call quiet-command,rm -f ${BEE_MANPAGES},"RM	${BEE_MANPAGES}")

install: install-core install-config

install-core: build install-man install-hooks install-buildtypes install-beeshlib install-tools install-bin

install-bin: $(addprefix ${DESTDIR}${BINDIR}/,${PROGRAMS_PERL} ${PROGRAMS_C} ${PROGRAMS_SHELL})

install-dir-bindir:
	$(call quiet-installdir,0755,${DESTDIR}${BINDIR})

${DESTDIR}${BINDIR}/%: % install-dir-bindir
	$(call quiet-install,0755,$<,$@)

${DESTDIR}${BINDIR}/%: %.sh install-dir-bindir
	$(call quiet-install,0755,$<,$@)

install-tools: $(addprefix ${DESTDIR}${LIBEXECDIR}/bee/bee.d/,${HELPER_BEE_SHELL} ${HELPER_BEE_C})

install-dir-tools:
	$(call quiet-installdir,0755,${DESTDIR}${LIBEXECDIR}/bee/bee.d)

${DESTDIR}${LIBEXECDIR}/bee/bee.d/%: %.sh install-dir-tools
	$(call quiet-install,0755,$<,$@)

${DESTDIR}${LIBEXECDIR}/bee/bee.d/%: % install-dir-tools
	$(call quiet-install,0755,$<,$@)

install-beeshlib: $(addprefix ${DESTDIR}${LIBEXECDIR}/bee/,${LIBRARY_SHELL})

install-dir-beeshlib:
	$(call quiet-installdir,0755,${DESTDIR}${LIBEXECDIR}/bee)

${DESTDIR}${LIBEXECDIR}/bee/%: % install-dir-beeshlib
	$(call quiet-install,0755,$<,$@)

install-buildtypes: $(addsuffix .sh,$(addprefix ${DESTDIR}${LIBEXECDIR}/bee/beesh.d/,${HELPER_BEESH_SHELL}))

install-dir-buildtypes:
	$(call quiet-installdir,0755,${DESTDIR}${LIBEXECDIR}/bee/beesh.d)

${DESTDIR}${LIBEXECDIR}/bee/beesh.d/%.sh: buildtypes/%.sh install-dir-buildtypes
	$(call quiet-install,0755,$<,$@)

install-hooks: $(addprefix ${DESTDIR}${LIBEXECDIR}/bee/hooks.d/,${HELPER_HOOKS_SHELL})

install-dir-hookdir:
	$(call quiet-installdir,0755,${DESTDIR}${LIBEXECDIR}/bee/hooks.d)

${DESTDIR}${LIBEXECDIR}/bee/hooks.d/%: hooks/%.sh install-dir-hookdir
	$(call quiet-install,0755,$<,$@)

install-man: $(addprefix ${DESTDIR}${MANDIR}/man1/,${BEE_MANPAGES})

install-dir-mandir:
	$(call quiet-installdir,0755,${DESTDIR}${MANDIR}/man1)

${DESTDIR}${MANDIR}/man1/%.1: %.1 install-dir-mandir
	$(call quiet-install,0644,$<,$@)

install-dir-config:
	$(call quiet-installdir,0755,${DESTDIR}${DEFCONFDIR}/bee/templates)

install-config: install-config-defaults install-config-templates

install-config-defaults: $(addprefix ${DESTDIR}${DEFCONFDIR}/bee/,${CONFIG_FILES})

install-config-templates: $(addprefix ${DESTDIR}${DEFCONFDIR}/bee/templates/,${CONFIG_TEMPLATES})

${DESTDIR}${DEFCONFDIR}/bee/%: conf/% install-dir-config
	$(call quiet-install,0444,$<,$@)
