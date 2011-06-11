
if [ ! -x ${S}/configure ] ; then
    return
fi

BEE_BUILDTYPE=configure

#### bee_configure() ##########################################################

bee_configure() {
    start_cmd ${S}/configure ${DEFCONFIG} $@
}

#### bee_build() ##############################################################

bee_build() {
    start_cmd make $@
}

#### bee_install() ############################################################

bee_install() {
    start_cmd make install DESTDIR=${D} $@
}


# check IGNORE_DATAROOTDIR for compatibility with old bee-files
if [ ${IGNORE_DATAROOTDIR} ] ; then
    print_error "IGNORE_DATAROOTDIR is deprecated! pleade use BEE_CONFIGURE='compat' instead." >&2
    BEE_CONFIGURE='compat'
fi

if [ "${BEE_CONFIGURE}" = "compat" ] ; then
    unset DATAROOTDIR
    unset LOCALEDIR
    unset DOCDIR
fi

### create default configure line
: ${DEFCONFIG:='
--prefix=${PREFIX}
--exec-prefix=${EPREFIX}
--bindir=${BINDIR}
--sbindir=${SBINDIR}
--libexecdir=${LIBEXECDIR}
--sysconfdir=${SYSCONFDIR}
--sharedstatedir=${SHAREDSTATEDIR}
--localstatedir=${LOCALSTATEDIR}
--libdir=${LIBDIR}
--includedir=${INCLUDEDIR}
${DATAROOTDIR:+--datarootdir=${DATAROOTDIR}}
--datadir=${DATADIR}
--infodir=${INFODIR}
${LOCALEDIR:+--localedir=${LOCALEDIR}}
--mandir=${MANDIR}
${DOCDIR:+--docdir=${DOCDIR}}'}

eval DEFCONFIG=\"${DEFCONFIG}\"
