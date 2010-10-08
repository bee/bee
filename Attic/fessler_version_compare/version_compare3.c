#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <string.h>
#include <ctype.h>

struct nr {
	char* string;
        
	char* pkgname;
	char* subname;
        
	char* version;
	char* extraversion;
        
	int   extraversion_typ;
	char* extraversion_nr;
        
	char* pkgrevision;
	char* arch;
};

char cmp(struct nr *v1, struct nr *v2);
char parse_extra(struct nr *nr1);


int how_many(char z, char* str, int len)
{
	int i=0;
	int counter=0;
        
	for(i=0;i<len;i++) {
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
	//printf("%s%s%s%s%s%s\n",versionsnummer->pkgname,versionsnummer->subname,versionsnummer->version,versionsnummer->extraversion,versionsnummer->pkgrevision,versionsnummer->arch);
	return 0;	
}


char version_cmp(char *v1, char *v2) {
    char *a, *b;
    long long i,j;
    
    a = v1;
    b = v2;
    
    while(*a && *b && *a == *b) {
        a++;
        b++;
    }
    
    /* strings are equal ; *a==*b==0*/
    if(*a == *b)
        return(0);
    
    /* check *a is digit */
    if(isdigit(*a)) {
        if(isdigit(*b)) {
            i = atoll(a);
            j = atoll(b);
           
            if(i<j)
                return(-1);
            if(i>j)
                return(1);
                
            fprintf(stderr, "YOU HIT A BUG! #001\n");
            exit(254);
        }
        /* a > ('.',alpha, 0) */
        return(1);
    }
    
    /* check *a is alpha */
    if(isalpha(*a)) {
    
        /*  alpha < digit */
        if(isdigit(*b))
            return(-1);
        
        if(isalpha(*b)) {
            if(*a < *b)
                return(-1);
            return(1);
        }
        return(1);
    }
    
    if(! *b)
        return(1);
    
    return(-1);
}

char lt(struct nr *a, struct nr *b)
{	
    return(cmp(a,b) < 0);
}

char gt(struct nr *a, struct nr *b)
{
    return(cmp(a,b) > 0);
}

char le(struct nr *a, struct nr *b)
{
    return(cmp(a,b) <= 0);
}

char ge(struct nr *a, struct nr *b)
{	
    return(cmp(a,b) >= 0);
}

char ne(struct nr *a, struct nr *b)
{
    return(cmp(a,b) != 0);
}

char e(struct nr *a, struct nr *b)
{
    return(cmp(a,b) == 0);
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


char cmp(struct nr *v1, struct nr *v2) {
    char ret;

    ret = version_cmp(v1->version, v2->version);
    
    if(ret) return(ret);
    
    parse_extra(v1);
    parse_extra(v2);
    
    if(v1->extraversion_typ < v2->extraversion_typ)
        return(-1);

    if(v1->extraversion_typ > v2->extraversion_typ)
        return(1);
    
    if(v1->extraversion_nr && v2->extraversion_nr) {
        ret = version_cmp(v1->extraversion_nr, v2->extraversion_nr);
     
        if(ret) return(ret);
    }
    
    if(v1->pkgrevision && v2->pkgrevision) {
        ret = version_cmp(v1->pkgrevision, v2->pkgrevision);
        return(ret);
    }
    
    fprintf(stderr, "YOU HIT A BUG! 002\n");
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

struct nr* max(struct nr* array, int anzahl)
{
	int i=0;
	int retval=0;
	struct nr* akt=&array[0];
	for(i=1; i<anzahl;i++)
	{
		if(gt(&array[i], akt)) {
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
		if(lt(&array[i], akt)) {
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
        char keyvalue=0;
	int mode=0;
	int option_index = 0;
	int i=0;
	int c=0;
	int retval=0;
	char (*fp[])(struct nr *, struct nr *)={&lt,&gt,&le,&ge,&ne,&e};
        
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
                
		retval=fp[mode-1](&versionsnummern[0], &versionsnummern[1]);
                
                
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
		//standardmaessig alle ausgeben
		if(!show_pkgname && !show_subname && !show_version && !show_extraversion && !show_pkgrevision && !show_arch)
		{
			if(versionsnummern[0].pkgname!=NULL)
				printf("%s",versionsnummern[0].pkgname);
			if(versionsnummern[0].subname!=NULL)
				printf("_%s ",versionsnummern[0].subname);
			if(versionsnummern[0].version!=NULL)
				printf("-%s",versionsnummern[0].version);
			if(versionsnummern[0].extraversion!=NULL)
				printf("_%s ",versionsnummern[0].extraversion);
			if(versionsnummern[0].pkgrevision!=NULL)
				printf("-%s",versionsnummern[0].pkgrevision);
			if(versionsnummern[0].arch!=NULL)
				printf(".%s",versionsnummern[0].arch);
			printf("\n");
			free(versionsnummern);
			return 0;
		}	
			
			if(keyvalue && show_pkgname && versionsnummern[0].pkgname!=NULL)
				printf("PKGNAME=");
			if(show_pkgname && versionsnummern[0].pkgname!=NULL)
				printf("%s",versionsnummern[0].pkgname);
			if(keyvalue && show_pkgname && versionsnummern[0].subname!=NULL)
				printf("SUBNAME=");
			if(show_subname && versionsnummern[0].subname!=NULL)
				printf("_%s ",versionsnummern[0].subname);
			if(show_version && versionsnummern[0].version!=NULL)
				printf("-%s",versionsnummern[0].version);
			if(show_extraversion && versionsnummern[0].extraversion!=NULL)
				printf("_%s ",versionsnummern[0].extraversion);
			if(show_pkgrevision && versionsnummern[0].pkgrevision!=NULL)
				printf("-%s",versionsnummern[0].pkgrevision);
			if(show_arch && versionsnummern[0].arch!=NULL)
				printf(".%s",versionsnummern[0].arch);
			printf("\n");
		printf("\n");
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
			printf("_%s ",ret->extraversion);
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
			printf("_%s ",ret->extraversion);
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
