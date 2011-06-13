
if [ ! -r ${S}/Makefile ] ; then
    return
fi

BEE_BUILDTYPE=make

build_in_sourcedir

#### bee_build() ##############################################################

bee_build() {
    start_cmd make ${DEFCONFIG} $@
}

#### bee_install() ############################################################

bee_install() {
    start_cmd make install ${DEFCONFIG} DESTDIR=${D} $@
}

if [ -z "${DEFCONFIG}" ] ; then
    for var in prefix eprefix bindir sbindir libexecdir sysconfdir \
               sharedstatedir localstatedir libdir includedir \
               datarootdir datadir infodir localedir mandir docdir ; do
        eval eval ${var^^}=\${${var^^}}
        DEFCONFIG="${DEFCONFIG} \${${var^^}:+${var^^}=\${${var^^}}}"
    done
fi

eval DEFCONFIG=\"${DEFCONFIG}\"
