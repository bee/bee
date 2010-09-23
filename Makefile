
PREFIX=/usr
SBINDIR=${PREFIX}/sbin

DESTDIR=

SHELLS=bee beeinit beecheck beeremove beeinstall beesh
PERLS=beefind.pl
PROGRAMS=beeversion beesep

all: build

build: shells perls programs

perls: $(PERLS)

programs: $(PROGRAMS)

shells: $(SHELLS)

beesep: src/beesep/beesep.c
	gcc -Wall -o beesep src/beesep/beesep.c

beeversion: src/beeversion/beeversion.c
	gcc -Wall -o beeversion src/beeversion/beeversion.c

bee:
	cp src/bee.sh        bee

beeinit:
	cp src/beeinit.sh    beeinit

beecheck:
	cp src/beecheck.sh   beecheck

beeremove:
	cp src/beeremove.sh  beeremove

beeinstall:
	cp src/beeinstall.sh beeinstall

beesh:
	cp src/beesh.sh      beesh
	

beefind.pl:
	cp src/beefind.pl    beefind.pl

clean:
	rm -f $(SHELLS)
	rm -f $(PERLS)
	rm -f $(PROGRAMS)
	

install: build
	@mkdir -vp ${DESTDIR}${SBINDIR}
	@for i in $(SHELLS) $(PERLS) $(PROGRAMS) ; do \
	     echo "installing $(DESTDIR)$(SBINDIR)/$${i}" ; \
	     install -m 0755 $${i} ${DESTDIR}${SBINDIR} ; \
	 done
