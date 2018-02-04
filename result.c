#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "header.h"
#include "structure.h"
#include "function.h"
#include "result.h"

int *l1_count;
int *l2_count;

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
                for(k=0; k<gp[i][na].lv2[j].r; k++){
                    ruleID = gp[i][na].lv2[j].rule[k];

                    l1_count[ruleID]++;
                }
            }
        }
    }


    int total = 0;
    int l1_dis[20] = {0};
    for(i=0; i<num_entry; i++){
        if(table[i].group != (s-'B')) continue;

        l1_dis[log_2(l1_count[i])]++;
        total++;
    }
    printf("%d\n", total);
    for(i=0; i<20; i++)
        printf("%d\n", l1_dis[i]);
    //printf("l1 level1_duplication\n");
    /*
    int seg1, seg2;
    unsigned int ip, len;
    for(i=0; i<num_entry; i++){
        if(table[i].group == 4) continue;

        ip = table[i].srcIP;
        len = table[i].srclen;

        if(len >= 16){
            seg1 = ip >> 16;
            seg2 = seg1;
        }
        else{
            seg1 = ip >> 16;
            seg2 = seg1 + 1 << (16-len);
            seg2--;
        }

        //printf("rule %d: group %c, seg %d - %d     ", i+1, table[i].group+'B', seg1, seg2);
        //printf("duplicated %d times\n", l1_count[i]);
        printf("%d\n", l1_count[i]);
    }*/

}
void level2_duplication(char s){
    int i, na, j, k, r;
    l2_count = calloc(num_entry , sizeof(int));
    
    for(i=0; i<num_entry; i++)
        l2_count[i] = 0;

    int ruleID, N;
    for(i=0; i<3; i++){
        for(na=0; na<65536; na++){
            N = gp[i][na].n;

            for(j=1; j<N; j++){
                for(k=1; k<gp[i][na].lv2[j].n; k++){
                    for(r=0; r<gp[i][na].lv2[j].b[k].r; r++){
                        ruleID = gp[i][na].lv2[j].b[k].rule[r];

                        l2_count[ruleID]++;
                    }
                }
            }
        }
    }

    int total = 0;
    int l2_dis[20] = {0};
    for(i=0; i<num_entry; i++){
        if(table[i].group != (s-'B')) continue;

        l2_dis[log_2(l2_count[i])]++;
        total++;
    }
    printf("%d\n", total);
    for(i=0; i<20; i++)
        printf("%d\n", l2_dis[i]);
    //printf("level2_duplication\n");
    /*
    int seg1, seg2;
    unsigned int ip, len;
    for(i=0; i<num_entry; i++){
        if(table[i].group == 4) continue;

        ip = table[i].srcIP;
        len = table[i].srclen;

        if(len >= 16){
            seg1 = ip >> 16;
            seg2 = seg1;
        }
        else{
            seg1 = ip >> 16;
            seg2 = seg1 + 1 << (16-len);
            seg2--;
        }

        //rintf("rule %d: group %c, seg %d - %d     ", i+1, table[i].group+'B', seg1, seg2);
        //printf("duplicated %d times\n", l2_count[i]);
        printf("%d\n", l2_count[i]);
    }
    */
}