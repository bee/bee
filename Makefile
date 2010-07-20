
PREFIX=/usr
BINDIR=${PREFIX}/bin

DEST=

all:
	@echo "make install"

install:
	@mkdir -vp ${DEST}${BINDIR}
	@cp -vax iee-alpha bee-alpha beefind.pl ${DEST}${BINDIR}
