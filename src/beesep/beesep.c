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
        char gl=0;      //gibt an, ob ein Gleichzeichen gefunden wurde
        char dp=0;      //gibt an, ob ein Doppelpunkt gefunden wurde
        int dp_pos=0;   //gibt die Position des letzten Doppelpunktes an
        int segmente=0; //Anzahl der Segmente, in die der String sich teilen laesst
        int strlaenge=0;
        int i=0;
        if(argc<2)
        {
                fprintf(stderr,"Parameter fehlt\n");
                return -1;
        }
        
        while(argv[1][i]!='\0')
        {       
                //bevor nicht ein = gekommen ist, kann kein Trenner( : ) kommen
                if(argv[1][i]=='=')
                {
                        gl=1;
                }
                //wenn ein Doppelpunkt kommt, merken dass einer da war und wo
                if(gl && argv[1][i]==':')
                {
                        dp_pos=i;
                        dp=1;
                }
                //wenn das naechste = kommt, dann war der letzte Doppelpunkt Trenner
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
                fprintf(stderr,"Es ist kein '=' vorhanden\n");
                return -1;
        }
        
        //Zerlegten String in Array von Strings speichern und ausgeben
        strlaenge=i;
        i=0;
        gl=0;
        while(i<strlaenge)
        {
                printf("%c",argv[1][i]);
                fflush(stdout);
                if(!gl && !isalnum(argv[1][i]) && argv[1][i]!='=')
                {
                        fprintf(stderr,"\nFehler: %c   Key darf nur alphanumerische Zeichen enthalten.\n",argv[1][i]);
                        return -1;
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
