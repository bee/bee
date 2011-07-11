/*
 * beeversion - compare bee package versionnumbers
 * Copyright (C) 2010-2011
 *       Marius Tolzmann <tolzmann@molgen.mpg.de>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, see <http://www.gnu.org/licenses/>.
 */

#include "beeversion.h"

int compare_version_strings(char *v1, char *v2);
int compare_beepackage_names(struct beeversion *v1, struct beeversion *v2);
int compare_beeversions(struct beeversion *v1, struct beeversion *v2);
int compare_beepackages(struct beeversion *v1, struct beeversion *v2);
