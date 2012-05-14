/*
** beesep - split beefind output
**
** Copyright (C) 2009-2011
**       David Fessler <dfessler@uni-potsdam.de>
**       Marius Tolzmann <tolzmann@molgen.mpg.de>
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

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
int main(int argc, char** argv)
{
        char gl=0;      /* equal sign */
        char dp=0;      /* colon */
        int dp_pos=0;   /* position of last colon */
        int segmente=0; /* number of tokens */
        int strlaenge=0;
        int i=0;
        if(argc<2)
        {
                fprintf(stderr,"beesep: missing argument\n");
                return 2;
        }
        
        while(argv[1][i]!='\0')
        {       
                /* no ':' before '=' */
                if(argv[1][i]=='=')
                {
                        gl=1;
                }
                /* save occurrence and position of ':' */
                if(gl && argv[1][i]==':')
                {
                        dp_pos=i;
                        dp=1;
                }
                /* last ':' was a delimiter if there is a '=' */
                if(gl && dp && argv[1][i]=='=')
                {
                        argv[1][dp_pos]='\0';
                        segmente++;
                        gl=1;
                        dp=0;
                }
                i++;
        }
        if(!gl)
        {
                fprintf(stderr,"beesep: there is no '='\n");
                return 1;
        }
        
        /* save tokens in an array and print them */
        strlaenge=i;
        i=0;
        gl=0;
        while(i<strlaenge)
        {
                printf("%c",argv[1][i]);
                fflush(stdout);
                if(!gl && !isalnum(argv[1][i]) && argv[1][i]!='=')
                {
                        fprintf(stderr,"\nbeesep: '%c' only alpha numeric characters are allowed in the key\n",argv[1][i]);
                        return 1;
                }
                if(argv[1][i]=='\0')
                {       
                        gl=0;
                        printf("'\n");
                }
                if(argv[1][i]=='\'')
                {
                        printf("\\''");
                }       
                if(argv[1][i]=='=' && !gl)
                {       
                        gl=1;
                        printf("'");
                }
                i++;            
        }
        printf("'\n");
        return 0;
}
