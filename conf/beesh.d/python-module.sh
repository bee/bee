
if [ ! -r ${S}/setup.py ] ; then
    return
fi

BEE_BUILDTYPE=python-module

: ${PYTHON:=python}

build_in_sourcedir

#### bee_build() ##############################################################

bee_build() {
    start_cmd ${PYTHON} setup.py build $@
}

#### bee_install() ############################################################

bee_install() {
    start_cmd ${PYTHON} setup.py install --root=${D} $@
}
