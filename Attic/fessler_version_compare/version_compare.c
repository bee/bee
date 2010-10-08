#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <string.h>
#include <ctype.h>
#include "version_compare.h"

int how_many(char z, char* str, int len)
{
	int i=0;
	int counter=0;
	for(i=0;i<len;i++)
	{
		if(str[i]==z)
		counter++;
	}
	return counter;
}
int parse_argument(char* text, int len, struct nr *versionsnummer)
{	
	int i=0;
	int minuszeichen=0;
	//Syntaxfehler finden
	if(strstr(text,"_-")!=NULL)
	{
		printf("Syntaxfehler bei: %s. '_-' nicht erlaubt.\nName-Syntax: <pkgname>[_<subname>]-<version>[_<extraversion>]-<pkgrevision>[.<arch>]\n",text);
		return -1;
	}
	if((minuszeichen=how_many('-',text,strlen(text)))<2)
	{
		printf("Syntaxfehler bei: %s. Zu wenig '-'.\nName-Syntax: <pkgname>[_<subname>]-<version>[_<extraversion>]-<pkgrevision>[.<arch>]\n",text);
		return -1;
	}
	
	//Speicher fuer die drei Teilstrings reservieren und zerlegen
	char separator[] = "-";
	char* teil1;
	char* teil2;
	char* teil3;
	char* token_alt;
	char* token_alt2;
	char* token;
	token=strtok(text, separator);
	teil1=token;
	while(token!=NULL)
	{	
		token_alt2=token_alt;
		token_alt=token;
		token=strtok(NULL, separator);
	}
	teil2=token_alt2;
	teil3=token_alt;

	
	//Falls in pkgname Minuszeichen vorhanden sind, so wurden diese durch \0 ersetzt. Das wird wieder rueckgaengig gemacht;
	while(minuszeichen-2>0)
	{
		if(text[i]=='\0')
		{
			text[i]='-';
			minuszeichen--;	
		}
		i++;
	}
		
	//falls subname vorhanden zerlegen
	strcpy(separator,"_");
	versionsnummer->pkgname=strtok(teil1, separator);
	versionsnummer->subname=strtok(NULL, separator);
	//falls extraversion vorhanden zerlegen
	strcpy(separator,"_");
	versionsnummer->version=strtok(teil2, separator);
	versionsnummer->extraversion=strtok(NULL, separator);
	//falls arch vorhanden zerlegen
	strcpy(separator,".");
	versionsnummer->pkgrevision=strtok(teil3, separator);
	versionsnummer->arch=strtok(NULL, separator);
	if(versionsnummer->pkgrevision==NULL)
	{
		fprintf(stderr,"Syntaxfehler bei: %s. Keine Paketrevision angegeben.\nName-Syntax: <pkgname>[_<subname>]-<version>[_<extraversion>]-<pkgrevision>[.<arch>]\n",text);
	}
	//printf("%s%s%s%s%s%s\n",versionsnummer->pkgname,versionsnummer->subname,versionsnummer->version,versionsnummer->extraversion,versionsnummer->pkgrevision,versionsnummer->arch);
	return 0;	
}
char lt2(int i1, int i2, char ende)
{	
	if(ende)
		return (i1<i2);
	return (i1<=i2);
}
char gt2(int i1, int i2, char ende)
{
	if(ende)
		return (i1>i2);
	return (i1>=i2);
}
char le2(int i1, int i2, char ende)
{
	return (i1<=i2);
}
char ge2(int i1, int i2,char ende)
{
	return (i1>=i2);
}
char e2(int i1, int i2, char ende)
{
	return (i1==i2);
}
char ne2(int i1, int i2,char ende)
{
	return (i1!=i2);
}
char lt(char* z1,char* z2, int i)
{	
	//eine Versionsnummer laenger als die andere
	if(z1[i]=='.' && z2[i]!='.' && isalnum(z2[i]))
		return 2;
	if(z1[i]!='.' && z2[i]=='.' && isalnum(z1[i]))
		return 0;
	//beide Stellen gleich und kein Stringende: weiter
	if(z1[i]==z2[i] && (z1[i+1]!='\0' || z2[i+1]!='\0'))
		return 1;
	//einer der Strings ist zuende
	if(z1[i]=='\0')
		return 2;
	if(z2[i]=='\0')
		return 0;
	//Buchstabe und Buchstabe werden verglichen
	if(!isdigit(z1[i]) && !isdigit(z2[i]))
		return (z1[i]<z2[i]);
	//Buchstabe und Ziffer werden verglichen
	if(isdigit(z1[i]) && !isdigit(z2[i]))
		return 0;
	if(!isdigit(z1[i]) && isdigit(z2[i]))
		return 2;
	//unterschiedliche Ziffern: auswerten und danach Vergleich beenden
	if(atoi(&z1[i])<atoi(&z2[i]))
	{
		return 2;
	}
	else
	{
		return 0;
	}
}
char gt(char* z1,char* z2, int i)
{
	//eine Versionsnummer laenger als die andere
	if(z1[i]=='.' && z2[i]!='.'  && isalnum(z2[i]))
		return 0;
	if(z1[i]!='.' && z2[i]=='.'  && isalnum(z1[i]))
		return 2;
	//beide Stellen gleich und kein Stringende: weiter
	if(z1[i]==z2[i] && (z1[i+1]!='\0' || z2[i+1]!='\0'))
		return 1;
	//einer der Strings ist zuende
	if(z1[i]=='\0')
		return 0;
	if(z2[i]=='\0')
		return 2;
	//Buchstabe und Buchstabe werden verglichen
	if(!isdigit(z1[i]) && !isdigit(z2[i]))
		return (z1[i]>z2[i]);
	//Buchstabe und Ziffer werden verglichen
	if(isdigit(z1[i]) && !isdigit(z2[i]))
		return 2;
	if(!isdigit(z1[i]) && isdigit(z2[i]))
		return 0;
	//unterschiedliche Ziffern: auswerten und danach Vergleich beenden
	if(atoi(&z1[i])>atoi(&z2[i]))
	{
		return 2;
	}
	else
	{
		return 0;
	}
}
char le(char* z1,char* z2, int i)
{
	//beide Stellen gleich: weiter
	if(z1[i]==z2[i])
		return 1;
	//eine Versionsnummer laenger als die andere
	if(z1[i]=='.' && z2[i]!='.'  && isalnum(z2[i]))
		return 2;
	if(z1[i]!='.' && z2[i]=='.'  && isalnum(z1[i]))
		return 0;
	//einer der Strings ist zuende
	if(z1[i]=='\0')
		return 2;
	if(z2[i]=='\0')
		return 0;
	//Buchstabe und Buchstabe werden verglichen
	if(!isdigit(z1[i]) && !isdigit(z2[i]))
		return (z1[i]<=z2[i]);
	//Buchstabe und Ziffer werden verglichen
	if(isdigit(z1[i]) && !isdigit(z2[i]))
		return 0;
	if(!isdigit(z1[i]) && isdigit(z2[i]))
		return 2;
	//unterschiedliche Ziffern: auswerten und danach Vergleich beenden
	if(atoi(&z1[i])<=atoi(&z2[i]))
	{
		return 2;
	}
	else
	{
		return 0;
	}
}
char ge(char* z1,char* z2, int i)
{	
	//beide Stellen gleich und kein Stringende: weiter
	if(z1[i]==z2[i])
		return 1;
	//eine Versionsnummer laenger als die andere
	if(z1[i]=='.' && z2[i]!='.'  && isalnum(z2[i]))
		return 0;
	if(z1[i]!='.' && z2[i]=='.'  && isalnum(z1[i]))
		return 2;
	//einer der Strings ist zuende
	if(z1[i]=='\0')
		return 0;
	if(z2[i]=='\0')
		return 2;
	//Buchstabe und Buchstabe werden verglichen
	if(!isdigit(z1[i]) && !isdigit(z2[i]))
		return (z1[i]>=z2[i]);
	//Buchstabe und Ziffer werden verglichen
	if(isdigit(z1[i]) && !isdigit(z2[i]))
		return 2;
	if(!isdigit(z1[i]) && isdigit(z2[i]))
		return 0;
	//unterschiedliche Ziffern: auswerten und danach Vergleich beenden
	if(atoi(&z1[i])>=atoi(&z2[i]))
	{
		return 2;
	}
	else
	{
		return 0;
	}
}
char ne(char* z1,char* z2, int i)
{
	//eine Versionsnummer laenger als die andere
	if(z1[i]=='.' && z2[i]!='.')
		return 2;
	if(z1[i]!='.' && z2[i]=='.')
		return 2;
	//Buchstabe und Buchstabe werden verglichen
	if(!isdigit(z1[i]) && !isdigit(z2[i]) && z1[i]!=z2[i])
		return (2);
	//einer der Strings ist zuende
	if(z1[i]=='\0')
		return 2;
	if(z2[i]=='\0')
		return 2;
	//Buchstabe und Ziffer werden verglichen
	if(isdigit(z1[i]) && !isdigit(z2[i]))
		return 2;
	if(!isdigit(z1[i]) && isdigit(z2[i]))
		return 2;
	//unterschiedliche Ziffern: auswerten und danach Vergleich beenden
	if(atoi(&z1[i])!=atoi(&z2[i]))
	{
		return 2;
	}
	return(1);
	
}
char e(char* z1,char* z2, int i)
{
	return(z1[i]==z2[i]);
}
//faengt str1 mit str2 an?
char starts_with(char* str1, char* str2, int len1, int len2, int* pos)
{
	int i=0;
	while(i<len1 && i<len2)
	{
		if(str1[i]!=str2[i])
		{
			return 0;
		}
		*pos=i+1,
		i++;
	}
	return(i==len2);
}
//return: 0 falsch, 2 gleich, 1 wahr
char compare(struct nr *nr1, struct nr *nr2, char(*fp)(char*,char*, int))
{	
	int i=0;
	char falsch=0;
	int ret_val=0;
	//Versionsnummer auf Gleichheit testen, bei Gleichheit wird Extraversion ausgewertet
	if(strcmp(nr1->version, nr2->version)==0)
	{
		return 2;
	}
	//Versionsnummern auswerten
	while((nr1->version[i]!='\0' || nr2->version[i]!='\0') && !falsch)
	{
		//Vergleich kann nur noch falsch sein
		if(!(ret_val=fp(nr1->version,nr2->version,i)))
		{	
			//printf("f: %c -- %c\n",nr1->version[i],nr2->version[i]);
			falsch=1;
		}
		//Vergleich ist bisher wahr
		else
		{
			//Vergleich kann nur noch wahr sein
			if(ret_val==2)
			{
				return 1;
			}
			//printf("w: %c -- %c\n",nr1->version[i],nr2->version[i]);
		}
		i++;	
	}
	//Ausgabe je nachdem, ob ein Fehler gefunden wurde
	if(falsch)
	{
		return 0;
	}
	else
	{
		return 1;
	}
	return 0;
}
char parse_extra(struct nr *nr1)
{

	if(nr1->extraversion==NULL)
	{	
		nr1->extraversion_typ=8;
		return 0;
	}
	nr1->extraversion_typ=-1;
	//Extraversion vergleichen	
	char extraversions[5][10]={{"alpha"}, {"beta"}, {"rc"},{"\n"},{"p"}};
	int i=0;
	int pos;
	//alle moeglichen Extraversionen durchgehen
	for(i=0;i<5;i++)
	{	
			//wenn eine Extraversion mit einem gueltigen Wort beginnt
			if(starts_with(nr1->extraversion,extraversions[i],strlen(nr1->extraversion),strlen(extraversions[i]),&pos))
			{
				nr1->extraversion_typ=2*(i+1);
				//nicht mehr weitersuchen
				break;	
			}	
	}
	int j=0;
	//keine Extraversion gefunden
	if(nr1->extraversion_typ==-1)
	{	
		//durch die Nummer wird der Typ 12
		nr1->extraversion_typ=11;
	}
	//wenn es noch eine Nummer gibt
	if(pos<strlen(nr1->extraversion))
	{
		nr1->extraversion_typ++;
		nr1->extraversion_nr=calloc(strlen(nr1->extraversion)-strlen(extraversions[i])+1,sizeof(char));
		while(pos<strlen(nr1->extraversion))
		{	
			//extraversion mit Nummer hat einen um eins hoeheren Rang
			nr1->extraversion_nr[j]=nr1->extraversion[pos];
			pos++;
			j++;
		}
			nr1->extraversion_nr[j]='\0';
	}
	return 	0;
}
char compare_extra(struct nr *nr1, struct nr *nr2, char(*fp)(int, int, char))
{
	
	parse_extra(nr1);
	parse_extra(nr2);
	//Falls Vergleich der Nummer noetig, also wenn wenn Typen der Extraversion unterschiedlich sind
	if(nr1->extraversion_typ==nr2->extraversion_typ)
	{		
			//Wenn keine Extraversion vorhanden ist (eine Extraversion wird durch den Typ abgefangen)
			if(nr1->extraversion_nr==NULL && nr2->extraversion_nr==NULL)
			{	
				//Paketrevision vergleichen
				return fp(atoi(nr1->pkgrevision),atoi(nr2->pkgrevision),1);
			}
			//sonst extraversion Nr. auswerten, bei Gleichheit Paketrevision vergleichen
			if(strcmp(nr1->extraversion_nr,nr2->extraversion_nr)==0)
			{
				return fp(atoi(nr1->pkgrevision),atoi(nr2->pkgrevision),1);
			}
			return fp(atoi(nr1->extraversion_nr),atoi(nr2->extraversion_nr),1);
	}
	return fp(nr1->extraversion_typ,nr2->extraversion_typ,1);
}
struct nr* max(struct nr* array, int anzahl)
{
	int i=0;
	int retval=0;
	struct nr* akt=&array[0];
	for(i=1; i<anzahl;i++)
	{
		if((retval=compare(&array[i], akt,gt))==2)
		{
			if(compare_extra(&array[i],akt,gt2))
			{
				akt=&array[i];
				//printf("%s %s %s\n",akt->pkgname,akt->version,akt->extraversion);
			}
		}
		else if(retval==1)
		{
			akt=&array[i];
		}
	}
	return akt;
}
struct nr* min(struct nr* array, int anzahl)
{
	int i=0;
	int retval=0;
	struct nr* akt=&array[0];
	for(i=1; i<anzahl;i++)
	{
		if((retval=compare(&array[i], akt,lt))==2)
		{
			if(compare_extra(&array[i],akt,lt2))
			{
			akt=&array[i];
			//printf("%s %s %s\n",akt->pkgname,akt->version,akt->extraversion);
			}
		}
		else if(retval==1)
		{
			akt=&array[i];
		}
	}
	return akt;
}
char ismax(struct nr* versionsnummer, struct nr* max)
{
	return (versionsnummer->version==max->version && versionsnummer->extraversion==max->extraversion && versionsnummer->pkgrevision==max->pkgrevision);
		
}

char ismin(struct nr* versionsnummer, struct nr* min)
{
	return (versionsnummer->version==min->version && versionsnummer->extraversion==min->extraversion && versionsnummer->pkgrevision==min->pkgrevision);
		
}

int main(int argc, char **argv)
{
	char show_pkgname=0;
	char show_subname=0;
	char show_version=0;
	char show_extraversion=0;
	char show_pkgrevision=0;
	char show_arch=0;
	char show_keyvalue=0;
	int mode=0;
	int option_index = 0;
	int i=0;
	int c=0;
	int retval=0;
	char (*fp[6])(char*, char*, int)={&lt,&gt,&le,&ge,&ne,&e};
	char (*fp2[6])(int, int, char)={&lt2,&gt2,&le2,&ge2,&ne2,&e2};
	
	static struct option long_options[] = {
	
    	{"lt", no_argument, 0, 1},
	{"gt", no_argument, 0, 2},
	{"le", no_argument, 0, 3},
	{"ge", no_argument, 0, 4},
	{"ne", no_argument, 0, 5},
	{"eq",  no_argument, 0, 6},
	{"parse", no_argument, 0, 7},
	{"max", no_argument, 0, 8},
	{"min", no_argument, 0, 9},
	{"ismax", no_argument, 0, 10},
	{"ismin", no_argument, 0, 11},
	{"isnotmax", no_argument, 0, 12},
	{"isnotmin", no_argument, 0, 13},
	{"pkgname", no_argument, 0, 'l'},
	{"subname", no_argument, 0, 'm'},
	{"version", no_argument, 0, 'n'},
	{"extraversion", no_argument, 0, 'o'},
	{"pkgrevision", no_argument, 0, 'p'},
	{"arch", no_argument, 0, 'q'},
	{"keyvalue", no_argument, 0, 'r'},
	{0, 0, 0, 0}
    };
	
	c = getopt_long(argc, argv, "",long_options, &option_index);

    	while (c != -1)
	{
		if(c<14)
		{
			if(mode)
			{
				printf("beeversion kann nur mit einem der Parameter --[lt, gt, le, ge, ne, e, parse, max, min, isnotmax, isnotmin] gestartet werden.\n");
			}
			mode=c;
		}
		switch (c) 
		{
			case 'l':
			{
				show_pkgname=1;
				break;
			}
			case 'm':
			{
				show_subname=1;
				break;
			}case 'n':
			{
				show_version=1;
				break;
			}
			case 'o':
			{
				show_extraversion=1;
				break;
			}
			case 'p':
			{
				show_pkgrevision=1;
				break;
			}
			case 'q':
			{
				show_arch=1;
				break;
			}
			case 'r':
			{
				show_keyvalue=1;
				break;
			}
		}
		c = getopt_long(argc, argv, "abcdef",long_options, &option_index);
    	}
	//speicher reservieren
	struct nr *versionsnummern=calloc(argc-optind,sizeof(struct nr));
	if(versionsnummern==NULL)
	{
		printf("calloc error\n");
		return(-1);
	}
	//Anzahl Versionsnummern berechnen und Strukturen bereitstellen
	for(i=optind; i < argc; i++)
	 {
		if(parse_argument(argv[i],strlen(argv[i]),&versionsnummern[i-optind])<0)
		{	
			free(versionsnummern);
			return -1;
		}
	 }
	 //kein Parameter angegeben
	 if(mode<1)
	 {
	 	printf("beeversion muss mit einem der Parameter --[lt, gt, le, ge, ne, e, parse, max, min, isnotmax, isnotmin] gestartet werden.\n");
		free(versionsnummern);
		return -1;
	 }
	 //lt, gt, le, ge, ne, e
	if(mode<7)
	{
		if(argc-optind<2)
		{		
			printf("Zwei Parameter werden benoetigt.\n");
			free(versionsnummern);
			return -1;
		}
		//Namen vergleichen, falls unterschiedlich: falsch ausgeben
		if(strcmp(versionsnummern[0].pkgname,versionsnummern[1].pkgname)!=0)
		{
			printf("Die Paketnamen muessen gleich sein.\n");
			free(versionsnummern);
			return -1;
		}
		//Versionsnummervergleich ergab Gleichheit
		if((retval=compare(&versionsnummern[0], &versionsnummern[1],fp[mode-1]))==2)
		{
			retval=compare_extra(&versionsnummern[0], &versionsnummern[1],fp2[mode-1]);
			free(versionsnummern);
			return !retval;
		}
		return !retval;
	}
	//parse
	if(mode==7)
	{	
		if(argc-optind<1)
		{
			printf("Ein Parameter wird benoetigt.\n");
			free(versionsnummern);
			return -1;
		}
		//standardmaessig alle ausgeben, wenn keine Parameter fuer Ausgabe
		if(!show_pkgname && !show_subname && !show_version && !show_extraversion && !show_pkgrevision && !show_arch)
		{
			show_pkgname=1;
			show_subname=1;
			show_version=1;
			show_extraversion=1;
			show_pkgrevision=1;
			show_arch=1;
		}	
			
			//wenn --keyvalue aktiviert ist
			if(show_keyvalue)
			{
				if( show_pkgname && versionsnummern[0].pkgname!=NULL)
					printf("PKGNAME=");
				if(show_pkgname && versionsnummern[0].pkgname!=NULL)
					printf("%s",versionsnummern[0].pkgname);
				if(show_pkgname && versionsnummern[0].subname!=NULL)
					printf("\nPKGSUBNAME=");
				if(show_subname && versionsnummern[0].subname!=NULL)
					printf("%s",versionsnummern[0].subname);
				if(show_pkgname && versionsnummern[0].version!=NULL)
					printf("\nPKGVERSION=");
				if(show_version && versionsnummern[0].version!=NULL)
					printf("%s",versionsnummern[0].version);
				if(show_pkgname && versionsnummern[0].extraversion!=NULL)
					printf("\nPKGEXTRAVERSION=");
				if(show_extraversion && versionsnummern[0].extraversion!=NULL)
					printf("%s",versionsnummern[0].extraversion);
				if(show_pkgname && versionsnummern[0].pkgrevision!=NULL)
					printf("\nPKGREVISION=");
				if(show_pkgrevision && versionsnummern[0].pkgrevision!=NULL)
					printf("%s",versionsnummern[0].pkgrevision);
				if(show_pkgname && versionsnummern[0].arch!=NULL)
					printf("\nPKGARCH=");
				if(show_arch && versionsnummern[0].arch!=NULL)
					printf("%s",versionsnummern[0].arch);
				printf("\n");
			}
			//sonst normal ausgeben
			else
			{
				if(show_pkgname && versionsnummern[0].pkgname!=NULL)
					printf("%s",versionsnummern[0].pkgname);
				if(show_subname && versionsnummern[0].subname!=NULL)
					printf("_%s",versionsnummern[0].subname);
				if(show_version && versionsnummern[0].version!=NULL)
					printf("-%s",versionsnummern[0].version);
				if(show_extraversion && versionsnummern[0].extraversion!=NULL)
					printf("_%s",versionsnummern[0].extraversion);
				if(show_pkgrevision && versionsnummern[0].pkgrevision!=NULL)
					printf("-%s",versionsnummern[0].pkgrevision);
				if(show_arch && versionsnummern[0].arch!=NULL)
					printf(".%s",versionsnummern[0].arch);
				printf("\n");
			}
	}
	//max
	if(mode==8)
	{	
		if(argc-optind<2)
		{		
			printf("Zwei Parameter werden benoetigt.\n");
			free(versionsnummern);
			return -1;
		}
		
		struct nr* ret=max(versionsnummern, argc-optind);
		if(versionsnummern[0].pkgname!=NULL)
			printf("%s",ret->pkgname);
		if(versionsnummern[0].subname!=NULL)
			printf("_%s", ret->subname);
		if(versionsnummern[0].version!=NULL)
			printf("-%s",ret->version);
		if(versionsnummern[0].extraversion!=NULL)
			printf("_%s",ret->extraversion);
		if(versionsnummern[0].pkgrevision!=NULL)
			printf("-%s",ret->pkgrevision);
		if(versionsnummern[0].arch!=NULL)
			printf(".%s",ret->arch);
		printf("\n");
	}
	//min
	if(mode==9)
	{	
		if(argc-optind<2)
		{		
			printf("Zwei Parameter werden benoetigt.\n");
			free(versionsnummern);
			return -1;
		}
	
		struct nr* ret=min(versionsnummern, argc-optind);
		if(versionsnummern[0].pkgname!=NULL)
			printf("%s",ret->pkgname);
		if(versionsnummern[0].subname!=NULL)
			printf("_%s", ret->subname);
		if(versionsnummern[0].version!=NULL)
			printf("-%s",ret->version);
		if(versionsnummern[0].extraversion!=NULL)
			printf("_%s",ret->extraversion);
		if(versionsnummern[0].pkgrevision!=NULL)
			printf("-%s",ret->pkgrevision);
		if(versionsnummern[0].arch!=NULL)
			printf(".%s",ret->arch);
		printf("\n");
	}
	//ismax
	if(mode==10)
	{	
		struct nr* ret=max(versionsnummern, argc-optind);
		int retval=ismax(&versionsnummern[0],ret);
		free(versionsnummern);
		return(!retval);
	}
	//ismin
	if(mode==11)
	{	
		struct nr* ret=min(versionsnummern, argc-optind);
		int retval=ismin(&versionsnummern[0],ret);
		free(versionsnummern);
		return(!retval);
	}
	//isnotmax
	if(mode==12)
	{	
		struct nr* ret=max(versionsnummern, argc-optind);
		int retval=!ismax(&versionsnummern[0],ret);
		free(versionsnummern);
		return(!retval);
	}
	//isnotmin
	if(mode==13)
	{	
		struct nr* ret=min(versionsnummern, argc-optind);
		int retval=!ismin(&versionsnummern[0],ret);
		free(versionsnummern);
		return(!retval);
	}     
	free(versionsnummern);
	return(0);
}
