#include <stdlib.h>
#include <stdio.h>
#include "global.h" 

struct global_setting setting;

void set(char **arg){
	setting.group = arg[2][0];
	if(setting.group == 'B') setting.group = 0;
	else if(setting.group == 'C') setting.group = 1;
	else if(setting.group == 'D') setting.group = 2; 
	else setting.group = -1;

	setting.change = atoi(arg[3]);
	setting.cut = 16;
	//setting.cut = atoi(arg[4]);
	setting.bit1 = atoi(arg[4]);
	setting.bit2 = atoi(arg[5]);
	setting.newID = atoi(arg[6]);
	setting.rebuild = 0;
}

void print_setting(){
	char group;

	if(setting.group == 0) group = 'B';
	else if(setting.group == 1) group = 'C';
	else if(setting.group == 2) group = 'D';
	else group = '?';

	printf("select group: %c\n", group);
	printf("change: %c\n", (setting.change)? 'Y': 'N');
	printf("cut at %d-bit\n", setting.cut);
	printf("group %c(len >= %d) use %d-bit segmentation table\n", group, setting.cut, setting.bit1);
	printf("group %c'(len < %d) use %d-bit segmentation table\n", group, setting.cut, setting.bit2);
	if(setting.newID)
		printf("With NewID Mapping\n");
	else
		printf("Without NewID Mapping\n");
}