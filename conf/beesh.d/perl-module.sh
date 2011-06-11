
if [ ! -r ${S}/Build.PL ] ; then
    return
fi

BEE_BUILDTYPE=perl

: ${PERL:=perl}

build_in_sourcedir

#### bee_configure() ##########################################################

bee_configure() {
    start_cmd ${PERL} Build.PL \
        --destdir ${D} \
        ${@}
}

#### bee_build() ##############################################################

bee_build() {
    start_cmd ./Build \
        ${@}
}

#### bee_install() ############################################################

bee_install() {
    start_cmd ./Build \
        install \
        ${@}
}
