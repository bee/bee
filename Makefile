
PREFIX=/usr
SBINDIR=${PREFIX}/sbin

DESTDIR=

all:
	@echo "make install"

install:
	@mkdir -vp ${DESTDIR}${SBINDIR}
	@cp -vax beesh bee_install bee_init bee_remove bee_check beefind.pl ${DESTDIR}${SBINDIR}
