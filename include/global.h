struct global_setting {
	char cut;
	char bit1;
	char bit2;
	char change;
	char newID;
	char rebuild;
	char ignore;
};

extern struct global_setting setting[4];

void set(char *);
void print_setting();