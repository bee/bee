BEE_VERSION = 1.0_rc10

PREFIX     = /usr
EPREFIX    = ${PREFIX}
SBINDIR    = ${EPREFIX}/sbin
BINDIR     = ${EPREFIX}/bin
LIBDIR     = ${EPREFIX}/lib
LIBEXECDIR = ${EPREFIX}/libexec
DATADIR    = ${PREFIX}/share
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

PROGRAMS_C=beeversion beesep beecut
PROGRAMS_SHELL=bee beesh
PROGRAMS_PERL=beefind.pl

HELPER_BEE_SHELL=bee-init bee-check bee-remove bee-install bee-list bee-query

LIBRARY_SHELL=beelib.config

HELPER_BEESH_SHELL=configure cmake autogen perl-module perl-module-makemaker make python-module

HELPER_HOOKS_SHELL=update-mime-database glib-compile-schemas mkfontdir-mkfontscale gtk-update-icon-cache \
                   ldconfig update-desktop-database

CONFIG_TEMPLATES=fallback
CONFIG_FILES=skiplist beerc

.SUFFIXES: .in .sh .sh.in .pl

all: build

build: shellscripts perlscripts cprograms

SHELLSCRIPTS=$(PROGRAMS_SHELL) $(HELPER_BEE_SHELL) $(LIBRARY_SHELL)

BEEVERSION_OBJECTS=beeversion.o parse.o compare.o

shellscripts: $(addsuffix .sh,$(SHELLSCRIPTS))
perlscripts:  $(PROGRAMS_PERL)
cprograms:    $(PROGRAMS_C)

beesep: src/beesep/beesep.c
	@echo "linking $@ .."
	@gcc -Wall -o $@ $^

beeversion: $(addprefix  src/beeversion/, ${BEEVERSION_OBJECTS})
	@echo "linking $@ .."
	@gcc -Wall -o $@ $^

beecut: src/beecut/beecut.c
	@echo "linking $@ .."
	@gcc -Wall -o $@ $^

%.o: %.c
	@echo "compiling $@ .."
	@gcc -Wall -o $@ -c $^

%.sh: src/%.sh.in
	@echo "creating $@ .."
	@sed \
	    -e 's,@PREFIX@,${PREFIX},g' \
	    -e 's,@EPREFIX@,${EPREFIX},g' \
	    -e 's,@BINDIR@,${BINDIR},g' \
	    -e 's,@SBINDIR@,${SBINDIR},g' \
	    -e 's,@LIBDIR@,${LIBDIR},g' \
	    -e 's,@SYSCONFDIR@,${SYSCONFDIR},g' \
	    -e 's,@DEFCONFDIR@,${DEFCONFDIR},g' \
	    -e 's,@LIBEXECDIR@,${LIBEXECDIR},g' \
	    -e 's,@BEE_VERSION@,${BEE_VERSION},g' \
	    -e 's,@DATADIR@,${DATADIR},g' \
	    $< > $@

%.pl: src/%.pl
	@echo "creating $@ .."
	@cp $< $@

clean:
	@rm -vf $(addsuffix .sh,${SHELLSCRIPTS})
	@rm -vf ${PROGRAMS_PERL}
	@rm -vf ${PROGRAMS_C}
	@rm -vf $(addprefix  src/beeversion/, ${BEEVERSION_OBJECTS})

install: install-core install-config

install-core: build
	@mkdir -p ${DESTDIR}${BINDIR}

	@for i in ${PROGRAMS_SHELL} ; do \
	     echo "installing ${DESTDIR}${BINDIR}/$${i}" ; \
	     install -m 0755 $${i}.sh ${DESTDIR}${BINDIR}/$${i} ; \
	 done

	@for i in ${PROGRAMS_PERL} ${PROGRAMS_C} ; do \
	     echo "installing ${DESTDIR}${BINDIR}/$${i}" ; \
	     install -m 0755 $${i} ${DESTDIR}${BINDIR}/$${i} ; \
	 done

	@mkdir -p ${DESTDIR}${LIBEXECDIR}/bee/bee.d

	@for i in ${HELPER_BEE_SHELL} ; do \
	     echo "installing ${DESTDIR}${LIBEXECDIR}/bee/bee.d/$${i}" ; \
	     install -m 0755 $${i}.sh ${DESTDIR}${LIBEXECDIR}/bee/bee.d/$${i} ; \
	 done

	@for i in ${LIBRARY_SHELL} ; do \
	     echo "installing ${DESTDIR}${LIBEXECDIR}/bee/$${i}.sh" ; \
	     install -m 0755 $${i}.sh ${DESTDIR}${LIBEXECDIR}/bee/$${i}.sh ; \
	 done

	@mkdir -p ${DESTDIR}${LIBEXECDIR}/bee/beesh.d
	@for i in ${HELPER_BEESH_SHELL} ; do \
	     echo "installing ${DESTDIR}${LIBEXECDIR}/bee/beesh.d/$${i}.sh" ; \
	     install -m 0644 src/beesh.d/$${i}.sh ${DESTDIR}${LIBEXECDIR}/bee/beesh.d/$${i}.sh ; \
	 done

	@mkdir -p ${DESTDIR}${LIBEXECDIR}/bee/hooks.d
	@for i in ${HELPER_HOOKS_SHELL} ; do \
	     echo "installing ${DESTDIR}${LIBEXECDIR}/bee/hooks.d/$${i}.sh" ; \
	     install -m 0755 src/hooks.d/$${i}.sh ${DESTDIR}${LIBEXECDIR}/bee/hooks.d/$${i}.sh ; \
	 done

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
