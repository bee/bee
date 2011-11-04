#/bin/bash
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


# beeversion
# exit-codes:
#  254 argument syntax error
#  253 pkg syntax error

#  250 BUG FOUND

#    2 usage 
#    5 pkgnames do not match


#  -lt|gt|le|ge|eq|ne 
#    0 true
#    1 false

#  -min|max
#    0 true




echo -n "testing usage .. "
if ! ./beeversion >/dev/null ; then
    echo "OK"
else
    echo "FAILED"
fi


for p in --lt --gt --le --ge --eq --ne ; do 
   echo -n "testing fail when no args in ${p} .. "
   if ! ./beeversion ${p} >/dev/null ; then
       echo "OK"
   else
       echo "FAILED"
   fi
   
   echo -n "testing fail when no args in ${p} .. "
   if ! ./beeversion ${p} dummy-1.2-3 >/dev/null ; then
       echo "OK"
   else
       echo "FAILED"
   fi
done

expect_success() {
    args=${1}
    str=${2-$1}
    
    echo -n "testing $str .. "
    ./beeversion $args
    
    if [ $? -eq 0 ] ; then
        echo "PASSED"
    else
        echo "FAILED"
    fi
}

expect_failure() {
    args=${1}
    str=${2-$1}
    
    echo -n "testing $str .. "
    ./beeversion $args
    
    if [ $? -eq 1 ] ; then
        echo "PASSED"
    else
        echo "FAILED"
    fi
}


lt() {
    expect_success "$1 --lt $2"
    expect_failure "$2 --lt $1"
    expect_failure "$1 --lt $1"
    expect_failure "$2 --lt $2"
}


le() {  # $1 < $2
    expect_success "$1 --le $2"
    expect_failure "$2 --le $1"
    expect_success "$1 --le $1"
    expect_success "$2 --le $2"
}

gt() { 
    expect_failure "$1 --gt $2"
    expect_success "$2 --gt $1"
    expect_failure "$1 --gt $1"
    expect_failure "$2 --gt $2"
}

ge() {  
    expect_failure "$1 --ge $2"
    expect_success "$2 --ge $1"
    expect_success "$1 --ge $1"
    expect_success "$2 --ge $2"
}

eq() {
    expect_failure "$1 --eq $2"
    expect_failure "$2 --eq $1"
    expect_success "$1 --eq $1"
    expect_success "$2 --eq $2"
}

ne() {
    expect_success "$1 --ne $2"
    expect_success "$2 --ne $1"
    expect_failure "$1 --ne $1"
    expect_failure "$2 --ne $2"
}

for i in \
   "paket-1.2.3-4 paket-1.2.5-4" \
   "paket-1.2.3_alpha-0 paket-1.2.3_beta-0" \
   "paket-1.2.3_alpha-0 paket-1.2.3_alpha1-0" \
   "paket-1.2.3_beta-0 paket-1.2.3_beta1-0" \
   "paket-1.2.3_rc-0 paket-1.2.3_rc1-0" \
   "paket-1.2.3-0 paket-1.2.3_p-0" \
   "paket-1.2.3_p-0 paket-1.2.3_p1-0" \
   "paket-1-0 paket-1a_p-0" \
   "paket-a-0 paket-1_p-0" \
   "paket-1-0 paket-1.0_p-0" \
   "paket-1.0-0 pake-1a.0_p-0" \
   "paket_1-0-0 paket-0_p-0" \
   "1.0-0 paket_sub-10_p-0" \
   "a_paket_subname-1.3.4-2 a_paket_subname-123456_p-0" \
   "1.a 1.0_p" \
   "1.a 1.0_p-0" 
   
  do
    lt ${i}
    le ${i}
    gt ${i}
    ge ${i}
    eq ${i}
    ne ${i}
#    max ${i}
#    min ${i}
done
