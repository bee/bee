#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
int main(int argc, char** argv)
{
	char gl=0;	//gibt an, ob ein Gleichzeichen gefunden wurde
	char dp=0;	//gibt an, ob ein Doppelpunkt gefunden wurde
	int dp_pos=0;	//gibt die Position des letzten Doppelpunktes an
	int segmente=0;	//Anzahl der Segmente, in die der String sich teilen laesst
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