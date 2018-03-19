struct global_setting {
	char group;
	char change;
	char bit1;
	char bit2;
};

extern struct global_setting setting;

void set(char **);
void print_setting();