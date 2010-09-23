
PREFIX=/usr
SBINDIR=${PREFIX}/sbin

DESTDIR=

SHELLS=bee beeinit beecheck beeremove beeinstall beesh
PERLS=beefind.pl

all: build

build: shells perls

shells: $(SHELLS) $(PERLS)


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
	
perls: beefind.pl

beefind.pl:
	cp src/beefind.pl    beefind.pl

clean:
	rm -f $(SHELLS)
	rm -f $(PERLS)
	

install: build
	@mkdir -vp ${DESTDIR}${SBINDIR}
	@for i in $(SHELLS) $(PERLS) ; do \
	     echo "installing $(DESTDIR)$(SBINDIR)/$${i}" ; \
	     install -m 0755 $${i} ${DESTDIR}${SBINDIR} ; \
	 done
