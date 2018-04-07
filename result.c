#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "header.h"
#include "structure.h"
#include "function.h"
#include "result.h"
#include "global.h"

struct result data;
struct memory memory_use;

void get_result() {
    printf("start getting result ...\n");

    int i, na, j, k, r, s, ruleID, newID, N, g;
    int total, total2;

    data.dim1_count = malloc(num_entry * sizeof(int));
    data.dim2_count = malloc(num_entry * sizeof(int));
    data.merge_count = malloc(num_entry * sizeof(int));

    for (i = 0; i < num_entry; i++) {
        data.dim1_count[i] = 0;
        data.dim2_count[i] = 0;
        data.merge_count[i] = 0;
    }

    for (i = 0; i < 8; i++) {
        data.total_rules[i] = groupp[i];
        data.dim1_buckets[i][0] = 0;
        data.dim1_buckets[i][1] = 0;
        data.dim2_buckets[i][0] = 0;
        data.dim2_buckets[i][1] = 0;
        data.merge_buckets[i] = 0;
        data.bucket_size[i] = thres2[i];

        for (j = 0; j < 20; j++) {
            data.dim1_nodes[i][j] = 0;
            data.dim2_nodes[i][j] = 0;
        }
        data.dim1_roots[i] = 0;
    }

    struct ENTRY *table_use = table3;
    int num = numcombine;

    int **cnt = malloc(num_entry * sizeof(int *));
    for (i = 0; i < num_entry; i++) {
        cnt[i] = malloc(100 * sizeof(int));
    }

    for (i = 0; i < num_entry; i++) {
        for (j = 0; j < 100; j++)
            cnt[i][j] = -1;
    }

    for (i = 0; i < 8; i++) {
        for (na = 0; na < 65536; na++) {

            if (group[i][na] > 0) data.dim1_roots[i]++;

            N = gp[i][na].n;

            for (j = 1; j < N; j++) {
                if (gp[i][na].lv2[j].type != 1) continue;
                for (k = 0; k < gp[i][na].lv2[j].r; k++) {
                    ruleID = gp[i][na].lv2[j].rule[k];
                    data.dim1_count[ruleID]++;
                }
                for (k = 1; k < gp[i][na].lv2[j].n; k++) {
                    if (gp[i][na].lv2[j].b_type[k] != 1) continue;
                    for (r = 0; r < gp[i][na].lv2[j].b[k]->r; r++) {
                        ruleID = gp[i][na].lv2[j].b[k]->rule[r];

                        data.dim2_count[ruleID]++;

                        for (s = 0; s < data.merge_count[ruleID]; s++) {
                            if (cnt[ruleID][s] == gp[i][na].lv2[j].b[k]->mergeID) {
                                break;
                            }
                        }

                        if (s == data.merge_count[ruleID]) {
                            cnt[ruleID][s] = gp[i][na].lv2[j].b[k]->mergeID;
                            if (data.merge_count[ruleID] == 99) printf("alert!!\n");
                            data.merge_count[ruleID]++;
                        }
                    }
                }
            }
        }
    }

    for (i = 0; i < 8; i++) {
        for (na = 0; na < 65536; na++) {
            N = gp[i][na].n;
            if ( N < 1) continue;
            total = 0;
            for (j = 1; j < N; j++) {
                if (gp[i][na].lv2[j].n > 0)
                    total++;
                if (gp[i][na].lv2[j].n > 1 && gp[i][na].lv2[j].type == 1) {

                    total2 = gp[i][na].lv2[j].n - 1;
                    if (gp[i][na].lv2[j].endpoint[total2] == 0xFFFFFFFF)
                        total2--;
                    layer_count(total2, data.dim2_nodes[i]);
                }
            }
            if (gp[i][na].endpoint[total] == 0xFFFFFFFF)
                total--;

            layer_count(total, data.dim1_nodes[i]);
        }
    }

    for (i = 0; i < 8; i++) {
        for (na = 0; na < 65536; na++) {
            N = gp[i][na].n;

            for (j = 1; j < N; j++) {
                if (gp[i][na].lv2[j].type == 1 && gp[i][na].lv2[j].n > 1) data.dim1_buckets[i][1]++;
                if (gp[i][na].lv2[j].n > 1) {
                    data.dim1_buckets[i][0]++;
                    data.dim2_buckets[i][0] += gp[i][na].lv2[j].n - 1;
                }
            }

        }
    }
    for (i = 0; i < uni_num; i++) {
        ruleID = uni_bucket[i]->rule[0];
        g = table[ruleID].group;

        data.dim2_buckets[g][1]++;
    }

    for (i = 0; i < 8; i++) {
        for (j = 0; j < mrg_num[i]; j++) {
            ruleID = merge_bucket[i][j].rule[0];

            data.merge_buckets[i]++;
        }
    }

    return;
}

void compute_memory_use() {
    int i, j, g, bit;
    int n1 = 0, n2 = 0;

    for (g = 0; g < 8; g++) {

        for (i = 0; i < 20; i++) {
            memory_use.dim1_node_ptr[g][i] = 0;
            memory_use.dim2_node_ptr[g][i] = 0;
        }

        memory_use.total[g][0] = 0;
        memory_use.total[g][1] = 0;
        memory_use.bucket_ptr[g] = 0;
        memory_use.seg_table[g] = 0;
    }

    memory_use.sum_bit = 0;
    memory_use.sum_byte = 0;
    memory_use.byte_per_rule = 0;

    for (g = 0; g < 8; g++) {

        if (g < 4) bit = setting[g].bit1;
        else bit = setting[g - 4].bit2;

        memory_use.bucket_ptr[g] = data.dim2_buckets[g][1] * (ceil_log2(data.merge_buckets[g]) + data.bucket_size[g]);
        memory_use.seg_table[g] = (1 << bit) * ceil_log2(data.dim1_roots[g]);

        for (i = 19; i >= 0; i--) {
            if (data.dim1_nodes[i] > 0) {
                n1 = i;
                break;
            }
        }

        for (i = 19; i >= 0; i--) {
            if (data.dim2_nodes[i] > 0) {
                n2 = i;
                break;
            }
        }

        for (i = 0; i < n1; i++) {
            memory_use.dim1_node_ptr[g][i] = data.dim1_nodes[g][i] * (ceil_log2(data.dim1_nodes[g][i + 1]) + ceil_log2(data.dim1_buckets[g][1]) + 32 + 2);
        }
        memory_use.dim1_node_ptr[g][i] = data.dim1_nodes[g][i] * (ceil_log2(data.dim1_buckets[g][1]) + 32 + 2);

        for (i = 0; i < n2; i++) {
            memory_use.dim2_node_ptr[g][i] = data.dim2_nodes[g][i] * (ceil_log2(data.dim2_nodes[g][i + 1]) + ceil_log2(data.dim2_buckets[g][1]) + 32 + 2);
        }
        memory_use.dim2_node_ptr[g][i] = data.dim2_nodes[g][i] * (ceil_log2(data.dim2_buckets[g][1]) + 32 + 2);
    }

    for (g = 0; g < 8; g++) {

        for (i = 0; i < 20; i++) {
            memory_use.sum_bit += memory_use.dim1_node_ptr[g][i];
            memory_use.sum_bit += memory_use.dim2_node_ptr[g][i];

            memory_use.total[g][0] += memory_use.dim1_node_ptr[g][i];
            memory_use.total[g][1] += memory_use.dim2_node_ptr[g][i];
        }
        memory_use.sum_bit += memory_use.bucket_ptr[g];
        memory_use.sum_bit += memory_use.seg_table[g];
    }

    memory_use.sum_byte = memory_use.sum_bit / 8;
    memory_use.byte_per_rule = memory_use.sum_byte / num_entry;
}

void show_duplication(char g) {

    int i;
    int max[3] = {0};
    double sum[3] = {0};
    double avg[3] = {0};

    int dis[3][20] = {0};

    for (i = 0; i < 20; i++) {
        dis[0][i] = 0;
        dis[1][i] = 0;
        dis[2][i] = 0;
    }

    for (i = 0; i < num_entry; i++) {
        if (table[i].group != g) continue;

        dis[0][log_2(data.dim1_count[i])]++;
        dis[1][log_2(data.dim2_count[i])]++;
        dis[2][log_2(data.merge_count[i])]++;

        sum[0] += data.dim1_count[i];
        sum[1] += data.dim2_count[i];
        sum[2] += data.merge_count[i];

        if (data.dim1_count[i] > max[0]) max[0] = data.dim1_count[i];
        if (data.dim2_count[i] > max[1]) max[1] = data.dim2_count[i];
        if (data.merge_count[i] > max[2]) max[2] = data.merge_count[i];
    }

    for (i = 0; i < 3; i++) {
        if (data.total_rules[g] > 0)
            avg[i] = sum[i] / data.total_rules[g];
    }

    char gName[10];

    if (g < 4) sprintf(gName, "%c(%d-bit)", g + 'A', setting[g].bit1);
    else sprintf(gName, "%c'(%d-bit)", g - 4 + 'A', setting[g - 4].bit2);

    printf("%s\n", gName);

    int n = 0;
    for (i = 19; i >= 0; i--) {
        if (dis[0][i] > 0 || dis[1][i] > 0 || dis[2][i] > 0) {
            n = i;
            break;
        }
    }

    printf("%d\n", data.total_rules[g]);
    for (i = 0; i <= n; i++) {
        printf(", %d, %d, %d\n", dis[0][i], dis[1][i], dis[2][i]);
    }
    printf("avg., %.2f, %.2f, %.2f\n", avg[0], avg[1], avg[2]);
    printf("max, %d, %d, %d\n", max[0], max[1], max[2]);
    printf("\n=========================\n");

    return;
}

void show_buckets_data() {
    int i;

    for (i = 0; i < 4; i++) {
        printf("%c(%d-bit), ", i+'A', setting[i].bit1);
        printf("%c'(%d-bit), ", i+'A', setting[i].bit2);
    }
    printf("\n");

    for (i = 0; i < 4; i++) {
        printf("%d, ", data.total_rules[i]);
        printf("%d, ", data.total_rules[i+4]);
    }
    printf("\n");

    for(i=0; i<4; i++) {
        printf("%d, ", data.dim1_roots[i]);
        printf("%d, ", data.dim1_roots[i+4]);
    }
    printf("\n");

    for (i = 0; i < 4; i++) {
        printf("%d, ", data.dim1_buckets[i][0]);
        printf("%d, ", data.dim1_buckets[i+4][0]);
    }
    printf("\n");

    for (i = 0; i < 4; i++) {
        printf("%d, ", data.dim1_buckets[i][1]);
        printf("%d, ", data.dim1_buckets[i+4][1]);
    }
    printf("\n");

    for (i = 0; i < 4; i++){
        printf("%d, ", data.dim2_buckets[i][0]);
        printf("%d, ", data.dim2_buckets[i+4][0]);
    }
    printf("\n");

    for (i = 0; i < 4; i++){
        printf("%d, ", data.dim2_buckets[i][1]);
        printf("%d, ", data.dim2_buckets[i+4][1]);
    }
    printf("\n");

    for (i = 0; i < 4; i++) {
        printf("%d, ", data.merge_buckets[i]);
        printf("%d, ", data.merge_buckets[i+4]);
    }
    printf("\n");

    for (i = 0; i < 4; i++) {
        printf("%d, ", data.bucket_size[i]);
        printf("%d, ", data.bucket_size[i+4]);
    }
    printf("\n");


    printf("\n=========================\n");
    return;
}

void show_nodes_count(char g) {
    int i, n = 0;
    char gName[10];
    int total[2] = {0};

    if (g < 4) sprintf(gName, "%c(%d-bit)", g + 'A', setting[g].bit1);
    else sprintf(gName, "%c'(%d-bit)", g - 4 + 'A', setting[g - 4].bit2);

    printf("%s\n", gName);

    for (i = 19; i >= 0; i--) {
        if (data.dim1_nodes[g][i] > 0 || data.dim2_nodes[g][i] > 0) {
            n = i;
            break;
        }
    }

    for (i = 0; i <= n; i++) {
        printf("level %d, %d, %d\n", i, data.dim1_nodes[g][i], data.dim2_nodes[g][i]);
        total[0] += data.dim1_nodes[g][i];
        total[1] += data.dim2_nodes[g][i];
    }
    printf("total, %d, %d\n", total[0], total[1]);
    printf("\n=========================\n");
    return;
}

void show_memory_use() {

    int i;

    for (i = 0; i < 4; i++) {
        printf("%c(%d-bit), ", i+'A', setting[i].bit1);
        printf("%c'(%d-bit), ", i+'A', setting[i].bit2);
    }
    printf("\n");

    for (i = 0; i < 4; i++) {
        printf("%d, ", memory_use.seg_table[i]);
        printf("%d, ", memory_use.seg_table[i+4]);
    }
    printf("\n");

    for (i = 0; i < 4; i++) {
        printf("%d, ", memory_use.total[i][0]);
        printf("%d, ", memory_use.total[i+4][0]);
    }
    printf("\n");

    for (i = 0; i < 4; i++) {
        printf("%d, ", memory_use.total[i][1]);
        printf("%d, ", memory_use.total[i+4][1]);
    }
    printf("\n");

    for (i = 0; i < 4; i++) {
        printf("%d, ", memory_use.bucket_ptr[i]);
        printf("%d, ", memory_use.bucket_ptr[i+4]);
    }
    printf("\n");

    printf("total(bit), %d\n", memory_use.sum_bit);
    printf("total(byte), %.0f\n", memory_use.sum_byte);
    printf("avg(per rule), %.02f\n", memory_use.byte_per_rule);

    printf("\n=========================\n");
    return;
}

/*
void level1_duplication(int s) {
    int i, na, j, k;
    l1_count = calloc(num_entry , sizeof(int));

    for (i = 0; i < num_entry; i++)
        l1_count[i] = 0;

    int ruleID, N;
    for (i = 0; i < 6; i++) {
        for (na = 0; na < 65536; na++) {
            N = gp[i][na].n;

            for (j = 1; j < N; j++) {
                if (gp[i][na].lv2[j].type != 1) continue;
                for (k = 0; k < gp[i][na].lv2[j].r; k++) {
                    ruleID = gp[i][na].lv2[j].rule[k];

                    l1_count[ruleID]++;
                }
            }
        }
    }

    int max = 0;
    int total = 0;
    double sum = 0;
    double avg;
    int l1_dis[20] = {0};
    for (i = 0; i < num_entry; i++) {
        if (table[i].group != s) continue;

        l1_dis[log_2(l1_count[i])]++;
        total++;

        sum += l1_count[i];

        if (l1_count[i] > max) max = l1_count[i];
    }

    if (s < 3) printf("group %c(%d-bit)\n", s + 'B', setting[0].bit1);
    else printf("group %c'(%d-bit)\n", s - 3 + 'B', setting[0].bit2);

    printf("%d\n", total);
    for (i = 11; i < 20; i++)
        l1_dis[10] += l1_dis[i];

    for (i = 0; i < 11; i++)
        printf("%d\n", l1_dis[i]);

    if (total == 0)
        avg = 0;
    else
        avg = sum / total;
    printf("%.2f\n", avg);
    printf("%d\n", max);

    printf("\n\n");
}
void level2_duplication(int s) {
    int i, na, j, k, r;

    l2_count = calloc(num_entry , sizeof(int));

    for (i = 0; i < num_entry; i++)
        l2_count[i] = 0;

    int ruleID, N;

    for (i = 0; i < 6; i++) {
        for (na = 0; na < 65536; na++) {
            N = gp[i][na].n;
            for (j = 1; j < N; j++) {
                for (k = 1; k < gp[i][na].lv2[j].n; k++) {
                    if (gp[i][na].lv2[j].b_type[k] != 1) continue;
                    for (r = 0; r < gp[i][na].lv2[j].b[k]->r; r++) {
                        ruleID = gp[i][na].lv2[j].b[k]->rule[r];

                        l2_count[ruleID]++;
                    }
                }
            }
        }
    }

    int max = 0;
    int total = 0;
    double sum = 0;
    double avg;
    int l2_dis[20] = {0};
    for (i = 0; i < num_entry; i++) {
        if (table[i].group != s) continue;

        l2_dis[log_2(l2_count[i])]++;
        total++;

        sum += l2_count[i];

        if (l2_count[i] > max) max = l2_count[i];
    }

    if (s < 3) printf("group %c(%d-bit)\n", s + 'B', setting[0].bit1);
    else printf("group %c'(%d-bit)\n", s - 3 + 'B', setting[0].bit2);

    printf("%d\n", total);
    for (i = 11; i < 20; i++)
        l2_dis[10] += l2_dis[i];

    for (i = 0; i < 11; i++)
        printf("%d\n", l2_dis[i]);

    if (total == 0)
        avg = 0;
    else
        avg = sum / total;
    printf("%.2f\n", avg);
    printf("%d\n", max);

    printf("\n\n");
}
void merge_duplication(int s) {
    int i, j, ruleID, newID, N;
    int na, k, r;

    struct ENTRY *table_use;
    int num;
    if (setting[0].newID) {
        table_use = table3;
        num = numcombine;
    }
    else {
        table_use = table;
        num = num_entry;
    }

    int **cnt;

    cnt = malloc(num_entry * sizeof(int *));
    for (i = 0; i < num_entry; i++) {
        cnt[i] = malloc(100 * sizeof(int));
    }


    //int cnt[num_entry][100] ;
    for (i = 0; i < num_entry; i++) {
        for (j = 0; j < 100; j++)
            cnt[i][j] = -1;
    }
    merge_count = malloc(num_entry * sizeof(int));

    for (i = 0; i < num_entry; i++)
        merge_count[i] = 0;

    for (na = 0; na < 65536; na++) {
        N = gp[s][na].n;
        for (j = 1; j < N; j++) {
            for (k = 1; k < gp[s][na].lv2[j].n; k++) {
                if (gp[s][na].lv2[j].b_type[k] != 1) continue;
                for (r = 0; r < gp[s][na].lv2[j].b[k]->r; r++) {
                    ruleID = gp[s][na].lv2[j].b[k]->rule[r];

                    //merge_count[ruleID]++;

                    for (i = 0; i < merge_count[ruleID]; i++) {
                        if (cnt[ruleID][i] == gp[s][na].lv2[j].b[k]->mergeID) {
                            break;
                        }
                    }
                    if (i == merge_count[ruleID]) {
                        cnt[ruleID][i] = gp[s][na].lv2[j].b[k]->mergeID;
                        if (merge_count[ruleID] == 99) printf("alert!!\n");
                        merge_count[ruleID]++;
                    }
                }
            }

        }
    }

    for (i = 0; i < mrg_num; i++) {
        ruleID = merge_bucket[i]->rule[0];
        if(table_use[ruleID].group != s) continue;

        for (j = 0; j < merge_bucket[i]->r; j++) {

            merge_count[merge_bucket[i]->rule[j]]++;
        }
        //printf("\n");
    }

    int max = 0;
    int total = 0;
    double sum = 0;
    double avg;
    int merge_dis[20] = {0};

    for (i = 0; i < num_entry; i++) {
        if (table[i].group != s) continue;

        merge_dis[log_2(merge_count[i])]++;
        total++;

        sum += merge_count[i];

        if (merge_count[i] > max) max = merge_count[i];
    }

    if (s < 3) printf("group %c(%d-bit)\n", s + 'B', setting[0].bit1);
    else printf("group %c'(%d-bit)\n", s - 3 + 'B', setting[0].bit2);

    int total2 = 0;
    for (i = 0; i < mrg_num[s]; i++) {
        ruleID = merge_bucket[s][i].rule[0];
        if (table_use[ruleID].group != s) continue;

        total2++;
    }

    printf("bucket_num %d %d\n", total2, mrg_num[s]);
    printf("bucket_size: %d\n\n", thres2[s]);


    printf("%d\n", total);
    for (i = 11; i < 20; i++)
        merge_dis[10] += merge_dis[i];

    for (i = 0; i < 11; i++)
        printf("%d\n", merge_dis[i]);

    if (total == 0)
        avg = 0;
    else
        avg = sum / total;
    printf("%.2f\n", avg);
    printf("%d\n", max);

    printf("\n\n");
}

void merge_duplication2(int s) {
    int i, j, ruleID;

    merge_count = malloc(num_entry * sizeof(int));

    for (i = 0; i < num_entry; i++)
        merge_count[i] = 0;

    for (i = 0; i < mrg_num[s]; i++) {
        ruleID = merge_bucket[s][i].rule[0];

        for (j = 0; j < merge_bucket[s][i].r; j++) {

            merge_count[merge_bucket[s][i].rule[j]]++;
        }
        //printf("\n");
    }

    int max = 0;
    int total = 0;
    double sum = 0;
    double avg;
    int merge_dis[20] = {0};

    for (i = 0; i < num_entry; i++) {
        if (table[i].group != s) continue;

        merge_dis[log_2(merge_count[i])]++;
        total++;

        sum += merge_count[i];

        if (merge_count[i] > max) max = merge_count[i];
    }

    if (s < 3) printf("group %c(%d-bit)\n", s + 'B', setting[0].bit1);
    else printf("group %c'(%d-bit)\n", s - 3 + 'B', setting[0].bit2);

    printf("%d\n", total);
    for (i = 11; i < 20; i++)
        merge_dis[10] += merge_dis[i];

    for (i = 0; i < 11; i++)
        printf("%d\n", merge_dis[i]);

    if (total == 0)
        avg = 0;
    else
        avg = sum / total;
    printf("%.2f\n", avg);
    printf("%d\n", max);

    printf("\n\n");
}

void node_count(char g) {
    int i, j, na, N, ruleID;
    int total = 0, total2 = 0;
    int n1, n2;

    int dim1_node[20] = {0};
    int dim2_node[20] = {0};

    int dim1_memory[20] = {0};
    int dim2_memory[20] = {0};

    int bucket[2] = {0};

    int count = 0;

    for(na=0; na<65536; na++){
        N = gp[g][na].n;
        total = 0;
        for(j=1; j< N; j++) {
            if(gp[g][na].lv2[j].n > 0)
                total++;
            if(gp[g][na].lv2[j].n > 1 && gp[g][na].lv2[j].type == 1) { //&& gp[g][na].lv2[j].type == 1
                bucket[0]++;
                total2 = gp[g][na].lv2[j].n - 1;
                if(gp[g][na].lv2[j].endpoint[total2] == 0xFFFFFFFF)
                    total2--;
                layer_count(total2, dim2_node);
            }
        }
        if(gp[g][na].endpoint[total] == 0xFFFFFFFF)
            total--;
        layer_count(total, dim1_node);
    }

    for(i=0; i<uni_num; i++){
        ruleID = uni_bucket[i]->rule[0];
        if( table[ruleID].group != g) continue;
        bucket[1]++;
    }


    for(i=19; i>=0; i--) {
        if(dim1_node[i] > 0){
            n1 = i;
            break;
        }
    }

    for(i=19; i>=0; i--) {
        if(dim2_node[i] > 0){
            n2 = i;
            break;
        }
    }

    for(i=0; i<=n1-1; i++){
        dim1_memory[i] = dim1_node[i] * (ceil_log2(dim1_node[i+1])+ceil_log2(bucket[0])+32+2);
    }

    dim1_memory[i] = dim1_node[i] * (ceil_log2(bucket[0])+32+2);

    for(i=0; i<=n2-1; i++){
        dim2_memory[i] = dim2_node[i] * (ceil_log2(dim2_node[i+1])+ceil_log2(bucket[1])+32+2);
    }

    dim2_memory[i] = dim2_node[i] * (ceil_log2(bucket[1])+32+2);

    if (g < 3) printf("group %c(%d-bit)\n", g + 'B', setting[0].bit1);
    else printf("group %c'(%d-bit)\n", g - 3 + 'B', setting[0].bit2);

    int sum = 0;
    printf("dim1\n");
    for(i=0; i<=n1; i++){
        sum+=dim1_memory[i];
        printf("%d %d\n", dim1_node[i], dim1_memory[i]);
    }

    printf("%d\n", sum);
    printf("\n===============\n");
    sum = 0;
    printf("dim2\n");
    for(i=0; i<=n2; i++){
        sum+=dim2_memory[i];
        printf("%d %d\n", dim2_node[i], dim2_memory[i]);
    }
    printf("%d\n", sum);
    printf("\n===============\n");

    int sum = 0;
    printf("dim1\n");
    for(i=0; i<20; i++){
        sum+=dim1_node[i];
        printf("%d\n", dim1_node[i]);
    }
    printf("%d\n", sum);
    printf("\n==================\n");

    sum = 0;
    printf("dim2\n");
    for(i=0; i<20; i++){
        sum+=dim2_node[i];
        printf("%d\n", dim2_node[i]);

    }
    printf("%d\n", sum);
    printf("\n==================\n");
}


void result1(char s) {
    printf("group %c\n", s);
    int sel = 1;

    int i, j, k, l, m, n, na;
    int N;
    int total = 0;
    int l1_eleitv = 0;
    int l2_eleitv = 0;
    int ruleID;
    if (sel) {
        for (i = 0; i < numcombine; i++) {
            if (table3[i].group != (s - 'B')) continue;
            total++;
        }
        printf("total rules: %d\n", total);

        for (i = 0; i < 3; i++) {
            if (i != (s - 'B')) continue;
            for (na = 0; na < 65536; na++) {
                N = gp[i][na].n;

                if (N > 1)
                    l1_eleitv += N - 1;

                for (j = 1; j < N; j++)
                    l2_eleitv += gp[i][na].lv2[j].n - 1;

            }
        }
        printf("# of level 1 elementary intervals: %d\n", l1_eleitv);
        printf("# of level 2 elementary intervals: %d\n", l2_eleitv);
        total = 0;
        for (i = 0; i < mrg_num; i++) {
            ruleID = merge_bucket[i]->rule[0];
            if (table3[ruleID].group != (s - 'B')) continue;

            total++;
        }
        printf("# of distinct merge_bucket: %d\n", total);

        printf("bucket size: %d\n", thres2[s - 'B']);
    }
    else {
        for (i = 0; i < num_entry; i++) {
            if (table[i].group != (s - 'B')) continue;
            total++;
        }
        printf("total rules: %d\n", total);

        for (i = 0; i < 3; i++) {
            if (i != (s - 'B')) continue;
            for (na = 0; na < 65536; na++) {
                N = gp[i][na].n;

                l1_eleitv += (N - 1);

                for (j = 1; j < N; j++)
                    l2_eleitv += (gp[i][na].lv2[j].n - 1);

            }
        }
        printf("# of level 1 elementary intervals: %d\n", l1_eleitv);
        printf("# of level 2 elementary intervals: %d\n", l2_eleitv);
        total = 0;
        for (i = 0; i < mrg_num; i++) {
            ruleID = merge_bucket[i]->rule[0];
            if (table[ruleID].group != (s - 'B')) continue;

            total++;
        }
        printf("# of distinct merge_bucket: %d\n", total);

        printf("bucket size: %d\n", thres2[s - 'B']);
    }
    printf("===================\n\n");
}

void result2(int g) {
    int sum = 0;
    int total = 0;
    int total2 = 0;
    int total3 = 0;
    int total4 = 0;
    int total5 = 0;

    int i, j, k, na, ruleID;
    int N;

    for (i = 0; i < 6; i++) {
        if ( i != g) continue;
        for (na = 0; na < 65536; na++) {
            N = gp[i][na].n;

            for (j = 1; j < N; j++) {
                if (gp[i][na].lv2[j].type == 1 && gp[i][na].lv2[j].n > 1) total3++;
                if (gp[i][na].lv2[j].n > 1) {
                    total++;
                    total2 += gp[i][na].lv2[j].n - 1;
                }

                //printf("%d\n", gp[i][na].lv2[j].n);
            }
        }
    }
    for (i = 0; i < uni_num; i++) {
        ruleID = uni_bucket[i]->rule[0];
        if ( table[ruleID].group != g) continue;
        total4++;
    }
    for (i = 0; i < mrg_num[g]; i++) {
        ruleID = merge_bucket[g][i].rule[0];

        total5++;
    }
    for (i = 0; i < num_entry; i++) {
        if (table[i].group != g) continue;
        sum ++;
    }

    if (g < 3) printf("group %c(%d-bit)\n", g + 'B', setting[0].bit1);
    else printf("group %c'(%d-bit)\n", g - 3 + 'B', setting[0].bit2);

    printf("%d\n", sum);
    printf("%d\n", total);
    printf("%d\n", total3);
    printf("%d\n", total2);
    printf("%d\n", total4);
    printf("%d\n", total5);
    printf("%d\n", thres2[g]);

    printf("\n\n");
    //printf("%f %d\n", sum, total);
}
void result3(char g) {
    int count1[100] = {0};
    int count2[100] = {0};

    int i, j, N;

    for(i=0; i<65536; i++){
        N = gp[g][i].n;
        if( N > 1) {
            count1[N]++;
            printf("%d\n" , N);
        }
        for(j=1; j<N; j++) {
            count2[gp[g][i].lv2[j].n]++;
        }
    }

    int n1, n2;

    for(i=99; i>=0; i--) {
        if(count1[i] != 0) {
            n1 = i;
            break;
        }
    }

    for(i=99; i>=0; i--) {
        if(count2[i] != 0) {
            n2 = i;
            break;
        }
    }

    printf("dim 1 root\n");
    for(i=0; i<=n1; i++)
        printf("%d\n", count1[i]);

    printf("dim2 2 root\n");
    for(i=0; i<=n2; i++)
        printf("%d\n", count2[i]);
    return;
}*/