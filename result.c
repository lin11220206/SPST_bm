#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "header.h"
#include "structure.h"
#include "function.h"
#include "result.h"
#include "global.h"

struct result data;
struct result_A data_A;
struct memory memory_use;

void get_result() {
    //printf("start getting result ...\n");

    int i, na, j, k, r, s, ruleID, newID, N, g;
    int total, total2;

    data_A.total_rules = 0;
    data_A.buckets = 0;
    data_A.bucket_size = 0;

    for(i=0; i<num_entry; i++) {
        if(table[i].group != -1) continue;

        data_A.total_rules++;
    }
    data_A.bucket_size = thres_A;

    for(i=0; i<4; i++){
        for(j=0; j<4; j++) {
            if(gp_A[i][j].r > 0) data_A.buckets++;
        }
    }

    data.dim1_count = malloc(num_entry * sizeof(int));
    data.dim2_count = malloc(num_entry * sizeof(int));
    data.merge_count = malloc(num_entry * sizeof(int));

    for (i = 0; i < num_entry; i++) {
        data.dim1_count[i] = 0;
        data.dim2_count[i] = 0;
        data.merge_count[i] = 0;
    }

    for (i = 0; i < 6; i++) {
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
        data.dim1_rules[i] = 0;
    }

    int **cnt = malloc(num_entry * sizeof(int *));
    for (i = 0; i < num_entry; i++) {
        cnt[i] = malloc(100 * sizeof(int));
    }

    for (i = 0; i < num_entry; i++) {
        for (j = 0; j < 100; j++)
            cnt[i][j] = -1;
    }

    for (i = 0; i < 6; i++) {
        for (na = 0; na < 65536; na++) {

            if (group[i][na] > 0) data.dim1_roots[i]++;

            N = gp[i][na].n;

            for (j = 1; j < N; j++) {
                if (gp[i][na].lv2[j].type != 1) continue;
                if (gp[i][na].lv2[j].r > data.dim1_rules[i])
                    data.dim1_rules[i] = gp[i][na].lv2[j].r;
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

    unsigned int max_value;
    for (i = 0; i < 6; i++) {
        for (na = 0; na < 65536; na++) {
            N = gp[i][na].n;
            if ( N < 1) continue;
            total = 0;
            for (j = 1; j < N; j++) {
                if (gp[i][na].lv2[j].n > 0)
                    total++;
                if (gp[i][na].lv2[j].n > 1 && gp[i][na].lv2[j].type == 1) {

                    total2 = gp[i][na].lv2[j].n - 1;
                    if (gp[i][na].lv2[j].endpoint[total2] == 0xFFFFFFFF && total2 > 1)
                        total2--;
                    layer_count(total2, data.dim2_nodes[i]);
                }
            }
            if (i > 2) {
                max_value = (1 << 15 - seg_bit[i]) - 1;
            }
            else {
                if (seg_bit[i] != 0)
                    max_value = (1 << 32 - seg_bit[i]) - 1;
                else
                    max_value = 0xFFFFFFFF;
            }
            if (gp[i][na].endpoint[total] == max_value && total > 1)
                total--;

            layer_count(total, data.dim1_nodes[i]);
        }
    }

    for (i = 0; i < 6; i++) {
        for (na = 0; na < 65536; na++) {
            N = gp[i][na].n;

            for (j = 1; j < N; j++) {
                //if (gp[i][na].lv2[j].type == 1 && gp[i][na].lv2[j].n > 1) data.dim1_buckets[i][1]++;
                if (gp[i][na].lv2[j].n > 1) {
                    data.dim1_buckets[i][0]++;
                    data.dim2_buckets[i][0] += gp[i][na].lv2[j].n - 1;
                }
            }

        }
    }
    for (i = 0; i < uni_num[0]; i++) {
        ruleID = uni_dim1_bucket[i]->rule[0];
        g = table[ruleID].group;

        data.dim1_buckets[g][1]++;
    }

    for (i = 0; i < uni_num[1]; i++) {
        ruleID = uni_dim2_bucket[i]->rule[0];
        g = table[ruleID].group;

        data.dim2_buckets[g][1]++;
    }

    for (i = 0; i < 6; i++) {
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



    for (g = 0; g < 6; g++) {

        for (i = 0; i < 20; i++) {
            memory_use.dim1_node_ptr[g][i] = 0;
            memory_use.dim2_node_ptr[g][i] = 0;
        }

        memory_use.total[g][0] = 0;
        memory_use.total[g][1] = 0;
        memory_use.bucket_ptr[g] = 0;
        memory_use.seg_table[g] = 0;
    }

    memory_use.group_A_use = 0;
    memory_use.sum_bit = 0;
    memory_use.sum_byte = 0;
    memory_use.byte_per_rule = 0;

    memory_use.group_A_use = data.group_A_buckets * (ceil_log2(thres_A));

    for (g = 0; g < 6; g++) {
        if (g < 3) bit = setting[g].bit1;
        else bit = setting[g - 3].bit2;

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

    for (g = 0; g < 6; g++) {
        for (i = 0; i < 20; i++) {
            memory_use.sum_bit += memory_use.dim1_node_ptr[g][i];
            memory_use.sum_bit += memory_use.dim2_node_ptr[g][i];

            memory_use.total[g][0] += memory_use.dim1_node_ptr[g][i];
            memory_use.total[g][1] += memory_use.dim2_node_ptr[g][i];
        }
        memory_use.sum_bit += memory_use.bucket_ptr[g];
        memory_use.sum_bit += memory_use.seg_table[g];
    }
    memory_use.sum_bit += memory_use.group_A_use;

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

    if (g < 3) sprintf(gName, "%c(%d-bit)", g + 'B', setting[g].bit1);
    else sprintf(gName, "%c'(%d-bit)", g - 3 + 'B', setting[g - 3].bit2);

    printf("%s, ", gName);

    int n = 0;
    for (i = 19; i >= 0; i--) {
        if (dis[0][i] > 0 || dis[1][i] > 0 || dis[2][i] > 0) {
            n = i;
            break;
        }
    }

    printf(", total rules, %d\n", data.total_rules[g]);
    printf(", after dim1, after dim2, after merge\n");

    for (i = 0; i <= n; i++) {
        printf("%d~%d", 1 << i, (1 << i + 1) - 1);
        printf(", %d, %d, %d\n", dis[0][i], dis[1][i], dis[2][i]);
    }
    printf(">=%d", 1 << i);
    printf("avg., %.2f, %.2f, %.2f\n", avg[0], avg[1], avg[2]);
    printf("max, %d, %d, %d\n", max[0], max[1], max[2]);
    printf("\n\n");

    return;
}

void show_buckets_data() {
    int i;

    printf("Group A\n");
    printf("total rules, %d,\n", data_A.total_rules);
    printf("# of bucekts, %d\n", data_A.buckets);
    printf("bucket size, %d\n", data_A.bucket_size);
    printf("\n");

    printf("group, ");
    for (i = 0; i < 3; i++) {
        printf("%c(%d-bit), ", i + 'B', setting[i].bit1);
        printf("%c'(%d-bit), ", i + 'B', setting[i].bit2);
    }
    printf("\n");

    printf("total rules, ");
    for (i = 0; i < 3; i++) {
        printf("%d, ", data.total_rules[i]);
        printf("%d, ", data.total_rules[i + 3]);
    }
    printf("\n");

    printf("dim1 EIs, ");
    for (i = 0; i < 3; i++) {
        printf("%d, ", data.dim1_buckets[i][0]);
        printf("%d, ", data.dim1_buckets[i + 3][0]);
    }
    printf("\n");

    printf("dim1 EIs(shared), ");
    for (i = 0; i < 3; i++) {
        printf("%d, ", data.dim1_buckets[i][1]);
        printf("%d, ", data.dim1_buckets[i + 3][1]);
    }
    printf("\n");

    printf("Max # of rules in each dim2 tree, ");
    for (i = 0; i < 3; i++) {
        printf("%d, ", data.dim1_rules[i]);
        printf("%d, ", data.dim1_rules[i + 3]);
    }
    printf("\n");

    printf("dim2 EIs, ");
    for (i = 0; i < 3; i++) {
        printf("%d, ", data.dim2_buckets[i][0]);
        printf("%d, ", data.dim2_buckets[i + 3][0]);
    }
    printf("\n");

    printf("dim2 EIs(shared), ");
    for (i = 0; i < 3; i++) {
        printf("%d, ", data.dim2_buckets[i][1]);
        printf("%d, ", data.dim2_buckets[i + 3][1]);
    }
    printf("\n");

    printf("merge buckets, ");
    for (i = 0; i < 3; i++) {
        printf("%d, ", data.merge_buckets[i]);
        printf("%d, ", data.merge_buckets[i + 3]);
    }
    printf("\n");

    printf("buckets size, ");
    for (i = 0; i < 3; i++) {
        printf("%d, ", data.bucket_size[i]);
        printf("%d, ", data.bucket_size[i + 3]);
    }
    printf("\n");

    printf("\n\n");
    return;
}

void show_nodes_count(char g) {
    int i, n = 0;
    char gName[10];
    int total[2] = {0};

    if (g < 3) sprintf(gName, "%c(%d-bit)", g + 'B', setting[g].bit1);
    else sprintf(gName, "%c'(%d-bit)", g - 3 + 'B', setting[g - 3].bit2);

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
    printf("\n\n");
    return;
}

void show_memory_use() {

    int i;

    printf("group, ");
    for (i = 0; i < 3; i++) {
        printf("%c(%d-bit), ", i + 'B', setting[i].bit1);
        printf("%c'(%d-bit), ", i + 'B', setting[i].bit2);
    }
    printf("\n");

    printf("segmentation table, ");
    for (i = 0; i < 3; i++) {
        printf("%d, ", memory_use.seg_table[i]);
        printf("%d, ", memory_use.seg_table[i + 3]);
    }
    printf("\n");

    printf("dim1 ptr, ");
    for (i = 0; i < 3; i++) {
        printf("%d, ", memory_use.total[i][0]);
        printf("%d, ", memory_use.total[i + 3][0]);
    }
    printf("\n");

    printf("dim2 ptr, ");
    for (i = 0; i < 3; i++) {
        printf("%d, ", memory_use.total[i][1]);
        printf("%d, ", memory_use.total[i + 3][1]);
    }
    printf("\n");

    printf("buckets ptr, ");
    for (i = 0; i < 3; i++) {
        printf("%d, ", memory_use.bucket_ptr[i]);
        printf("%d, ", memory_use.bucket_ptr[i + 3]);
    }
    printf("\n");

    printf("group A use, %d\n", memory_use.group_A_use);
    printf("total(bit), %d\n", memory_use.sum_bit);
    printf("total(byte), %.0f\n", memory_use.sum_byte);
    printf("avg(per rule), %.02f\n", memory_use.byte_per_rule);

    printf("\n\n");
    return;
}

void show_prefix_length(char g) {

    int i;
    int srclen[10] = {0};
    int dstlen[10] = {0};


    for (i = 0; i < num_entry; i++) {
        if (table[i].group != g) continue;

        srclen[table[i].srclen]++;
        dstlen[table[i].dstlen]++;
    }

    for (i = 0; i < 10; i++) {
        printf("%d\t%d\t%d\n", i, srclen[i], dstlen[i]);
    }
}