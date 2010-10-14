PREFIX=/usr
SBINDIR=${PREFIX}/sbin
BINDIR=${PREFIX}/bin
SYSCONFDIR=/etc

BEEDIR=${SYSCONFDIR}/bee
TEMPLATEDIR=${BEEDIR}/templates

DESTDIR=

SHELLS=bee bee-init bee-check bee-remove bee-install bee-list beesh
PERLS=beefind.pl
PROGRAMS=beeversion beesep

TEMPLATES=default
CONFIGS=skiplist defaults

all: build

build: shells perls programs

perls: $(PERLS)

programs: $(PROGRAMS)

shells: $(SHELLS)

beesep: src/beesep/beesep.c
	gcc -Wall -o $@ $^

beeversion: src/beeversion/beeversion.c
	gcc -Wall -o $@ $^

bee: src/bee.sh
	cp $< $@

bee-init: src/beeinit.sh
	cp $< $@

bee-check: src/beecheck.sh
	cp $< $@

bee-remove: src/beeremove.sh
	cp $< $@

bee-install: src/beeinstall.sh
	cp $< $@

bee-list: src/beelist.sh
	cp $< $@

beesh: src/beesh.sh
	cp $< $@

beefind.pl: src/beefind.pl
	cp $< $@

clean:
	rm -f $(SHELLS)
	rm -f $(PERLS)
	rm -f $(PROGRAMS)

install: build
	@mkdir -vp ${DESTDIR}${BINDIR}
	@for i in $(SHELLS) $(PERLS) $(PROGRAMS) ; do \
	     echo "installing $(DESTDIR)$(BINDIR)/$${i}" ; \
	     install -m 0755 $${i} ${DESTDIR}${BINDIR} ; \
	 done

install-config:
	@mkdir -vp ${DESTDIR}${BEEDIR}
	@for i in ${CONFIGS} ; do \
	     echo "installing ${DESTDIR}${BEEDIR}/$${i}.sample" ; \
	     install -vm 0660 conf/$${i} ${DESTDIR}${BEEDIR}/$${i}.sample; \
	 done
	@for t in ${TEMPLATES} ; do \
	     echo "installing ${DESTDIR}${TEMPLATEDIR}/$${i}" ; \
	     install -vm 0660 conf/templates/$${i} ${DESTDIR}${TEMPLATEDIR} ; \
	 done
