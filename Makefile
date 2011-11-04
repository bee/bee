BEE_VERSION = 1.0_rc17

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

PROGRAMS_C=beeversion beesep beecut beeuniq beesort
PROGRAMS_SHELL=bee beesh
PROGRAMS_PERL=beefind.pl

HELPER_BEE_SHELL=bee-init bee-check bee-remove bee-install bee-list bee-query

LIBRARY_SHELL=beelib.config.sh

HELPER_BEESH_SHELL=configure cmake autogen perl-module perl-module-makemaker make python-module

HELPER_HOOKS_SHELL=update-mime-database glib-compile-schemas mkfontdir-mkfontscale gtk-update-icon-cache \
                   ldconfig update-desktop-database gdk-pixbuf-query-loaders mandb systemd-tmpfiles

BEE_MANPAGES=bee.1 bee-check.1 bee-init.1 bee-install.1 bee-list.1 bee-query.1 bee-remove.1

CONFIG_TEMPLATES=fallback
CONFIG_FILES=skiplist beerc

.SUFFIXES: .in .sh .sh.in .pl

all: build

build: shellscripts perlscripts cprograms manpages

SHELLSCRIPTS=$(PROGRAMS_SHELL) $(HELPER_BEE_SHELL)

BEEVERSION_OBJECTS=beeversion.o parse.o compare.o output.o
BEESEP_OBJECTS=beesep.o
BEECUT_OBJECTS=beecut.o
BEEUNIQ_OBJECTS=beeuniq.o
BEESORT_OBJECTS=beesort.o compare.o output.o parse.o tree.o

shellscripts: $(addsuffix .sh,$(SHELLSCRIPTS)) $(LIBRARY_SHELL)
perlscripts:  $(PROGRAMS_PERL)
cprograms:    $(PROGRAMS_C)
manpages:     ${BEE_MANPAGES}

beesep: $(addprefix src/beesep/, ${BEESEP_OBJECTS})
	$(call quiet-command,${CC} ${LDFLAGS} -o $@ $^,"LD	$@")

beeversion: $(addprefix  src/beeversion/, ${BEEVERSION_OBJECTS})
	$(call quiet-command,${CC} ${LDFLAGS} -o $@ $^,"LD	$@")

beecut: $(addprefix src/beecut/, ${BEECUT_OBJECTS})
	$(call quiet-command,${CC} ${LDFLAGS} -o $@ $^,"LD	$@")

beeuniq: $(addprefix src/beeuniq/, ${BEEUNIQ_OBJECTS})
	$(call quiet-command,${CC} ${LDFLAGS} -o $@ $^,"LD	$@")

beesort: $(addprefix src/beeversion/, ${BEESORT_OBJECTS})
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
	@rm -vf $(addsuffix .sh,${SHELLSCRIPTS}) $(LIBRARY_SHELL)
	@rm -vf ${PROGRAMS_PERL}
	@rm -vf ${PROGRAMS_C}
	@rm -vf $(addprefix  src/beeversion/, ${BEEVERSION_OBJECTS})
	@rm -vf $(addprefix  src/beesep/, ${BEESEP_OBJECTS})
	@rm -vf $(addprefix  src/beecut/, ${BEECUT_OBJECTS})
	@rm -vf $(addprefix  src/beeuniq/, ${BEEUNIQ_OBJECTS})
	@rm -vf $(addprefix  src/beeversion/, ${BEESORT_OBJECTS})
	@rm -vf ${BEE_MANPAGES}

install: install-core install-config

install-core: build install-man install-hooks install-buildtypes install-beeshlib install-tools
	@mkdir -p ${DESTDIR}${BINDIR}

	@for i in ${PROGRAMS_SHELL} ; do \
	     echo "installing ${DESTDIR}${BINDIR}/$${i}" ; \
	     install -m 0755 $${i}.sh ${DESTDIR}${BINDIR}/$${i} ; \
	 done

	@for i in ${PROGRAMS_PERL} ${PROGRAMS_C} ; do \
	     echo "installing ${DESTDIR}${BINDIR}/$${i}" ; \
	     install -m 0755 $${i} ${DESTDIR}${BINDIR}/$${i} ; \
	 done

install-tools: $(addprefix ${DESTDIR}${LIBEXECDIR}/bee/bee.d/,${HELPER_BEE_SHELL})

install-dir-tools:
	$(call quiet-installdir,0755,${DESTDIR}${LIBEXECDIR}/bee/bee.d)

${DESTDIR}${LIBEXECDIR}/bee/bee.d/%: %.sh install-dir-tools
	$(call quiet-install,0755,$<,$@)

install-beeshlib: $(addprefix ${DESTDIR}${LIBEXECDIR}/bee/,${LIBRARY_SHELL})

install-dir-beeshlib:
	$(call quiet-installdir,0755,${DESTDIR}${LIBEXECDIR}/bee)

${DESTDIR}${LIBEXECDIR}/bee/%: % install-dir-beeshlib
	$(call quiet-install,0755,$<,$@)

install-buildtypes: $(addsuffix .sh,$(addprefix ${DESTDIR}${LIBEXECDIR}/bee/beesh.d/,${HELPER_BEESH_SHELL}))

install-dir-buildtypes:
	$(call quiet-installdir,0755,${DESTDIR}${LIBEXECDIR}/bee/beesh.d)

${DESTDIR}${LIBEXECDIR}/bee/beesh.d/%.sh: src/beesh.d/%.sh install-dir-buildtypes
	$(call quiet-install,0755,$<,$@)

install-hooks: $(addprefix ${DESTDIR}${LIBEXECDIR}/bee/hooks.d/,${HELPER_HOOKS_SHELL})

install-dir-hookdir:
	$(call quiet-installdir,0755,${DESTDIR}${LIBEXECDIR}/bee/hooks.d)

${DESTDIR}${LIBEXECDIR}/bee/hooks.d/%: src/hooks.d/%.sh install-dir-hookdir
	$(call quiet-install,0755,$<,$@)

install-man: $(addprefix ${DESTDIR}${MANDIR}/man1/,${BEE_MANPAGES})

install-dir-mandir:
	$(call quiet-installdir,0755,${DESTDIR}${MANDIR}/man1)

${DESTDIR}${MANDIR}/man1/%.1: %.1 install-dir-mandir
	$(call quiet-install,0644,$<,$@)

install-config:
	@mkdir -p ${DESTDIR}${DEFCONFDIR}/bee

	@for i in ${CONFIG_FILES} ; do \
	     echo "installing ${DESTDIR}${DEFCONFDIR}/bee/$${i}" ; \
	     install -m 0444 conf/$${i} ${DESTDIR}${DEFCONFDIR}/bee/$${i}; \
	 done

	@mkdir -p ${DESTDIR}${DEFCONFDIR}/bee/templates

	@for i in ${CONFIG_TEMPLATES} ; do \
	     echo "installing ${DESTDIR}${DEFCONFDIR}/bee/templates/$${i}" ; \
	     install -m 0444 conf/templates/$${i} ${DESTDIR}${DEFCONFDIR}/bee/templates/$${i} ; \
	 done
