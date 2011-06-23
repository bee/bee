
if [ ! -r ${S}/CMakeLists.txt ] ; then
    return
fi

BEE_BUILDTYPE=cmake

#### bee_configure() ##########################################################

bee_configure() {
    start_cmd cmake \
        -DCMAKE_INSTALL_PREFIX=${PREFIX} \
        "${@}" \
        ${S}
}

#### bee_build() ##############################################################

bee_build() {
    start_cmd make "${@}"
}

#### bee_install() ############################################################

bee_install() {
    start_cmd make install DESTDIR=${D} "${@}"
}

