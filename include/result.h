struct result {
	int *dim1_count;
	int *dim2_count;
	int *merge_count;
	int total_rules[8];
	int dim1_buckets[8][2];
	int dim2_buckets[8][2];
	int merge_buckets[8];
	int bucket_size[8];
	int dim1_nodes[8][20];
	int dim2_nodes[8][20];
	int dim1_roots[8];
};

struct memory {
	int dim1_node_ptr[8][20];
	int dim2_node_ptr[8][20];
	int bucket_ptr[8];
	int seg_table[8];
	int total[8][2];
	int sum_bit;
	double sum_byte;
	double byte_per_rule;
};

extern struct result data;
extern struct memory memory_use;

void get_result();
void compute_memory_use();

void show_duplication(char);
void show_buckets_data();
void show_nodes_count(char);
void show_memory_use();

void level1_duplication(int);
void level2_duplication(int);
void merge_duplication(int);
void merge_duplication2(int);
void node_count(char);
void result1(char);
void result2(int);
void result3(char);