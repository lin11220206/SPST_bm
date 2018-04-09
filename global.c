#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "global.h" 

struct global_setting setting[4];

void set(char *filename){
	FILE *fp;
	char str[100];
	char buf[10];
	char tok[] = " \t\n";
	fp = fopen(filename, "r");
	int i = 0;

	while(fgets(str, 100, fp) != NULL) {
		if(i>4) break;

		if(str[0] == '@') continue;

		sprintf(buf, "%s%c", strtok(str, tok), '\0');
		setting[i].cut = atoi(buf);
		sprintf(buf, "%s%c", strtok(NULL, tok), '\0');
		setting[i].bit1 = atoi(buf);
		sprintf(buf, "%s%c", strtok(NULL, tok), '\0');
		setting[i].bit2 = atoi(buf);
		sprintf(buf, "%s%c", strtok(NULL, tok), '\0');
		setting[i].change = atoi(buf);
		sprintf(buf, "%s%c", strtok(NULL, tok), '\0');
		setting[i].newID = atoi(buf);
		sprintf(buf, "%s%c", strtok(NULL, tok), '\0');
		setting[i].rebuild = atoi(buf);
		sprintf(buf, "%s%c", strtok(NULL, tok), '\0');
		setting[i].ignore = atoi(buf);
		sprintf(buf, "%s%c", strtok(NULL, tok), '\0');
		setting[i].dim1_only = atoi(buf);

		i++;
	}
}

void print_setting(){
	int i;
	for(i=0; i<4; i++) {
		printf("%d ", setting[i].cut);
		printf("%d ", setting[i].bit1);
		printf("%d ", setting[i].bit2);
		printf("%d ", setting[i].change);
		printf("%d ", setting[i].newID);
		printf("%d ", setting[i].rebuild);
		printf("%d ", setting[i].ignore);
		printf("%d\n", setting[i].dim1_only);
	}
}