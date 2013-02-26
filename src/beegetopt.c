/*
** beegetopt - parse options
**
** Copyright (C) 2009-2012
**       Marius Tolzmann <tolzmann@molgen.mpg.de>
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

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <assert.h>

#include "bee_getopt.h"

#define OTYPE_FLAG     0
#define OTYPE_OPTIONAL 1
#define OTYPE_REQUIRED 2


void usage(void)
{
   printf(
      "Usage: beegetopt [options] -- [arguments and options to be parsed]\n\n"
      "  -o | --option=OPTION[,OPTION]   options to be recognized\n"
      "\n"
      "  -n | --name=NAME                set program name\n"
      "  -N | --stop-on-no-option        stop parsing when argument\n"
      "                                  is not an option\n"
      "  -U | --stop-on-unknown-option   stop parsing when argument\n"
      "                                  is an unknown option\n"
      "  -S | --no-skip-unknown-option   carp on unknown options\n"
      "                                  by default unknown options are skipped\n"
      "                                  and handled as non-option argument.\n"
      "                                  -S is ignored if -U is given.\n"
      "  -k | --keep-option-end          respect but keep '--' at it's position\n"
      "\n"
      "  -h | --help                     print this little help\n"
      "  -V | --version                  print version\n"
      "\n"
      " OPTION may be a short or a long option or a combination of long and\n"
      " short options seperated by a slash (/). In this case the second to last\n"
      " option are considered aliases for the first. Only the first option is\n"
      " returned as a new commandline argument: e.g.\n"
      "    --option long/alias/a/b  will always return --long\n"
      "    --option a/b/long        will always return -a\n"
      "\n"
      " If the last character of OPTION is '=' this option will require an\n"
      " argument. "
      " If it is ':' this option takes an optional argument: e.g.\n"
      "    --option name/n=\n"
      "\n"
   );
}


int main(int argc, char *argv[])
{
   struct bee_getopt_ctl optctl;

   struct bee_option opts[] = {
       BEE_OPTION_NO_ARG("help",     'h'),
       BEE_OPTION_NO_ARG("version",  'V'),
       BEE_OPTION_REQUIRED_ARG("option", 'o'),
       BEE_OPTION_REQUIRED_ARG("longoption", 'o'),
       BEE_OPTION_REQUIRED_ARG("name", 'n'),
       BEE_OPTION_NO_ARG("stop-on-unknown-option",  'U'),
       BEE_OPTION_NO_ARG("stop-on-no-option",  'N'),
       BEE_OPTION_NO_ARG("keep-option-end",  'k'),
       BEE_OPTION_NO_ARG("no-skip-unknown-option",  'S'),
       BEE_OPTION_END
   };

   int opt;
   int i,l,idx;
   short type;

   struct bee_option *beeopts = NULL;

   int    beeoptc = 0;
   int    maxopts = 0;
   int    totalopts = 0;
   char **beeoptptr = NULL;
   char **tmp;
   char  *sep;
   char  *sepstr;
   char  *name = NULL;
   int    flags = BEE_FLAG_SKIPUNKNOWN;

   bee_getopt_init(&optctl, argc-1, &argv[1], opts);

   while((opt=bee_getopt(&optctl, &i)) != BEE_GETOPT_END) {

       if (opt == BEE_GETOPT_ERROR) {
           exit(1);
       }

       switch(opt) {
           case 'V':
                printf("beegetopt Vx.x\n");
           case 'h':
               usage();
               exit(0);
               break;

           case 'n':
               name = optctl.optarg;
               break;

           case 'N':
               flags |= BEE_FLAG_STOPONNOOPT;
               break;

           case 'U':
               flags |= BEE_FLAG_STOPONUNKNOWN;
               break;

           case 'k':
               flags |= BEE_FLAG_KEEPOPTIONEND;
               break;

           case 'S':
               flags &= ~BEE_FLAG_SKIPUNKNOWN;
               break;

           case 'o':
               sep = optctl.optarg;

               while(sep) {
                   sepstr = strsep(&sep, ",");
                   if (beeoptc == maxopts) {
                       maxopts += 128;
                       tmp = realloc(beeoptptr, sizeof(beeoptptr)*(maxopts));

                       if (!tmp) {
                           perror("realloc(beeopts)");
                           exit(1);
                       }
                       beeoptptr = tmp;
                   }
                   beeoptptr[beeoptc] = sepstr;
                   beeoptc++;
                   totalopts++;

                   while ((sepstr = strchr(sepstr, '/'))) {
                       while (*sepstr == '/') sepstr++;
                       totalopts++;
                   }

               }

               break;
       }
   }

   if(!totalopts) {
       usage();
       exit(1);
   }

   beeopts = calloc(totalopts+1, sizeof(*beeopts));

   if(!beeopts) {
       perror("calloc(beeopts)");
       exit(1);
   }

   for (i=0,opt=0; i < beeoptc; i++) {

       sep = beeoptptr[i];

       idx = opt;

       l = strlen(sep);

       if (sep[l-1] == ':') {
           type = OTYPE_OPTIONAL;
           sep[l-1] = 0;
       } else if (sep[l-1] == '=') {
           type = OTYPE_REQUIRED;
           sep[l-1] = 0;
       } else {
           type = OTYPE_FLAG;
       }

//       fprintf(stderr, "beeoptptr[%d] = \"%s\" type %d\n", i, sep, type);

       while(sep) {
           assert(opt < totalopts);

           sepstr = strsep(&sep, "/");
           while (sep && *sep == '/')
               sep++;

           if (sep && !*sep)
               sep = NULL;

           while (sepstr[0] == '-')
               sepstr++;

           if (!sepstr[0])
               continue;

           BEE_INIT_OPTION_DEFAULTS(&beeopts[opt]);

           beeopts[opt].value = idx;
           beeopts[opt].required_args = (type == OTYPE_REQUIRED);
           beeopts[opt].optional_args = (type == OTYPE_OPTIONAL);
           beeopts[opt].type = (type == OTYPE_FLAG)?BEE_TYPE_FLAG:BEE_TYPE_STRING;

           if (sepstr[1])
               beeopts[opt].long_opt = sepstr;
           else
               beeopts[opt].short_opt = sepstr[0];

//           fprintf(stderr, "  setting up option %d: idx=%d opt='%s' value=%d required=%d optional=%d type=%d\n", opt+1, opt, sepstr, beeopts[opt].value, beeopts[opt].required_args, beeopts[opt].optional_args, beeopts[opt].type);

           opt++;
       }

       BEE_INIT_OPTION_END(&beeopts[opt]);
    }

    bee_getopt_init(&optctl, optctl.argc-optctl.optind, &argv[optctl.optind+1], beeopts);

    optctl.program = name?name:"program";
    optctl.flags = flags;

    while((opt=bee_getopt(&optctl, &i)) != BEE_GETOPT_END) {
       if (opt == BEE_GETOPT_ERROR) {
           free(beeopts);
           free(beeoptptr);
           exit(1);
       }

       switch(opt) {
           default:
               if (beeopts[beeopts[i].value].long_opt) {
                   printf("--%s ", beeopts[beeopts[i].value].long_opt);
               } else {
                   printf("-%c ", beeopts[beeopts[i].value].short_opt);
               }

               if (beeopts[beeopts[i].value].required_args || beeopts[beeopts[i].value].optional_args) {

                  if (optctl.optarg) {
                       bee_getopt_print_quoted(optctl.optarg);
                       putchar(' ');
                  } else {
                       printf("'' ");
                  }
               }
               break;
        }
    }

    printf(" -- ");

   while(optctl.optind < optctl.argc) {
       bee_getopt_print_quoted(optctl.argv[optctl.optind]);
       putchar(' ');
       optctl.optind++;
   }
    printf("\n");

   free(beeopts);
   free(beeoptptr);

   return 0;
}



