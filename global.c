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
	setting.bit1 = atoi(arg[4]);
	setting.bit2 = atoi(arg[5]);
}

void print_setting(){
	char group;

	if(setting.group == 0) group = 'B';
	else if(setting.group == 1) group = 'C';
	else if(setting.group == 2) group = 'D';
	else group = '?';

	printf("select group: %c\n", group);
	printf("change: %c\n", (setting.change)? 'Y': 'N');
	printf("group %c use %d-bit segmentation table\n", group, setting.bit1);
	printf("group %c' use %d-bit segmentation table\n", group, setting.bit2);
}