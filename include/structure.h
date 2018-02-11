struct ENTRY3 {
    unsigned short int Port[4], type, n;
    unsigned char proto;
};

struct level1 {
    unsigned int *endpoint;
    struct level2 *lv2;
    int n, *n1; // n = total endpoint of level 1, n1 = number of each interval
    int *rule, r;
};

struct level2 {
    unsigned int *endpoint;
    struct bucket **b;
    int type;
    int *b_type;
    int n, *n2, *rule, r; // n2 = number of each interval
};

struct bucket {
    int *rule, r, set, *rule2, r2; //r, r2: size of rule[], rule2[]
    unsigned int BV;
};

extern struct level1 gp[3][65536];
extern int thres[3], thres2[3], thres3[4], count2[4], numcombine;
extern int group[3][65536], groupp[3];
extern struct bucket *uni_bucket[500000];
extern int uni_num;
extern struct bucket *merge_bucket[500000];
extern int mrg_num;
extern struct ENTRY3 *table3;

void groupping();
void first_level();
void second_level();
void convert();
void l1_bucket_share();
void l2_bucket_share();
void buekct_merge();
int cmp(const void *, const void *);
//void software_compress();
