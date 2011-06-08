PREFIX=/usr
EPREFIX=${PREFIX}
SBINDIR=${PREFIX}/sbin
BINDIR=${PREFIX}/bin
LIBEXECDIR=${EPREFIX}/lib/bee
SYSCONFDIR=/etc

BEEDIR=${SYSCONFDIR}/xdg/bee
TEMPLATEDIR=${BEEDIR}/templates

DESTDIR=

SHELLS=bee beesh
TOOLS=bee-init bee-check bee-remove bee-install bee-list bee-query
PERLS=beefind.pl
PROGRAMS=beeversion beesep beecut

TEMPLATES=fallback
CONFIGS=skiplist beerc

.SUFFIXES: .in .sh .sh.in

all: build

build: shells perls programs

perls: $(PERLS)

programs: $(PROGRAMS)

shells: $(addsuffix .sh,$(SHELLS) $(TOOLS))

beesep: src/beesep/beesep.c
	gcc -Wall -o $@ $^

beeversion: src/beeversion/beeversion.c
	gcc -Wall -o $@ $^

beecut: src/beecut/beecut.c
	gcc -Wall -o $@ $^

%.sh: src/%.sh.in
	cp $< $@
	
beefind.pl: src/beefind.pl
	cp $< $@

clean:
	rm -f $(addsuffix .sh,$(SHELLS) $(TOOLS))
	rm -f $(PERLS)
	rm -f $(PROGRAMS)

install: install-core install-config

install-core: build
	@mkdir -vp ${DESTDIR}${BINDIR}
	@for i in $(SHELLS) ; do \
	     install -v -m 0755 $${i}.sh ${DESTDIR}${BINDIR}/$${i} ; \
	 done
	
	@for i in $(PERLS) $(PROGRAMS) ; do \
	     install -v -m 0755 $${i} ${DESTDIR}${BINDIR} ; \
	 done
	@mkdir -vp ${DESTDIR}${LIBEXECDIR}
	@for i in $(TOOLS) ; do \
	     install -v -m 0755 $${i}.sh ${DESTDIR}${LIBEXECDIR}/$${i} ; \
	 done

install-config:
	@mkdir -vp ${DESTDIR}${BEEDIR}
	@for i in ${CONFIGS} ; do \
	     install -v -m 0644 conf/$${i} ${DESTDIR}${BEEDIR}/$${i}.sample; \
	 done
	@mkdir -vp ${DESTDIR}${TEMPLATEDIR}
	@for t in ${TEMPLATES} ; do \
	     install -v -m 0644 conf/templates/$${t} ${DESTDIR}${TEMPLATEDIR} ; \
	 done
