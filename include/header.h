struct ENTRY {
    unsigned int srcIP, dstIP;
	unsigned char srclen, dstlen, proto, protolen;
	unsigned short int srcPort[2], dstPort[2], type;
	int rule, group, n;
};

extern int num_entry;
extern struct ENTRY *table;

void read_table(char *, int);
void set_table(char *);
