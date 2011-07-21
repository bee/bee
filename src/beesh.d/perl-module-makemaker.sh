#
# bee magic for perl MakeMaker
#
# Copyright (C) 2009-2011
#       Marius Tolzmann <tolzmann@molgen.mpg.de>
#       Tobias Dreyer <dreyer@molgen.mpg.de>
#       and other bee developers
#
# This file is part of bee.
#
# bee is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.
#

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
