

if [ ! -r ${S}/Makefile ] ; then
    return
fi

BEE_BUILDTYPE=make

build_in_sourcedir

#### bee_build() ##############################################################

bee_build() {
    start_cmd make PREFIX=${PREFIX} $@
}

#### bee_install() ############################################################

bee_install() {
    start_cmd make install PREFIX=${PREFIX} DESTDIR=${D} $@
}

