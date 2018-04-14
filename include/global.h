struct global_setting {
	char cut;
	char bit1;
	char bit2;
	char change;
	char newID;
	char rebuild;
	char ignore;
	char dim1_only;
};

extern struct global_setting setting[3];

void set(char *);
void print_setting();