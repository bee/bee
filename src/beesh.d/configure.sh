
if [ ! -x ${S}/configure ] ; then
    return
fi

BEE_BUILDTYPE=configure

#### bee_configure() ##########################################################

bee_configure() {
    start_cmd ${S}/configure ${DEFCONFIG} "$@"
}

#### bee_build() ##############################################################

bee_build() {
    start_cmd make "$@"
}

#### bee_install() ############################################################

bee_install() {
    start_cmd make install DESTDIR=${D} "$@"
}

if ! grep -q datarootdir ${S}/configure ; then
    BEE_CONFIGURE='compat'
fi

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

# create default configure options
if [ -z "${DEFCONFIG}" ] ; then
    for var in prefix bindir sbindir libexecdir sysconfdir \
               sharedstatedir localstatedir libdir includedir \
               datarootdir datadir infodir localedir mandir docdir ; do
        DEFCONFIG="${DEFCONFIG} \${${var^^}:+--${var,,}=\${${var^^}}}"
    done
    DEFCONFIG="${DEFCONFIG} \${EPREFIX:+--exec-prefix=\${EPREFIX}}"
fi

# expand default confidure options
eval DEFCONFIG=\"${DEFCONFIG}\"
