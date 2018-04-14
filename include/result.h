struct result {
	int *dim1_count;
	int *dim2_count;
	int *merge_count;
	int total_rules[6];
	int dim1_buckets[6][2];
	int dim2_buckets[6][2];
	int merge_buckets[6];
	int group_A_buckets;
	int bucket_size[6];
	int dim1_nodes[6][20];
	int dim2_nodes[6][20];
	int dim1_roots[6];
	int dim1_rules[6];
};

struct result_A {
	int total_rules;
	int buckets;
	int bucket_size;
};

struct memory {
	int dim1_node_ptr[6][20];
	int dim2_node_ptr[6][20];
	int bucket_ptr[6];
	int seg_table[6];
	int total[6][2];
	int group_A_use;
	int sum_bit;
	double sum_byte;
	double byte_per_rule;
};

extern struct result data;
extern struct result_A data_A;
extern struct memory memory_use;

void get_result();
void compute_memory_use();

void show_duplication(char);
void show_buckets_data();
void show_nodes_count(char);
void show_memory_use();

void show_prefix_length(char);

void level1_duplication(int);
void level2_duplication(int);
void merge_duplication(int);
void merge_duplication2(int);
void node_count(char);
void result1(char);
void result2(int);
void result3(char);