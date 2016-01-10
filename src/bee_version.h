/*
** beeversion - compare bee package versionnumbers
**
** Copyright (C) 2009-2016
**       Marius Tolzmann <m@rius.berlin>
**       Tobias Dreyer <dreyer@molgen.mpg.de>
**       and other bee developers
**
** This file is part of bee.
**
** bee is free software; you can redistribute it and/or modify
** it under the terms of the GNU General Public License as published by
** the Free Software Foundation, either version 3 of the License, or
** (at your option) any later version.
**
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
** GNU General Public License for more details.
**
** You should have received a copy of the GNU General Public License
** along with this program; if not, see <http://www.gnu.org/licenses/>.
*/

#ifndef _BEE_BEEVERSION_H_
#define _BEE_BEEVERSION_H_

#define BEEVERSION_MAJOR    1
#define BEEVERSION_MINOR    3
#define BEEVERSION_PATCHLVL 0

#include <stdlib.h>

struct beeversion {
    char *string;
    char *pkgname;
    char *subname;
    char *version;
    char *extraversion;
    int   extraversion_typ;
    char *extraversion_nr;
    char *pkgrevision;
    char *arch;
    char *suffix;
};

struct extra_version {
    char         *string;
    unsigned int  priority;
    size_t        length;
};

#define SUPPORTED_ARCHITECTURES \
            "noarch", "any", \
            "x86_64", "i686", "i386", "i486", "i586", \
            "alpha", "arm", "m68k", "sparc", "mips", "ppc"

#define EXTRA_UNKNOWN 200
#define EXTRA_ALPHA   1
#define EXTRA_BETA    2
#define EXTRA_RC      3
#define EXTRA_NONE    4
#define EXTRA_PATCH   5
#define EXTRA_ANY     6

#endif
