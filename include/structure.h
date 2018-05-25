struct level1 {
    unsigned int *endpoint;
    struct level2 *lv2;
    int n, *n1; // n = total endpoint of level 1, n1 = number of each interval
    int *rule, r;
};

struct level2 {
    unsigned int *endpoint;
    struct bucket *b0;
    struct bucket **b;
    int type;
    int *b_type;
    int n, *n2, *rule, r; // n2 = number of each interval
};

struct bucket {
    int *rule, r, set, *rule2, r2; //r, r2: size of rule[], rule2[]
    unsigned int BV;
    int mergeID;
};

extern struct level1 gp[6][65536];
extern struct bucket gp_A[4][4];
extern struct level1 gp_AA;
extern int thres[6], thres2[6], numcombine;
extern int thres_A;
extern int group[6][65536], groupp[6];
extern int seg_bit[6];
extern struct bucket *uni_dim1_bucket[500000];
extern struct bucket *uni_dim2_bucket[500000];
extern int uni_num[2];
extern struct bucket merge_bucket[6][100000];
extern int mrg_num[6];
extern struct ENTRY *table3;

void analysis();
void groupping();
void group_A();
void group_AA();
void first_level();
void second_level();
void convert();
void l1_bucket_share();
void l2_bucket_share();
void bucket_merge();
void rebuild();
void pre_rebuild();
int cmp(const void *, const void *);