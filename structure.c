#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "header.h"
#include "structure.h"
#include "function.h"
#include "clock.h"
#include "global.h"

struct level1 gp[6][65536];
struct bucket gp_A[4][4];
struct level1 gp_AA;
int thres[6], thres2[6], numcombine;
int thres_A;
int group[6][65536], groupp[6];
int seg_bit[6];
struct bucket *uni_dim1_bucket[500000];
struct bucket *uni_dim2_bucket[500000];
int uni_num[2] = {0};
struct bucket merge_bucket[6][100000];
int mrg_num[6] = {0};

struct ENTRY *table3;

void analysis() {
    struct ENTRY *table_1;
    struct ENTRY *table_2;
    struct ENTRY *table_3;
    struct ENTRY *table_4;
    struct ENTRY *table_5;


    int i, j, k;
    int N[5] = {0};
    int NN[5] = {0};
    int total = 0;

    table_1 = (struct ENTRY *) calloc(100000, sizeof(struct ENTRY));

    for(i=0; i<num_entry; i++) {
        if(table[i].group != -1) continue;

        total++;

        if(table[i].srcIP == 0 && table[i].srclen == 0)
            NN[0]++;

        if(table[i].dstIP == 0 && table[i].dstlen == 0)
            NN[1]++;

        if(table[i].srcPort[0] == 0 && table[i].srcPort[1] == 65535)
            NN[2]++;

        if(table[i].dstPort[0] == 0 && table[i].dstPort[1] == 65535)
            NN[3]++;

        if(table[i].protolen == 0)
            NN[4]++;

        for(j=0; j<N[0]; j++) {
            if(table[i].srcIP == table_1[j].srcIP && table[i].srclen == table_1[j].srclen) {
                break;
            }
        }
        if(j == N[0]) {
            table_1[j].srcIP = table[i].srcIP;
            table_1[j].srclen = table[i].srclen;

            N[0]++;
        }
        for(j=0; j<N[1]; j++) {
            if(table[i].dstIP == table_1[j].dstIP && table[i].dstlen == table_1[j].dstlen) {
                break;
            }
        }
        if(j == N[1]) {
            table_1[j].dstIP = table[i].dstIP;
            table_1[j].dstlen = table[i].dstlen;

            N[1]++;
        }
        
        for(j=0; j<N[2]; j++) {
            if(table[i].srcPort[0] == table_1[j].srcPort[0] && table[i].srcPort[1] == table_1[j].srcPort[1]) {
                break;
            }
        }
        if(j == N[2]) {
            table_1[j].srcPort[0] = table[i].srcPort[0];
            table_1[j].srcPort[1] = table[i].srcPort[1];

            N[2]++;
        }

        for(j=0; j<N[3]; j++) {
            if(table[i].dstPort[0] == table_1[j].dstPort[0] && table[i].dstPort[1] == table_1[j].dstPort[1]) {
                break;
            }
        }
        if(j == N[3]) {
            table_1[j].dstPort[0] = table[i].dstPort[0];
            table_1[j].dstPort[1] = table[i].dstPort[1];

            N[3]++;
        }

        for(j=0; j<N[4]; j++) {
            if(table[i].proto == table_1[j].proto && table[i].protolen == table_1[j].protolen) {
                break;
            }
        }
        if(j == N[4]) {
            table_1[j].proto = table[i].proto;
            table_1[j].protolen = table[i].protolen;
            N[4]++;
        }
    }

    printf("total, %d\n", total);
    printf("srcPort, dstPort\n");
    for(i=0; i<5; i++) {
        printf("%d, ", N[i]);
    }
    printf("\n");
    for(i=0; i<5; i++) {
        printf("%d, ", NN[i]);
    }
    printf("\n");
    //printf("%d, %d\n", NN1, NN2);

    return;
}

void pre_rebuild() {
    int i;
    for (i = 0; i < num_entry; i++) {
        if (table[i].dstlen < 16) {
            table[i].dstIP = table[i].dstIP ^ table[i].srcIP;
            table[i].srcIP = table[i].dstIP ^ table[i].srcIP;
            table[i].dstIP = table[i].dstIP ^ table[i].srcIP;

            table[i].dstlen = table[i].dstlen ^ table[i].srclen;
            table[i].srclen = table[i].dstlen ^ table[i].srclen;
            table[i].dstlen = table[i].dstlen ^ table[i].srclen;

            table[i].group = 0;
        }
    }
    return;
}

void rebuild() {
    int i, na, j, k, r;

    //printf("start rebuild ...\n");
    int *count = calloc(num_entry , sizeof(int));

    for (i = 0; i < num_entry; i++)
        count[i] = 0;

    int ruleID, N;

    for (i = 0; i < 6; i++) {
        for (na = 0; na < 65536; na++) {
            N = gp[i][na].n;
            for (j = 1; j < N; j++) {
                if (gp[i][na].lv2[j].type != 1) continue;

                for (k = 0; k < gp[i][na].lv2[j].b0->r; k++) {
                    count[gp[i][na].lv2[j].b0->rule[k]]++;
                }
            }
        }
    }

    int sum;
    for (i = 0; i < 3; i++) {
        if (!setting[i].rebuild) continue;
        for (na = 0; na < 65536; na++) {
            N = gp[i][na].n;
            for (j = 1; j < N; j++) {
                if (gp[i][na].lv2[j].type != 1) continue;

                sum = 0;
                for (k = 0; k < gp[i][na].lv2[j].b0->r; k++) {
                    if (table[gp[i][na].lv2[j].b0->rule[k]].group > 2) sum++;
                }

                while ( gp[i][na].lv2[j].b0->r - sum > 1) {
                    sum++;

                    int MaxID;
                    int MaxValue = 0;

                    for (k = 0; k < gp[i][na].lv2[j].b0->r; k++) {
                        ruleID = gp[i][na].lv2[j].b0->rule[k];
                        if (table[ruleID].group > 2) continue;

                        if (count[ruleID] > MaxValue) {
                            MaxID = ruleID;
                            MaxValue = count[ruleID];
                        }
                    }
                    table[MaxID].group += 3;
                }

            }
        }
    }

    //reset all extra memory
    /*
    for (i = 0; i < 8; i++) {
        for (na = 0; na < 65536; na++) {
            N = gp[i][na].n;
            for (j = 1; j < N; j++) {
                for (k = 1; k < gp[i][na].lv2[j].n; k++) {
                    free(gp[i][na].lv2[j].b[k]->rule);
                    free(gp[i][na].lv2[j].b[k]->rule2);
                }
            }
        }
    }
    for (i = 0; i < 8; i++) {
        for (na = 0; na < 65536; na++) {
            N = gp[i][na].n;
            for (j = 0; j < N; j++) {
                if (gp[i][na].n1[j] == 0) continue;

                for (k = 0; k < (gp[i][na].n1[j] * 2 + 1); k++) {
                    free(gp[i][na].lv2[j].b[k]);
                }

                free(gp[i][na].lv2[j].endpoint);
                free(gp[i][na].lv2[j].n2);
                free(gp[i][na].lv2[j].b);
                free(gp[i][na].lv2[j].b_type);
                free(gp[i][na].lv2[j].rule);


            }
        }
    }
    for (i = 0; i < 8; i++) {
        for (na = 0; na < 65536; na++) {
            free(gp[i][na].endpoint);
            free(gp[i][na].lv2);
            free(gp[i][na].n1);
            free(gp[i][na].rule);

            group[i][na] = 0;
        }
        groupp[i] = 0;
    }*/

    for (i = 0; i < 6; i++)
        for (na = 0; na < 65536; na++)
            group[i][na] = 0;

    memset(thres2, 0, sizeof(int) * 6);
    for (i = 0; i < 500000; i++) {
        uni_dim1_bucket[i] = NULL;
        uni_dim2_bucket[i] = NULL;
    }
    uni_num[0] = 0;
    uni_num[1] = 0;
}

void groupping() {
    int i, j, segment, g;
    unsigned int ip, len;

    //printf("start groupping ...\n");

    for (i = 0; i < 3; i++) {
        seg_bit[i] = setting[i].bit1;
        seg_bit[i + 3] = setting[i].bit2;
    }

    for (i = 0; i < 6; i++) {
        groupp[i] = 0;
        for (j = 0; j < 65536; j++)
            group[i][j] = 0;
    }

    for (i = 0; i < num_entry; i++) {

        ip = table[i].srcIP;
        len = table[i].srclen;
        g = table[i].group;

        if (setting[g < 3 ? g : g - 3].ignore) continue;

        if (g == -1) continue;

        if (seg_bit[g] == 0) {
            group[g][0]++;
            groupp[g]++;
        }
        else if (len >= seg_bit[g]) {
            segment = ip >> 32 - seg_bit[g];
            group[g][segment]++;
            groupp[g]++;
        }
        else {
            segment = ip >> 32 - seg_bit[g];
            for (j = 0; j < (1 << seg_bit[g] - len); j++) {
                group[g][segment + j]++;
            }
            groupp[g]++;
        }

    }
}

void group_A() {
    int count[4][4] = {0};
    int i, j, k;
    unsigned int ip1, ip2;
    unsigned char len1, len2;
    int l1, l2;
    int r1, r2;

    thres_A = 0;
    for (i = 0; i < num_entry; i++) {
        if (table[i].group != -1) continue;

        ip1 = table[i].srcIP;
        ip2 = table[i].dstIP;
        len1 = table[i].srclen;
        len2 = table[i].dstlen;

        l1 = ip1 >> 30;
        l2 = ip2 >> 30;
        r1 = l1 + (1 << 2 - len1) - 1;
        r2 = l2 + (1 << 2 - len2) - 1;

        for (j = l1; j <= r1; j++)
            for (k = l2; k <= r2; k++) {
                count[j][k]++;

                if (count[j][k] > thres_A) thres_A = count[j][k];
            }
    }

    for (i = 0; i < 4; i++) {
        for (j = 0; j < 4; j++) {
            gp_A[i][j].rule = calloc(thres_A, sizeof(int));
        }
    }

    for (i = 0; i < num_entry; i++) {
        if (table[i].group != -1) continue;

        ip1 = table[i].srcIP;
        ip2 = table[i].dstIP;
        len1 = table[i].srclen;
        len2 = table[i].dstlen;

        l1 = ip1 >> 30;
        l2 = ip2 >> 30;
        r1 = l1 + (1 << 2 - len1) - 1;
        r2 = l2 + (1 << 2 - len2) - 1;

        for (j = l1; j <= r1; j++)
            for (k = l2; k <= r2; k++) {
                gp_A[j][k].rule[gp_A[j][k].r++] = i;
            }
    }
}

void group_AA() {
    int i, j, k, l, r, ruleID;
    int t1 = 0, t2 = 0;
    int total = 0;

    for(i=0; i<num_entry;i++) {
        if(table[i].group != -1) continue;

        if(1) continue;

        printf("%d, ", i);
        printf("%X/%d, ",table[i].srcIP, table[i].srclen);
        printf("%X/%d, ",table[i].dstIP, table[i].dstlen);
        printf("%d-%d, ",table[i].srcPort[0], table[i].srcPort[1]);
        printf("%d-%d, ",table[i].dstPort[0], table[i].dstPort[1]);
        printf("%d\n", table[i].proto);

    }

    for (i = 0; i < num_entry; i++) {
        if (table[i].group == -1) total++;
    }

    gp_AA.endpoint = (unsigned int *) calloc(total * 2 + 1, sizeof(unsigned int));
    gp_AA.lv2 = (struct level2 *) calloc(total * 2 + 1, sizeof(struct level2));
    gp_AA.n1 = (int *) calloc(total * 2 + 1, sizeof(int));
    gp_AA.n = 1;

    for (i = 0; i < num_entry; i++) {
        if (table[i].group != -1) continue;

        l = table[i].srcPort[0];
        r = table[i].srcPort[1];

        l = (l == 0) ? 0 : l - 1;

        add_endpoint(gp_AA.endpoint, &gp_AA.n, l, r);
    }

    for (i = 0; i < num_entry; i++) {
        if (table[i].group != -1) continue;

        l = table[i].srcPort[0];
        r = table[i].srcPort[1];

        for (j = 1; j < gp_AA.n; j++) {
            if ( l <= gp_AA.endpoint[j]) break;
        }

        for (; j < gp_AA.n; j++) {
            gp_AA.n1[j]++;

            if (gp_AA.endpoint[j] >= r) break;
        }
    }

    for (i = 1; i < gp_AA.n; i++) {
        if (gp_AA.n1[i] > t1) t1 = gp_AA.n1[i];
    }

    for (i = 0; i < gp_AA.n; i++) {
        if (gp_AA.n1[i] == 0) continue;

        gp_AA.lv2[i].endpoint = (unsigned int *) calloc (gp_AA.n1[i] * 2 + 1, sizeof(unsigned int));
        gp_AA.lv2[i].n2 = (int *) calloc (gp_AA.n1[i] * 2 + 1, sizeof(int));
        gp_AA.lv2[i].b = (struct bucket **) calloc (gp_AA.n1[i] * 2 + 1, sizeof(struct bucket *));
        gp_AA.lv2[i].b_type = (unsigned int *) calloc(gp_AA.n1[i] * 2 + 1, sizeof(unsigned int));
        gp_AA.lv2[i].n = 1;
        gp_AA.lv2[i].rule = (int *) calloc(gp_AA.n1[i], sizeof(int));
        gp_AA.lv2[i].r = 0;
        gp_AA.lv2[i].b0 = (struct bucket *) calloc(1, sizeof(struct bucket));
        gp_AA.lv2[i].type = 1;

        gp_AA.lv2[i].b0->rule = calloc(gp_AA.n1[i], sizeof(int));

        for (j = 0; j < gp_AA.n1[i] * 2 + 1; j++) {
            gp_AA.lv2[i].b[j] = (struct bucket *) calloc(1, sizeof(struct bucket));
            gp_AA.lv2[i].b_type[j] = 1;
        }
    }    //Finish level1

    int l1, r1;
    int l2, r2;
    for (i = 0; i < num_entry; i++) {
        if (table[i].group != -1) continue;

        l1 = table[i].srcPort[0];
        r1 = table[i].srcPort[1];

        l2 = table[i].dstPort[0];
        r2 = table[i].dstPort[1];

        l2 = (l2 == 0) ? 0 : l2 - 1;

        for (j = 1; j < gp_AA.n; j++) {
            if ( l1 <= gp_AA.endpoint[j]) break;
        }

        for (; j < gp_AA.n; j++) {

            gp_AA.lv2[j].rule[gp_AA.lv2[j].r++] = i;
            gp_AA.lv2[j].b0->rule[gp_AA.lv2[j].b0->r++] = i;

            add_endpoint(gp_AA.lv2[j].endpoint, &gp_AA.lv2[j].n, l2, r2);

            if (gp_AA.endpoint[j] >= r1) break;
        }
    }

    for (i = 1; i < gp_AA.n; i++) {
        for (j = 0; j < gp_AA.lv2[i].r; j++) {
            ruleID = gp_AA.lv2[i].rule[j];

            l = table[ruleID].dstPort[0];
            r = table[ruleID].dstPort[1];

            for (k = 1; k < gp_AA.lv2[i].n; k++) {
                if ( l <= gp_AA.lv2[i].endpoint[k]) break;
            }

            for (; k < gp_AA.lv2[i].n; k++) {

                gp_AA.lv2[i].n2[k]++;

                if (gp_AA.lv2[i].endpoint[k] >= r) break;
            }
        }
    }

    for (i = 1; i < gp_AA.n; i++) {
        for (j = 1; j < gp_AA.lv2[i].n; j++) {
            if (gp_AA.lv2[i].n2[j] > t2) t2 = gp_AA.lv2[i].n2[j];
        }
    }

    for (i = 1; i < gp_AA.n; i++) {
        for (j = 1; j < gp_AA.lv2[i].n; j++) {
            gp_AA.lv2[i].b[j]->rule = (int *) calloc(t2, sizeof(int));
            gp_AA.lv2[i].b[j]->rule2 = (int *) calloc(t2, sizeof(int));
            gp_AA.lv2[i].b[j]->r = 0;
            gp_AA.lv2[i].b[j]->r2 = 0;
            gp_AA.lv2[i].b[j]->set = 0;
            gp_AA.lv2[i].b[j]->BV = 0;
        }
    }

    for (i = 1; i < gp_AA.n; i++) {
        for (j = 0; j < gp_AA.lv2[i].r; j++) {
            ruleID = gp_AA.lv2[i].rule[j];

            l = table[ruleID].dstPort[0];
            r = table[ruleID].dstPort[1];

            for (k = 1; k < gp_AA.lv2[i].n; k++) {
                if ( l <= gp_AA.lv2[i].endpoint[k]) break;
            }

            for (; k < gp_AA.lv2[i].n; k++) {

                gp_AA.lv2[i].b[k]->rule[gp_AA.lv2[i].b[k]->r++] = ruleID;

                if (gp_AA.lv2[i].endpoint[k] >= r) break;
            }
        }
    }    // Finish level2

    struct ENTRY *table3_A;
    int table3_n = 0;
    table3 = (struct ENTRY *) calloc(10000, sizeof(struct ENTRY));


    for (i = 0; i < num_entry; i++) {
        if (table[i].group != -1) continue;

        for (j = 0; j < table3_n; j++) {
            if (table[i].srcIP == table3[j].srcIP && table[i].srclen == table3[j].srclen \
                    && table[i].dstIP == table3[j].dstIP && table[i].dstlen == table3[j].dstlen \
                    && table[i].proto == table3[j].proto)
            {
                table[i].rule = j;
                break;
            }
        }

        if (j == table3_n) {
            table[i].rule = j;
            table3[j].srcIP = table[i].srcIP;
            table3[j].srclen = table[i].srclen;
            table3[j].dstIP = table[i].dstIP;
            table3[j].dstlen = table[i].dstlen;
            table3[j].proto = table[i].proto;

            table3_n++;
        }
    }

    t2 = 0;
    for (i = 1; i < gp_AA.n; i++) {
        for (j = 1; j < gp_AA.lv2[i].n; j++) {

            for (k = 0; k < gp_AA.lv2[i].b[j]->r; k++) {
                ruleID = gp_AA.lv2[i].b[j]->rule[k];

                for (l = 0; l < gp_AA.lv2[i].b[j]->r2; l++) {
                    if (gp_AA.lv2[i].b[j]->rule2[l] == table[ruleID].rule) break;
                }

                if ( l == gp_AA.lv2[i].b[j]->r2) {
                    gp_AA.lv2[i].b[j]->rule2[gp_AA.lv2[i].b[j]->r2++] = table[ruleID].rule;
                }

                qsort(gp_AA.lv2[i].b[j]->rule2, gp_AA.lv2[i].b[j]->r2, sizeof(int), cmp_r);

                if (gp_AA.lv2[i].b[j]->r2 > t2) t2 = gp_AA.lv2[i].b[j]->r2;
            }
        }
    } // Finish New ID mapping

    struct bucket *uni_A_bucket0[100000];
    int uni_A_num0 = 0;

    for (i = 0; i < 100000; i++) {
        uni_A_bucket0[i] = NULL;
    }

    struct bucket *now;
    for (i = 1; i < gp_AA.n; i++) {
        if (gp_AA.n1[i] == 0) continue;

        now = gp_AA.lv2[i].b0;
        if (now->r == 0) continue;

        for (j = 0; j < uni_A_num0; j++) {
            if (now->r != uni_A_bucket0[j]->r) continue;

            if (rule_check_exact(now->rule, uni_A_bucket0[j]->rule, now->r, now->r)) {
                gp_AA.lv2[i].type = 0;
                break;
            }
        }

        if (j == uni_A_num0) {
            uni_A_bucket0[uni_A_num0++] = now;
            gp_AA.lv2[i].type = 1;
        }
    }// Finish level1 bucket share


    struct bucket *uni_A_bucket[100000];
    int uni_A_num = 0;

    for (i = 0; i < 100000; i++) {
        uni_A_bucket[i] = NULL;
    }


    for (i = 1; i < gp_AA.n; i++) {
        for (j = 1; j < gp_AA.lv2[i].n; j++) {
            now = gp_AA.lv2[i].b[j];

            if (now->r == 0) continue;

            for (k = 0; k < uni_A_num; k++) {
                if (now->r != uni_A_bucket[k]->r) continue;

                if (rule_check_exact(now->rule, uni_A_bucket[k]->rule, now->r, now->r)) {
                    gp_AA.lv2[i].b_type[j] = 0;
                    break;
                }
            }
            if (k == uni_A_num) {
                uni_A_bucket[uni_A_num++] = now;
                gp_AA.lv2[i].b_type[j] = 1;
            }
        }
    } // Finish level2 bucket share

    struct bucket merge_A_bucket[20000];
    int mrg_A_num = 0;

    for (i = 0; i < 20000; i++) {
        merge_A_bucket[i].rule = (int *) calloc(t2, sizeof(int));
        merge_A_bucket[i].r = 0;
    }

    qsort(uni_A_bucket, uni_A_num, sizeof(struct bucekt *), cmp);

    for (i = 0; i < uni_A_num; i++) {
        now = uni_A_bucket[i];

        for (j = 0; j < mrg_A_num; j++) {
            r = rule_check_merge(now->rule2, merge_A_bucket[j].rule, now->r2, merge_A_bucket[j].r, t2);
            if (r) {
                merge_A_bucket[j].r = r;
                now->mergeID = j;
                break;
            }
        }

        if (j == mrg_A_num) {
            for (k = 0; k < now->r2; k++) {
                merge_A_bucket[j].rule[k] = now->rule2[k];
            }
            merge_A_bucket[j].r = now->r2;

            now->mergeID = j;
            mrg_A_num++;
        }
    }// Finist merge

    int bucket_size1[1087] = {0};
    int bucket_size2[155] = {0};
    int bucket_size3[87] = {0};
    int *dim1_count = calloc(num_entry, sizeof(int));
    int *dim2_count = calloc(num_entry, sizeof(int));
    int *merge_count = calloc(num_entry, sizeof(int));

    int **cnt = malloc(num_entry * sizeof(int *));
    for (i = 0; i < num_entry; i++) {
        cnt[i] = malloc(2048 * sizeof(int));

        for (j = 0; j < 2048; j++) {
            cnt[i][j] = -1;
        }
    }

    int max1 = 0, max2 = 0, summ = 0;;
    for (i = 1; i < gp_AA.n; i++) {
        if (gp_AA.lv2[i].type != 1) continue;

        //printf("Level 1, endpoint: %d\n", gp_AA.endpoint[i]);
        for (j = 0; j < gp_AA.lv2[i].r; j++) {
            ruleID = gp_AA.lv2[i].rule[j];
            dim1_count[ruleID]++;
            //printf("%d ", ruleID+1);
        }
        //printf("%d\n", gp_AA.lv2[i].r);

        if(gp_AA.lv2[i].b0->r > max1) max1 = gp_AA.lv2[i].b0->r;
        bucket_size1[gp_AA.lv2[i].b0->r]++;
        printf("%d\n", gp_AA.lv2[i].b0->r);
        //printf("%d\n", gp_AA.lv2[i].b0->r;);
        summ += gp_AA.lv2[i].n;
        for (j = 1; j < gp_AA.lv2[i].n; j++) {
            if (gp_AA.lv2[i].b_type[j] != 1 ) continue;

            //printf("Level 2, endpoint: %d\n", gp_AA.lv2[i].endpoint[j]);
            for (k = 0; k < gp_AA.lv2[i].b[j]->r; k++) {
                ruleID = gp_AA.lv2[i].b[j]->rule[k];
                dim2_count[ruleID]++;
                //printf("%d ", ruleID);

                for (l = 0; l < merge_count[ruleID]; l++) {
                    if (cnt[ruleID][l] == gp_AA.lv2[i].b[j]->mergeID) {
                        break;
                    }
                }

                if (l == merge_count[ruleID]) {
                    cnt[ruleID][l] = gp_AA.lv2[i].b[j]->mergeID;
                    if (merge_count[ruleID] == 2047) printf("alert!!\n");
                    merge_count[ruleID]++;
                }
            }
            //printf("%d\n", gp_AA.lv2[i].b[j]->r2);
            //printf("%d\n", gp_AA.lv2[i].b[j]->r);
            if(gp_AA.lv2[i].b[j]->r > max2) max2 = gp_AA.lv2[i].b[j]->r;
            bucket_size2[gp_AA.lv2[i].b[j]->r]++;
            bucket_size3[gp_AA.lv2[i].b[j]->r2]++;
        }
    }
    printf("New ID Table, total, %d\n", table3_n);
    printf("SrcIP, DstIP, protocol\n");
    unsigned int ips[4];
    for(i=0; i<table3_n; i++) {
        for(j=0; j<table3[i].srclen; j++) {
            printf("%u", (table3[i].srcIP & (1 << 31 - j)) > 0);
        }
        printf("*, ");

        for(j=0; j<table3[i].dstlen; j++) {
            printf("%u", (table3[i].dstIP & (1 << 31 - j)) > 0);
        }
        printf("*, ");


        printf("%u\n", table3[i].proto);
    }
    printf("\n");

    printf("bucket size distribution\n");
    for(i=75; i<155; i++) {
        printf("size %d, %d\n", i, bucket_size2[i]);
    }
    for(i=75; i<87; i++) {
        printf("size %d, %d\n", i, bucket_size3[i]);
    }

    int max[3] = {0};
    int dis[3][20] = {0};
    double sum[3] = {0};
    double avg[3] = {0};

    for(i=0; i<num_entry; i++) {
        if(table[i].group != -1) continue;

        //printf("%d %d %d\n", dim1_count[i], dim2_count[i], merge_count[i]); 

        dis[0][log_2(dim1_count[i])]++;
        dis[1][log_2(dim2_count[i])]++;
        dis[2][log_2(merge_count[i])]++;

        sum[0] += dim1_count[i];
        sum[1] += dim2_count[i];
        sum[2] += merge_count[i];

        if (dim1_count[i] > max[0]) max[0] = dim1_count[i];
        if (dim2_count[i] > max[1]) max[1] = dim2_count[i];
        if (merge_count[i] > max[2]) max[2] = merge_count[i];
    }

    for(i=0; i<3; i++) {
        if(total > 0) 
            avg[i] = sum[i] / total;
    }

    j = 0;
    for(i=19; i>=0; i--) {
        if (dis[0][i] > 0 || dis[1][i] > 0 || dis[2][i] > 0) {
            j = i;
            break;
        }
    }

    printf("Rule Duplication\n");
    printf(", total rules, %d\n", total);
    printf(", after dim1, after dim2, after merge\n");

    for(i=0; i<=j; i++) {
        printf("%d~%d", 1 << i, (1 << i + 1) - 1);
        printf(", %d, %d, %d\n", dis[0][i], dis[1][i], dis[2][i]);
    }
    printf(">=%d, 0, 0, 0\n", 1 << i);
    printf("avg., %.2f, %.2f, %.2f\n", avg[0], avg[1], avg[2]);
    printf("max, %d, %d, %d\n", max[0], max[1], max[2]);
    printf("\n\n");


    printf("total rules, %d\n", total);
    printf("# of unique dim1 buckets, %d\n", uni_A_num0);
    printf("# of unique dim2 buckets, %d\n", uni_A_num);
    printf("# of merged buckets, %d\n", mrg_A_num);
    printf("bucket size, %d\n", t2);
    printf("\n\n");

    int dim1_nodes[20] = {0};
    int dim2_nodes[20] = {0};
    int total1 = 0, total2;

    for (i = 1; i < gp_AA.n; i++) {
        if (gp_AA.lv2[i].n > 0)
            total1++;
        if (gp_AA.lv2[i].n > 1 && gp_AA.lv2[i].type == 1) {
            total2 = gp_AA.lv2[i].n - 1;
            if (gp_AA.lv2[i].endpoint[total2] == 0xFFFF && total2 > 1)
                total2--;
            layer_count(total2, dim2_nodes);
        }
    }
    if (gp_AA.endpoint[total1] == 0xFFFF && total1 > 1)
        total1--;

    layer_count(total1, dim1_nodes);


    j = 0;
    for(i=19; i>=0; i--) {
        if(dim1_nodes[i] > 0 || dim2_nodes[i] > 0) {
            j = i;
            break;
        }
    }

    printf("Node Level\n");
    total1 = 0;
    total2 = 0;
    for(i=0; i<=j; i++) {
        printf("level %d, %d, %d\n", i, dim1_nodes[i], dim2_nodes[i]);
        total1 += dim1_nodes[i];
        total2 += dim2_nodes[i];
    }
    printf("total, %d, %d\n", total1, total2);

    printf("\n\n");

    int total_memory_bit = 0;
    int bucket_ptr_bit = 0;
    int dim1_node_ptr = 0;
    int dim2_node_ptr = 0;

    bucket_ptr_bit += uni_A_num * (ceil_log2(mrg_A_num) + t2);
    total_memory_bit += bucket_ptr_bit;

    int n1, n2;
    for (i = 19; i >= 0; i--) {
        if (dim1_nodes[i] > 0) {
            n1 = i;
            break;
        }
    }

    for (i = 19; i >= 0; i--) {
        if (dim2_nodes[i] > 0) {
            n2 = i;
            break;
        }
    }

    for (i = 0; i < n1; i++) {
        dim1_node_ptr += dim1_nodes[i] * (ceil_log2(dim1_nodes[i + 1]) + ceil_log2(uni_A_num0) + 16 + 2);
    }
    dim1_node_ptr += dim1_nodes[i] * (ceil_log2(uni_A_num0) + 16 + 2);
    total_memory_bit += dim1_node_ptr;

    for (i = 0; i < n2; i++) {
        dim2_node_ptr += dim2_nodes[i] * (ceil_log2(dim2_nodes[i + 1]) + ceil_log2(uni_A_num) + 16 + 2);
    }
    dim2_node_ptr += dim2_nodes[i] * (ceil_log2(uni_A_num) + 16 + 2);
    total_memory_bit += dim2_node_ptr;

    printf("Memory Use\n");
    printf("dim1 ptr, %d\n", dim1_node_ptr);
    printf("dim2 ptr, %d\n", dim2_node_ptr);
    printf("bucket ptr, %d\n", bucket_ptr_bit);
    printf("total(bit), %d\n", total_memory_bit);
    printf("total(byte), %.0f\n", (double)total_memory_bit / 8);
    printf("avg(per rule), %.02f\n", ((double)total_memory_bit / 8) / total);

    /*
    for(i=0; i<num_entry; i++) {
        if(table[i].group != -1) continue;

        printf("%d ", i+1);
        printf("%x/%d, ", table[i].srcIP, table[i].srclen);
        printf("%x/%d, ", table[i].dstIP, table[i].dstlen);
        printf("%d-%d, ", table[i].srcPort[0], table[i].srcPort[1]);
        printf("%d-%d, ", table[i].dstPort[0], table[i].dstPort[1]);
        printf("%d\n", table[i].proto);

    }*/

    return;
}

void first_level() {
    int i, j, g, N, na, k, segment, ruleID;

    //printf("start computing first level ...\n");

    for (i = 0; i < 6; i++) {
        for (na = 0; na < 65536; na++) {
            gp[i][na].endpoint = (unsigned int *) calloc ((group[i][na] * 2 + 1) , sizeof(unsigned int));
            gp[i][na].lv2      = (struct level2 *) calloc ((group[i][na] * 2 + 1) , sizeof(struct level2));
            gp[i][na].n1       = (int *) calloc ((group[i][na] * 2 + 1) , sizeof(int));
            gp[i][na].n        = 1;
            gp[i][na].rule     = (int *) calloc(group[i][na] , sizeof(int));
            gp[i][na].r        = 0;
        }
    }

    unsigned int l, r, ip;
    int len;

    for (i = 0; i < num_entry; i++) {

        ip = table[i].srcIP;
        len = table[i].srclen;
        g = table[i].group;

        if (setting[g < 3 ? g : g - 3].ignore) continue;
        if (g == -1) continue;

        if (seg_bit[g] == 0) {
            segment = 0;
            gp[g][segment].rule[gp[g][segment].r++] = i;
        }
        else if (len >= seg_bit[g]) {
            segment = ip >> 32 - seg_bit[g];
            gp[g][segment].rule[gp[g][segment].r++] = i;
        }
        else {
            segment = ip >> 32 - seg_bit[g];
            for (j = 0; j < (1 << seg_bit[g] - len); j++) {
                gp[g][segment + j].rule[gp[g][segment + j].r++] = i;
            }
        }
    }

    for (i = 0; i < 6; i++) {
        for (na = 0; na < 65536; na++)
        {
            for (j = 0; j < gp[i][na].r; j++)
            {
                ruleID = gp[i][na].rule[j];
                ip = table[ruleID].srcIP;
                len = table[ruleID].srclen;

                if (seg_bit[i] != 0) {
                    ip = ip & (1 << 32 - seg_bit[i]) - 1;
                }

                if ( i >= 3) {
                    ip = ip >> 32 - setting[i - 3].cut;
                    len = len + setting[i - 3].cut;
                }

                l = (ip == 0)  ?  0 : ip - 1;
                r = (len == 0) ? -1 : (((ip >> (32 - len)) + 1) << (32 - len)) - 1;

                add_endpoint(gp[i][na].endpoint, &gp[i][na].n, l, r);
            }
        }
    }

    for (i = 0; i < 6; i++) {
        for (na = 0; na < 65536; na++)
        {
            for (j = 0; j < gp[i][na].r; j++)
            {
                ruleID = gp[i][na].rule[j];

                ip = table[ruleID].srcIP;
                len = table[ruleID].srclen;

                if (seg_bit[i] != 0) {
                    ip = ip & (1 << 32 - seg_bit[i]) - 1;
                }

                if ( i >= 3) {
                    ip = ip >> 32 - setting[i - 3].cut;
                    len = len + setting[i - 3].cut;
                }

                interval_operation(1, ruleID, ip, len, 0, 0, gp[i][na].n, gp[i][na].endpoint, gp[i][na].n1, NULL, 0);
            }
        }
    }

    for (i = 0; i < 6; i++) {
        for (na = 0; na < 65536; na++) {
            for (j = 1; j < gp[i][na].n; j++) {
                if (gp[i][na].n1[j] > thres[i]) thres[i] = gp[i][na].n1[j];
            }
        }
    }

    for (i = 0; i < 6; i++) {
        for (na = 0; na < 65536; na++) {
            N = gp[i][na].n;

            for (j = 0; j < N; j++) {
                if (gp[i][na].n1[j] != 0) {
                    gp[i][na].lv2[j].endpoint = (unsigned int *) calloc ((gp[i][na].n1[j] * 2 + 1) , sizeof(unsigned int));
                    gp[i][na].lv2[j].n2       = (int *) calloc ((gp[i][na].n1[j] * 2 + 1) , sizeof(int));
                    gp[i][na].lv2[j].b0      = (struct bucket *) calloc(1, sizeof(struct bucket));
                    gp[i][na].lv2[j].b        = (struct bucket **) calloc ((gp[i][na].n1[j] * 2 + 1) , sizeof(struct bucket *));
                    gp[i][na].lv2[j].b_type   = (unsigned int *) calloc ((gp[i][na].n1[j] * 2 + 1) , sizeof(unsigned int));
                    gp[i][na].lv2[j].type     = 1;
                    gp[i][na].lv2[j].n        = 1;
                    gp[i][na].lv2[j].rule     = (int *) calloc ((gp[i][na].n1[j] * 2 + 1) , sizeof(int));
                    gp[i][na].lv2[j].r        = 0;

                    gp[i][na].lv2[j].b0->rule = calloc((gp[i][na].n1[j] * 2 + 1), sizeof(int));

                    for (k = 0; k < (gp[i][na].n1[j] * 2 + 1); k++) {
                        gp[i][na].lv2[j].b[k] = (struct bucket *)calloc(1 , sizeof(struct bucket));
                        gp[i][na].lv2[j].b_type[k] = 1;
                    }
                }
            }
        }
    }
}

void second_level() {
    int i, j, k, count, g, N, T, na;

    //printf("start computing second level ...\n");

    unsigned int l, r, ip, l2, r2, ip2;
    int len, len2;
    int ruleID;
    for (i = 0; i < 6; i++) {
        for (na = 0; na < 65536; na++) {
            for (j = 0; j < gp[i][na].r; j++) {
                ruleID = gp[i][na].rule[j];

                ip = table[ruleID].srcIP;
                len = table[ruleID].srclen;

                ip2 = table[ruleID].dstIP;
                len2 = table[ruleID].dstlen;

                if (seg_bit[i] != 0) {
                    ip = ip & (1 << 32 - seg_bit[i]) - 1;
                }

                if ( i >= 3) {
                    ip = ip >> 32 - setting[i - 3].cut;
                    len = len + setting[i - 3].cut;
                }

                interval_operation(2, ruleID, ip, len, ip2, len2, gp[i][na].n, gp[i][na].endpoint, NULL, gp[i][na].lv2, 0);
            }
        }
    }

    for (i = 0; i < 6; i++) {
        for (na = 0; na < 65536; na++)
        {
            N = gp[i][na].n;

            for (j = 1; j < N; j++) {
                for (k = 0; k < gp[i][na].lv2[j].r; k++) {
                    ruleID = gp[i][na].lv2[j].rule[k];

                    ip = table[ruleID].dstIP;
                    len = table[ruleID].dstlen;

                    interval_operation(1, 0, ip, len, 0, 0, gp[i][na].lv2[j].n, gp[i][na].lv2[j].endpoint, gp[i][na].lv2[j].n2, NULL, 0);
                }
            }

            for (j = 1; j < N; j++) {
                for (k = 1; k < gp[i][na].lv2[j].n; k++) {
                    if (gp[i][na].lv2[j].n2[k] > thres2[i]) thres2[i] = gp[i][na].lv2[j].n2[k];
                }
            }
            for (j = 1; j < N; j++) {
                for (k = 1; k < gp[i][na].lv2[j].n; k++) {
                    gp[i][na].lv2[j].b[k]->rule  = (int *) calloc (thres2[i] , sizeof(int));
                    gp[i][na].lv2[j].b[k]->rule2 = (int *) calloc (thres2[i] , sizeof(int));
                    gp[i][na].lv2[j].b[k]->set   = 0;
                    gp[i][na].lv2[j].b[k]->r     = 0;
                    gp[i][na].lv2[j].b[k]->r2    = 0;
                    gp[i][na].lv2[j].b[k]->BV    = 0;
                }
            }

            for (j = 1; j < N; j++) {
                for (k = 0; k < gp[i][na].lv2[j].r; k++) {
                    ruleID = gp[i][na].lv2[j].rule[k];

                    ip = table[ruleID].dstIP;
                    len = table[ruleID].dstlen;

                    interval_operation(3, ruleID, ip, len, 0, 0, gp[i][na].lv2[j].n, gp[i][na].lv2[j].endpoint, NULL, gp[i][na].lv2, j);
                }
            }
        }
    }
}

void convert() {
    int i, j, k, l, m, n = 0, N, nn;
    int ruleID;

    //char s[] = "start converting to new rule ID ...";
    //printf("%-40s", s);

    table3 = (struct ENTRY *) calloc (10000 , sizeof(struct ENTRY)); // 3-field sub-rules table

    for (i = 0; i < num_entry; i++) {

        for (j = 0; j < n; j++) {
            if (table[i].srcPort[0] == table3[j].srcPort[0] && table[i].srcPort[1] == table3[j].srcPort[1] \
                    && table[i].dstPort[0] == table3[j].dstPort[0] && table[i].dstPort[1] == table3[j].dstPort[1] \
                    && table[i].proto == table3[j].proto && table[i].group == table3[j].group)
            {
                table[i].rule = j; //new rule ID
                break;
            }
        } //Rule ID mapping

        if (j == n) { //代表找不到存在的mapping, 新增進去
            table[i].rule = n; // new rule ID
            table3[n].n = 1; // map到這個New Rule的數量?
            table3[n].type = table[i].type; // type不知道要幹嘛
            table3[n].srcPort[0] = table[i].srcPort[0];
            table3[n].srcPort[1] = table[i].srcPort[1];
            table3[n].dstPort[0] = table[i].dstPort[0];
            table3[n].dstPort[1] = table[i].dstPort[1];
            table3[n].group = table[i].group;
            table3[n++].proto = table[i].proto;
        }
    }

    numcombine = n;
    //printf("number of combination of port & protocol: %d\n\n", n);
    nn = n;

    for (i = 0; i < 6; i++)
        thres2[i] = 0;

    int *tmp, na;
    for (m = 0; m < 6; m++) {
        for (na = 0; na < 65536; na++) {
            N = gp[m][na].n; // dim 1 的endpoints

            for (i = 1; i < N; i++) {
                for (j = 1; j < gp[m][na].lv2[i].n; j++) {
                    n = gp[m][na].lv2[i].b[j]->r2;

                    for (k = 0; k < gp[m][na].lv2[i].b[j]->r; k++) { //traverse child bucket
                        ruleID = gp[m][na].lv2[i].b[j]->rule[k];
                        for (l = 0; l < n; l++) {
                            if (gp[m][na].lv2[i].b[j]->rule2[l] == table[ruleID].rule) break;
                        }

                        if (l == n) {
                            gp[m][na].lv2[i].b[j]->rule2[n++] = table[ruleID].rule; // merge bucket
                        }
                    }
                    gp[m][na].lv2[i].b[j]->r2 = n;
                    qsort(gp[m][na].lv2[i].b[j]->rule2, n, sizeof(int), cmp_r);

                    if (n > thres2[m]) thres2[m] = n;
                }
            }
        }
    }
}
void l1_bucket_share() {
    //printf("start l1_bucket_share ...\n");
    int i, j, k, na;
    int N, ruleID;

    for (i = 0; i < 500000; i++) {
        uni_dim1_bucket[i] = NULL;
    }
    uni_num[0] = 0;

    struct bucket *now;
    for (i = 0; i < 6; i++) {
        for (na = 0; na < 65536; na++) {
            N = gp[i][na].n;

            for (j = 1; j < N; j++) {
                if (gp[i][na].n1[j] == 0) continue;
                now = gp[i][na].lv2[j].b0;
                if (now->r == 0) continue;

                for (k = 0; k < uni_num[0]; k++) {
                    if (now->r != uni_dim1_bucket[k]->r) continue;

                    if (table[now->rule[0]].group != table[uni_dim1_bucket[k]->rule[0]].group) continue;

                    if (rule_check_exact(now->rule, uni_dim1_bucket[k]->rule, now->r, now->r)) {
                        //free(gp[i][na].lv2[j].rule);
                        //gp[i][na].lv2[j].rule = uni[k];
                        gp[i][na].lv2[j].type = 0;
                        break;
                    }
                }
                if (k == uni_num[0]) {
                    uni_dim1_bucket[uni_num[0]++] = now;
                    gp[i][na].lv2[j].type = 1;
                }
            }
        }
    }

    return;
}
void l2_bucket_share() {
    //printf("start l2_bucket_share ...\n");
    int i, j, k, l, na;
    int N, ruleID;

    for (i = 0; i < 500000; i++) {
        uni_dim2_bucket[i] = NULL;
    }
    uni_num[1] = 0;

    struct bucket *now;
    for (i = 0; i < 6; i++) {
        for (na = 0; na < 65536; na++) {
            N = gp[i][na].n;

            for (j = 1; j < N; j++) {
                for (k = 0; k < gp[i][na].lv2[j].n; k++) {
                    now = gp[i][na].lv2[j].b[k];
                    if (now->r == 0) continue;

                    for (l = 0; l < uni_num[1]; l++) {
                        if (now->r != uni_dim2_bucket[l]->r ) continue;

                        if (table[now->rule[0]].group != table[uni_dim2_bucket[l]->rule[0]].group) continue;

                        if (rule_check_exact(now->rule, uni_dim2_bucket[l]->rule, now->r, now->r)) {
                            //free(gp[i][na].lv2[j].b[k]);
                            //gp[i][na].lv2[j].b[k] = uni_bucket[l];
                            gp[i][na].lv2[j].b_type[k] = 0;
                            break;
                        }
                    }
                    if (l == uni_num[1]) {
                        uni_dim2_bucket[uni_num[1]++] = now;
                        gp[i][na].lv2[j].b_type[k] = 1;
                    }
                }
            }
        }
    }
    //printf("finist bucket share\n");
    /*printf("uni_bucket num: %d\n", uni_N);
    for(i=0; i<uni_N; i++){
        for(j=0; j<uni[i]->r; j++){
            printf("%d ", uni[i]->rule[j]+1);
        }
        printf("\n");
    }*/
}
void bucket_merge() {
    //printf("start bucket_merge ...\n");

    int i, j, k, l, na, g;
    int N, ruleID;
    for (i = 0; i < 6; i++) {
        for (j = 0; j < 100000; j++) {
            merge_bucket[i][j].rule = (int *) calloc(thres2[i], sizeof(int));
            merge_bucket[i][j].r = 0;
        }
    }

    qsort(uni_dim2_bucket, uni_num[1], sizeof(struct bucket *), cmp);

    struct bucket *now;

    for (i = 0; i < uni_num[1]; i++) {
        now = uni_dim2_bucket[i];
        g = table3[now->rule2[0]].group;

        for (j = 0; j < mrg_num[g]; j++) {
            int t = thres2[g];
            int r = rule_check_merge(now->rule2, merge_bucket[g][j].rule, now->r2, merge_bucket[g][j].r, t);
            if (r) {
                merge_bucket[g][j].r = r;
                now->mergeID = j;
                break;
            }
        }
        if ( j == mrg_num[g]) { // copy now to merge_bucket[]
            for (k = 0; k < now->r2; k++) {
                merge_bucket[g][mrg_num[g]].rule[k] = now->rule2[k];
            }
            merge_bucket[g][mrg_num[g]].r = now->r2;

            now->mergeID = mrg_num[g];
            mrg_num[g]++;

        }
    }
}

int cmp(const void *a, const void *b) {
    struct bucket *aa, *bb;
    aa = *(struct bucket **)a;
    bb = *(struct bucket **)b;

    return bb->r - aa->r;
}