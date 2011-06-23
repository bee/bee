
if [ ! -r ${S}/Makefile.PL ] ; then
    return
fi

BEE_BUILDTYPE=perl-makemaker

: ${PERL:=perl}

build_in_sourcedir

#### bee_configure() ##########################################################

bee_configure() {
    start_cmd ${PERL} Makefile.PL \
        DESTDIR=${D} \
        "${@}"
}

#### bee_build() ##############################################################

bee_build() {
    start_cmd make \
        "${@}"
}

#### bee_install() ############################################################

bee_install() {
    start_cmd make \
        install \
        "${@}"
}
