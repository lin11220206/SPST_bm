#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "header.h"
#include "structure.h"
#include "function.h"
#include "result.h"
#include "global.h"

int *l1_count;
int *l2_count;
int *merge_count;

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

    if(s < 3) printf("group %c(%d-bit)\n", s+'B', setting.bit1);
    else printf("group %c'(%d-bit)\n", s-3+'B', setting.bit2);

    printf("%d\n", total);
    for (i = 11; i < 20; i++)
        l1_dis[10] += l1_dis[i];

    for (i = 0; i < 11; i++)
        printf("%d\n", l1_dis[i]);

    if(total == 0)
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

        sum+= l2_count[i];

        if (l2_count[i] > max) max = l2_count[i];
    }

    if(s < 3) printf("group %c(%d-bit)\n", s+'B', setting.bit1);
    else printf("group %c'(%d-bit)\n", s-3+'B', setting.bit2);

    printf("%d\n", total);
    for(i=11; i<20; i++)
        l2_dis[10] += l2_dis[i];

    for (i = 0; i < 11; i++)
        printf("%d\n", l2_dis[i]);

    if(total == 0)
        avg = 0;
    else 
        avg = sum / total;
    printf("%.2f\n", avg);
    printf("%d\n", max);

    printf("\n\n");
}
void merge_duplication(int s) {
    int i, j, ruleID;

    merge_count = malloc(num_entry * sizeof(int));

    for (i = 0; i < num_entry; i++)
        merge_count[i] = 0;

    for (i = 0; i < mrg_num; i++) {
        ruleID = merge_bucket[i]->rule[0];
        if(table[ruleID].group != s) continue;

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

        sum+=merge_count[i];

        if (merge_count[i] > max) max = merge_count[i];
    }

    if(s < 3) printf("group %c(%d-bit)\n", s+'B', setting.bit1);
    else printf("group %c'(%d-bit)\n", s-3+'B', setting.bit2);

    printf("%d\n", total);
    for(i=11; i<20; i++)
        merge_dis[10]+=merge_dis[i];

    for (i = 0; i < 11; i++)
        printf("%d\n", merge_dis[i]);

    if(total == 0)
        avg = 0;
    else 
        avg = sum / total;
    printf("%.2f\n", avg);
    printf("%d\n", max);

    printf("\n\n");
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
                if (gp[i][na].lv2[j].type == 1) total3++;
                if (gp[i][na].lv2[j].n > 1) {
                    total++;
                    total2 += gp[i][na].lv2[j].n - 1;
                }

                //printf("%d\n", gp[i][na].lv2[j].n);
            }
        }
    }
    for(i=0; i<uni_num; i++){
        ruleID = uni_bucket[i]->rule[0];
        if( table[ruleID].group != g) continue;
        total4++;
    }
    for (i = 0; i < mrg_num; i++) {
        ruleID = merge_bucket[i]->rule[0];
        if (table[ruleID].group != g) continue;

        total5++;
    }
    for(i=0; i<num_entry; i++){
        if(table[i].group != g) continue;
        sum ++;
    }

    if(g < 3) printf("group %c(%d-bit)\n", g+'B', setting.bit1);
    else printf("group %c'(%d-bit)\n", g-3+'B', setting.bit2);

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
void result3(){
    int i;
    int total = 0;
    int total2 = 0;
    for(i=0; i<num_entry; i++){
        if(table[i].srclen < 16) total++;
        if(table[i].dstlen < 16) total2++;
    }

    printf("%d\n", total);
    printf("%d\n", total2);
    return;
}