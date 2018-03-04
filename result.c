#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "header.h"
#include "structure.h"
#include "function.h"
#include "result.h"

int *l1_count;
int *l2_count;
int *merge_count;

void level1_duplication(char s){
    int i, na, j, k;
    l1_count = calloc(num_entry , sizeof(int));
    
    for(i=0; i<num_entry; i++)
        l1_count[i] = 0;

    int ruleID, N;
    for(i=0; i<3; i++){
        for(na=0; na<65536; na++){
            N = gp[i][na].n;

            for(j=1; j<N; j++){
                if(gp[i][na].lv2[j].type != 1) continue;
                for(k=0; k<gp[i][na].lv2[j].r; k++){
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
    for(i=0; i<num_entry; i++){
        if(table[i].group != (s-'B')) continue;

        l1_dis[log_2(l1_count[i])]++;
        total++;

        sum += l1_count[i];

        if(l1_count[i] > max) max = l1_count[i];
    }
    printf("%d\n", total);
    for(i=11; i<20; i++)
        l1_dis[10]+=l1_dis[i];
    
    for(i=0; i<11; i++)
        printf("%d\n", l1_dis[i]);

    avg = sum/total;
    printf("%.2f\n", avg);
    printf("%d\n", max);

    printf("\n==================================\n");
}
void level2_duplication(char s){
    int i, na, j, k, r;
    
    int table_num = numcombine;

    l2_count = calloc(table_num , sizeof(int));

    for(i=0; i<table_num; i++)
        l2_count[i] = 0;

    int ruleID, N;

    /*printf("thresh: %d\n", thres2[s-'B']);
    N = 0;
    for(i=0; i<uni_num; i++){
        ruleID = uni_bucket[i]->rule[0];
        if(table3[ruleID].group != s-'B') continue;
        printf("uni_bucket: %d\n", i);

        if(uni_bucket[i]->r > N) N = uni_bucket[i]->r;

        for(j=0; j<uni_bucket[i]->r; j++){
            ruleID = uni_bucket[i]->rule[j];

            if(table3[ruleID].group != s-'B') continue;

            printf("%d ", ruleID+1);
        }
        printf("\n");
    }
    printf("max: %d\n", N);*/

    for(i=0; i<3; i++){
        for(na=0; na<65536; na++){
            N = gp[i][na].n;
            for(j=1; j<N; j++){
                for(k=1; k<gp[i][na].lv2[j].n; k++){
                    if(gp[i][na].lv2[j].b_type[k] != 1) continue;
                    for(r=0; r<gp[i][na].lv2[j].b[k]->r; r++){
                        ruleID = gp[i][na].lv2[j].b[k]->rule[r];

                        l2_count[ruleID]++;
                    }
                }
            }
        }
    }

    int max = 0;
    int total = 0;
    int l2_dis[20] = {0};
    for(i=0; i<table_num; i++){
        if(table3[i].group != (s-'B')) continue;

        l2_dis[log_2(l2_count[i])]++;
        total++;

        if(l2_count[i] > max) max = l2_count[i];
    }
    printf("%d\n", total);
    for(i=0; i<17; i++)
        printf("%d\n", l2_dis[i]);

    printf("%d\n", max);

    printf("\n==================================\n");
    /*
    printf("===\n");
    for(i=0; i<num_entry; i++)
        printf("%d\n", l2_count[i]);*/
    //printf("level2_duplication\n");
    /*
    int seg1, seg2;
    unsigned int ip, len;
    for(i=0; i<num_entry; i++){
        if(table3[i].group == 4) continue;

        ip = table3[i].srcIP;
        len = table3[i].srclen;

        if(len >= 16){
            seg1 = ip >> 16;
            seg2 = seg1;
        }
        else{
            seg1 = ip >> 16;
            seg2 = seg1 + 1 << (16-len);
            seg2--;
        }

        //rintf("rule %d: group %c, seg %d - %d     ", i+1, table3[i].group+'B', seg1, seg2);
        //printf("duplicated %d times\n", l2_count[i]);
        printf("%d\n", l2_count[i]);
    }
    */
}
void merge_duplication(char s){
    int i, j, ruleID;
    int N = numcombine;
    //printf("%d\n", N);
    merge_count = malloc(N * sizeof(int));

    for(i=0; i<N; i++)
        merge_count[i] = 0;

    int sum = 0;
    for(i=0; i<mrg_num; i++){
        ruleID = merge_bucket[i]->rule[0];
        //if(table3[ruleID].group != (s-'B')) continue;
        //printf("bucket: %d\n", i);
        for(j=0; j<merge_bucket[i]->r; j++){
            //printf("%d %d\n", i, j);
            //ruleID = merge_bucket[i]->rule[j];
            if(table3[ruleID].group != (s-'B')) continue;

            //printf("%d ", ruleID+1);
            merge_count[merge_bucket[i]->rule[j]]++;
        }
        //printf("\n");
    }

    int max = 0;
    int total = 0;
    int merge_dis[20] = {0};

    for(i=0; i<20; i++) merge_dis[i] = 0;

    for(i=0; i<N; i++){
        if(table3[i].group != (s-'B')) continue;

        total++;
    }

    for(i=0; i<N; i++){
        if(table3[i].group != (s-'B')) continue;

        j = merge_count[i];
        if(log_2(j)>=0) {
            merge_dis[log_2(j)]++;
        }
        //total++;

        if(j > max) max = j;
    }
    printf("%d\n", total);
    for(i=0; i<20; i++)
        printf("%d\n", merge_dis[i]);

    printf("%d\n", max);

    printf("\n============================\n");
    //char ss[] = "\n==================================\n";
    //printf("%-40s", ss);
    /*
    for(i=0; i<mrg_num; i++){
        for(j=0; j<merge_bucket[i]->r; j++){
            printf("%d ", merge_bucket[i]->rule[j]);
        }
        printf("\n");
    }*/

    if(merge_count != 0)
        free(merge_count);
    //printf("test\n");
}

void result1(char s){
    printf("group %c\n", s);
    int sel = 1;

    int i, j, k, l, m, n, na;
    int N; 
    int total = 0;
    int l1_eleitv = 0;
    int l2_eleitv = 0;
    int ruleID;
    if(sel){
        for(i=0; i<numcombine; i++){
            if(table3[i].group != (s-'B')) continue;
            total++;
        }
        printf("total rules: %d\n", total);
        
        for(i=0; i<3; i++){
            if(i != (s-'B')) continue;
            for(na=0; na<65536; na++){
                N = gp[i][na].n;
                
                if(N > 1)
                    l1_eleitv += N - 1;

                for(j=1; j<N; j++)
                    l2_eleitv += gp[i][na].lv2[j].n - 1;

                }
        }
        printf("# of level 1 elementary intervals: %d\n", l1_eleitv);
        printf("# of level 2 elementary intervals: %d\n", l2_eleitv);
        total = 0;
        for(i=0; i<mrg_num; i++){
            ruleID = merge_bucket[i]->rule[0];
            if(table3[ruleID].group != (s-'B')) continue;

            total++;
        }
        printf("# of distinct merge_bucket: %d\n", total);

        printf("bucket size: %d\n", thres2[s-'B']);
    }
    else {
        for(i=0; i<num_entry; i++){
            if(table[i].group != (s-'B')) continue;
            total++;
        }
        printf("total rules: %d\n", total);
        
        for(i=0; i<3; i++){
            if(i != (s-'B')) continue;
            for(na=0; na<65536; na++){
                N = gp[i][na].n;
                
                l1_eleitv += (N - 1);

                for(j=1; j<N; j++)
                    l2_eleitv += (gp[i][na].lv2[j].n - 1);

                }
        }
        printf("# of level 1 elementary intervals: %d\n", l1_eleitv);
        printf("# of level 2 elementary intervals: %d\n", l2_eleitv);
        total = 0;
        for(i=0; i<mrg_num; i++){
            ruleID = merge_bucket[i]->rule[0];
            if(table[ruleID].group != (s-'B')) continue;

            total++;
        }
        printf("# of distinct merge_bucket: %d\n", total);

        printf("bucket size: %d\n", thres2[s-'B']);
    }
    printf("\n=========================================\n");
}