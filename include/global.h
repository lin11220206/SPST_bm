struct global_setting {
	char group;
	char change;
	char cut;
	char bit1;
	char bit2;
	char newID;
	char rebuild;
};

extern struct global_setting setting;

void set(char **);
void print_setting();