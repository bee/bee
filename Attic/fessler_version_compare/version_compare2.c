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

int parse_version(char *s,  struct nr *v)
{
  char *p;
v->string=strdup(s);
v->pkgname=NULL;
  
  /* p-v-r   v-r   v */
  
  if((p=strrchr(s, '-'))) {
     v->pkgrevision = p+1;
     *p=0;
     
     if(!*(v->pkgrevision)) {
         return(p-s+1);
     }
     
     if((p=strchr(v->pkgrevision, '.'))) {
         v->arch = p+1;
         *p=0;
         
         if(!*(v->arch) || !*(v->pkgrevision)) {
             return(p-s+1);
         }
     }
     
     if((p=strrchr(s, '-'))) {
         v->version = p+1;
         *p=0;
         
         v->pkgname = s;
         
         if(!*(v->pkgname) || *(v->pkgname) == '_') {
            return(1);
         }
     } else {
         v->version = s;
     }
  } else {
     v->version = s;
  }
  
  if(!*(v->version) || *(v->version) == '_') {
      return(p-s+1);
  }
  
  if((p=strchr(v->version, '_'))) {
      *p=0;
      v->extraversion=p+1;
      if(!*(v->extraversion)) {
          return(p-s+1);
      }
  }
  
  if(v->pkgname && (p=strchr(v->pkgname, '_'))) {
      *p=0;
      v->subname=p+1;
      if(!*(v->subname)) {
          return(p-s+1);
      }
  }
  
  return(0);
}

int parse_argument(char* text, int len, struct nr *versionsnummer)
{	
    int p;
    char *s;
    
    if( !(s = strdup(text))) {
        perror("s = strdup(text)");
        return(-1);
    }
    
    if((p=parse_version(text, versionsnummer))) {
        fprintf(stderr, "syntax error at position %d in '%s'\n", p, s);
        free(s);
        return(-1);
    }
    
    free(s);
    return(0);
}

char version_cmp(char *v1, char *v2) {
    char *a, *b;
    long long i,j;
    a = v1;
    b = v2;

    if(a==NULL && b==NULL)
    {
    	return(0);
    } 
    if(a!=NULL && b==NULL)
    {
    	return(1);
    }
    if(a==NULL && b!=NULL)
    {
    	return(-1);
    }    
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

//faengt str1 mit str2 an? pos gibt die Position an, an der der String danach weitergeht
char starts_with(char* str1, char* str2, int* pos)
{
	int i=0;
	while(str1[i]!='\0' && str2[i]!='\0')
	{
		if(str1[i]!=str2[i])
		{
			return 0;
		}
		if(pos!=NULL)
			*pos=i+1;
		i++;
	}
	return(str2[i]=='\0');
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
    
	ret = version_cmp(v1->pkgrevision, v2->pkgrevision);
	return ret;
	fprintf(stderr, "YOU HIT A BUG! 002\n");
	return 0;
}

char parse_extra(struct nr *nr1)
{
//alpha:2, beta 4, rc 6, keine extraversion 8, p 10, irgendwas anderes 12. Wir weiterer Text hinter dem Schluesselwort gefunden: Wert+=1
	if(nr1->extraversion==NULL)
	{	
		nr1->extraversion_typ=8;
		return 0;
	}
	nr1->extraversion_typ=-1;
	char extraversions[5][10]={{"alpha"}, {"beta"}, {"rc"},{"\n"},{"p"}};
	int i=0;
	int pos;
	//alle moeglichen Extraversionen durchgehen
	for(i=0;i<5;i++)
	{	
			//wenn eine Extraversion mit einem gueltigen Wort beginnt
			if(starts_with(nr1->extraversion,extraversions[i],&pos))
			{
				nr1->extraversion_typ=2*(i+1);
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
	struct nr* akt=&array[0];
	for(i=1; i<anzahl;i++)
	{
		if(gt(&array[i], akt)) 
			akt=&array[i];
	}
	return akt;
}
struct nr* min(struct nr* array, int anzahl)
{
	int i=0;
	struct nr* akt=&array[0];
	for(i=1; i<anzahl;i++)
	{
		if(lt(&array[i], akt)) 
			akt=&array[i];
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

void print_format(char* s, struct nr* v)
{
	int i=0;    
	while(s[i]!='\0')
	{
		if(s[i]=='%')
		{
			i++;
			if(s[i]=='p')
			{
				if(v->pkgname!=NULL)
					printf("%s ",v->pkgname);
			}
			else if(s[i]=='s')
			{
				if(v->subname!=NULL)
					printf("%s ",v->subname);
			}
			else if(s[i]=='v')
			{
				if(v->version!=NULL)
					printf("%s ",v->version);
			}
			else if(s[i]=='e')
			{
				if(v->extraversion!=NULL)
					printf("%s ",v->extraversion);
			}
			else if(s[i]=='r')
			{	
				if(v->pkgrevision!=NULL)
					printf("%s ",v->pkgrevision);
			}
			else if(s[i]=='a')
			{
				if(v->arch!=NULL)
					printf("%s ",v->arch);
			}
			else if(s[i]=='P')
			{
				if(v->pkgname!=NULL)
					printf("%s ",v->pkgname);
				if(v->subname!=NULL)
					printf("%s ",v->subname);
			}
			else if(s[i]=='V')
			{
				if(v->version!=NULL)
					printf("%s ",v->version);
				if(v->extraversion!=NULL)
					printf("%s ",v->extraversion);
			}
			else if(s[i]=='n')
			{
				printf("\n");
			}
			else if(s[i]=='%')
			{
				printf("%%");
			}
		}
		else
		{
			printf("%c", s[i]);
		}
			i++;
	}
}

int main(int argc, char **argv)
{
	char restriktion=0;
        char show_keyvalue=0;
	char* format=NULL;
	int mode=0;
	int option_index = 0;
	int i=0;
	int params=0;
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
	{"format", required_argument, 0, 14},
	{"pkgname", no_argument, 0, 'l'},
	{"pkgsubname", no_argument, 0, 'm'},
	{"pkgversion", no_argument, 0, 'n'},
	{"pkgextraversion", no_argument, 0, 'o'},
	{"pkgrevision", no_argument, 0, 'p'},
	{"pkgarch", no_argument, 0, 'q'},
	{"keyvalue", no_argument, 0, 'x'},
	{"pkgfullname", no_argument, 0, 's'},
	{"pkgfullversion", no_argument, 0, 't'},	
	{0, 0, 0, 0}
    };
	
    	while ((c = getopt_long(argc, argv, "",long_options, &option_index)) != -1)
	{
		if(c<15)
		{
			if(mode)
			{
				fprintf(stderr, "beeversion kann nur mit einem der Parameter --[lt, gt, le, ge, ne, e, parse, max, min, isnotmax, isnotmin, format=""] gestartet werden.\n");
			}
			mode=c;
		}
		if(c>='l' && c<='t')
			restriktion=1;
		
		switch (c) 
		{	case 14:
				format=strdup(optarg);
				break;
	
			case 'x':
				show_keyvalue=1;
				break;
		}
    	}
	//speicher reservieren
	params=argc-optind;
	struct nr *versionsnummern=calloc(params,sizeof(struct nr));
	if(versionsnummern==NULL)
	{
		printf("calloc error\n");
		return(-1);
	}
	//Anzahl Versionsnummern berechnen, bereitstellen und auf Gleichheit der Paketnamen pruefen
	for(i=optind; i < argc; i++)
	 {
		if(parse_argument(argv[i],strlen(argv[i]),&versionsnummern[i-optind])<0)
		{	
			retval=-1;
		}
		else if((versionsnummern[0].pkgname && !versionsnummern[i-optind].pkgname) || (!versionsnummern[0].pkgname && versionsnummern[i-optind].pkgname))
		{
			
			fprintf(stderr,"Es muessen alle oder keine Paketnamen vorhanden sein.\n");
			retval=-1;
			break;
		}
		else if(versionsnummern[0].pkgname && versionsnummern[i-optind].pkgname && strcmp(versionsnummern[0].pkgname,versionsnummern[i-optind].pkgname))
		{
 			fprintf(stderr,"Die Paketnamen (%s und %s)muessen gleich sein.\n",versionsnummern[0].pkgname,versionsnummern[i-optind].pkgname );
			retval=-1;
			break;
		}
	 }
	 if(retval<0)
	 {
 	 	for(i=0;i<params;i++)
 			free(versionsnummern[i].string);
		free(versionsnummern);
		return -1;
	 }
	 //kein Parameter angegeben
	 if(mode<1)
	 {
	 	mode=7;
	 }
	 //lt, gt, le, ge, ne, e
	if(mode<7)
	{
		if(argc-optind<2)
		{		
			fprintf(stderr, "Zwei Parameter werden benoetigt.\n");
			for(i=0;i<params;i++)
				free(versionsnummern[i].string);
			free(versionsnummern);
			return -1;
		}

		retval=fp[mode-1](&versionsnummern[0], &versionsnummern[1]);
	              
		return !retval;
	}
	//parse
	if(mode==7)
	{	
		if(argc-optind<1)
		{
			fprintf(stderr, "Ein Parameter wird benoetigt.\n");
			for(i=0;i<params;i++)
				free(versionsnummern[i].string);
			free(versionsnummern);
			return -1;
		}
		//standardmaessig alle ausgeben, wenn keine Restriktion fuer Ausgabe
		if(!restriktion && !show_keyvalue)
		{
			print_format("%p%s%v%e%r%a", &versionsnummern[0]);
		}
		//wenn --keyvalue aktiviert ist
		else if(show_keyvalue)
		{
				print_format("PKGNAME=%p%nPKGSUBNAME=%s%nPKGVERSION=%v%nPKGEXTRAVERSION=%e%nPKGREVISION=%r%nPKGARCH=%a", &versionsnummern[0]);	
		}
		else
		{
			i=1;
			while(i<optind)
			{					
				if(starts_with("--pkgname",argv[i],NULL))
					print_format("%p",&versionsnummern[0]);
				if(starts_with("--pkgsubname",argv[i],NULL))
					print_format("%s",&versionsnummern[0]);	
				if(starts_with("--pkgversion",argv[i],NULL))
					print_format("%v",&versionsnummern[0]);
				if(starts_with("--pkgextraversion",argv[i],NULL))
					print_format("%e",&versionsnummern[0]);
				if(starts_with("--pkgrevision",argv[i],NULL))
					print_format("%r",&versionsnummern[0]);
				if(starts_with("--pkgarch",argv[i],NULL))
					print_format("%a",&versionsnummern[0]);
				if(starts_with("--pkgfullname",argv[i],NULL))
					print_format("%P",&versionsnummern[0]);
				if(starts_with("--pkgfullversion",argv[i],NULL))
					print_format("%V",&versionsnummern[0]);
			i++;
			}
		}
	}
	//max
	if(mode==8)
	{	
		if(argc-optind<2)
		{		
			fprintf(stderr, "Zwei Parameter werden benoetigt.\n");
			free(versionsnummern);
			for(i=0;i<params;i++)
				free(versionsnummern[i].string);
			return -1;
		}
		
		struct nr* ret=max(versionsnummern, argc-optind);
		printf("%s",ret->string);
	}
	//min
	if(mode==9)
	{	
		if(argc-optind<2)
		{		
			fprintf(stderr, "Zwei Parameter werden benoetigt.\n");
			free(versionsnummern);
			for(i=0;i<params;i++)
				free(versionsnummern[i].string);
			return -1;
		}
	
		struct nr* ret=min(versionsnummern, argc-optind);
		printf("%s",ret->string);
	}
	//ismax
	if(mode==10)
	{	
		struct nr* ret=max(versionsnummern, argc-optind);
		int retval=ismax(&versionsnummern[0],ret);
		for(i=0;i<params;i++)
			free(versionsnummern[i].string);
		free(versionsnummern);
		return(!retval);
	}
	//ismin
	if(mode==11)
	{	
		struct nr* ret=min(versionsnummern, argc-optind);
		int retval=ismin(&versionsnummern[0],ret);
		for(i=0;i<params;i++)
			free(versionsnummern[i].string);
		free(versionsnummern);
		return(!retval);
	}
	//isnotmax
	if(mode==12)
	{	
		struct nr* ret=max(versionsnummern, argc-optind);
		int retval=!ismax(&versionsnummern[0],ret);
		for(i=0;i<params;i++)
			free(versionsnummern[i].string);
		free(versionsnummern);
		return(!retval);
	}
	//isnotmin
	if(mode==13)
	{	
		struct nr* ret=min(versionsnummern, argc-optind);
		int retval=!ismin(&versionsnummern[0],ret);
		for(i=0;i<params;i++)
			free(versionsnummern[i].string);
		free(versionsnummern);
		return(!retval);
	}
	//format
	if(mode==14)
	{	
		print_format(format, &versionsnummern[0]);
	}
	printf("\n");

	for(i=0;i<params;i++)
		free(versionsnummern[i].string);
	free(format);
	free(versionsnummern);
	return(0);
}