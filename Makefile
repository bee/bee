
PREFIX=/usr
SBINDIR=${PREFIX}/sbin

DEST=

all:
	@echo "make install"

install:
	@mkdir -vp ${DEST}${SBINDIR}
	@cp -vax beesh bee_install bee_init bee_remove bee_check beefind.pl ${DEST}${SBINDIR}
