#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "header.h"
#include "structure.h"
#include "function.h"
#include "clock.h"
#include "global.h"

struct level1 gp[6][65536];
int thres[6], thres2[6], thres3[4], count2[4], numcombine;
int group[6][65536], groupp[6];
struct bucket *uni_bucket[500000];
int uni_num = 0;
struct bucket *merge_bucket[500000];
int mrg_num = 0;

// group: Group A~D, SrcIP 5-bit Seg後各root的rule數, groupp: 該組裡的rule數
// max[i]: Group A~D中擁有最多rules的root
//gp level 1 roots
struct ENTRY3 *table3;

void groupping() {
    int i, j, max[3] = {0}, segment;
    unsigned int ip, len;

    char s[] = "start groupping ...";
    //printf("%-40s", s);

    for (i = 0; i < num_entry; i++) { // count rule number of segmentation roots
        //if (table[i].group != setting.group) continue; //don't need because similar code in header.c

        ip = table[i].srcIP;
        len = table[i].srclen;

        if(len >= setting.bit1){ // use bit1-bit segmentation table
            if(setting.bit1 == 0)
                segment = 0;
            else 
                segment = ip >> 32 - setting.bit1;

            group[table[i].group][segment]++;
            groupp[table[i].group]++;
        }
        else{ // use bit2-bit segmentation table

            if(len < setting.bit2) printf("warning!\n");

            table[i].group += 3;
            
            if(setting.bit2 == 0)
                segment = 0;
            else 
                segment = ip >> 32 - setting.bit2;

            group[table[i].group][segment]++;
            groupp[table[i].group]++;
        }
        /*
        if (table[i].group > 2) { //don't need segmentation table
            group[table[i].group][0]++;
            groupp[table[i].group]++;
        }
        else { // need segmentation table
            if (len >= 16) {
                group[table[i].group][ip >> 16]++;
                groupp[table[i].group]++;
            }
            else { // duplication
                for (j = 0; j < (1 << 16 - len); j++) {
                    group[table[i].group][(ip >> 16) + j]++;
                    groupp[table[i].group]++;
                }
            }
        }*/
    }
}

void first_level() {
    int i, j, g, N, na, k, segment;

    char s[] = "start computing first level ...";
    //printf("%-40s", s);

    for (i = 0; i < 6; i++) {
        for (na = 0; na < 65536; na++) {
            gp[i][na].endpoint = (unsigned int *) malloc ((group[i][na] * 2 + 1) * sizeof(unsigned int)); // n條rule最多有2n+1個endpoints
            gp[i][na].lv2      = (struct level2 *) malloc ((group[i][na] * 2 + 1) * sizeof(struct level2)); //每個endpoints對應一個level 2 structure
            gp[i][na].n1       = (int *) malloc ((group[i][na] * 2 + 1) * sizeof(int)); //每個endpoints被cover的次數
            gp[i][na].n        = 1; // endpoints數
            gp[i][na].rule     = (int *) calloc(group[i][na] , sizeof(int));
            gp[i][na].r        = 0;
        }
    }

    unsigned int l, r, ip;
    int len;

    for (i = 0; i < num_entry; i++) {  //insert ruleID to there segmentation roots
        //if (table[i].group != setting.group) continue; //don't need because similar code in header.c

        ip = table[i].srcIP;
        len = table[i].srclen;

        if(len >= setting.bit1){ // use bit1-bit segmentation table
            if(setting.bit1 == 0)
                segment = 0;
            else 
                segment = ip >> 32 - setting.bit1;

            gp[table[i].group][segment].rule[gp[table[i].group][segment].r++] = i;
        }
        else{ // use bit2-bit segmentation table
            if(setting.bit2 == 0)
                segment = 0;
            else 
                segment = ip >> 32 - setting.bit2;

            gp[table[i].group][segment].rule[gp[table[i].group][segment].r++] = i;
        }
        /*
        if (table[i].group > 2) {
            gp[table[i].group][0].rule[gp[table[i].group][0].r++] = i;
        }
        else {
            if (len >= 16) {
                na = ip >> 16;
                gp[table[i].group][na].rule[gp[table[i].group][na].r++] = i;
            }
            else {
                for (j = 0; j < (1 << 16 - len); j++) {
                    na = (ip >> 16) + j;
                    gp[table[i].group][na].rule[gp[table[i].group][na].r++] = i;
                }
            }
        }*/
        //g = table[i].group;
        //gp[g][0].rule[gp[g][0].r++] = i;


    }

    int ruleID;
    for (i = 0; i < 6; i++) {
        for (na = 0; na < 65536; na++)
        {
            for (j = 0; j < gp[i][na].r; j++)
            {
                ruleID = gp[i][na].rule[j];

                ip = table[ruleID].srcIP;
                len = table[ruleID].srclen;

                l = (ip == 0)  ?  0 : ip - 1; // left interval
                r = (len == 0) ? -1 : (((ip >> (32 - len)) + 1) << (32 - len)) - 1; //right interval

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


                interval_operation(1, ruleID, ip, len, 0, 0, gp[i][na].n, gp[i][na].endpoint, gp[i][na].n1, NULL, 0);
            }
        }
    }

    for (i = 0; i < 6; i++) {
        for (na = 0; na < 65536; na++) {
            for (j = 1; j < gp[i][na].n; j++) {
                if (gp[i][na].n1[j] > thres[i]) thres[i] = gp[i][na].n1[j]; // 找出cover最多rule的interval
            }
        }

        //printf("group %c bucket size(lv1): %d\n", i + 65, thres[i]);
    }

    for (i = 0; i < 6; i++) {
        for (na = 0; na < 65536; na++) {
            N = gp[i][na].n; //child

            for (j = 0; j < N; j++) {
                if (gp[i][na].n1[j] != 0) {
                    gp[i][na].lv2[j].endpoint = (unsigned int *) malloc ((gp[i][na].n1[j] * 2 + 1) * sizeof(unsigned int));
                    //level 2 root所擁有的endpoints數, 最多為2n+1個, n為這個root所cover的rule subset
                    gp[i][na].lv2[j].n2       = (int *) malloc ((gp[i][na].n1[j] * 2 + 1) * sizeof(int));
                    gp[i][na].lv2[j].b        = (struct bucket **) malloc ((gp[i][na].n1[j] * 2 + 1) * sizeof(struct bucket *)); //child => bucket
                    gp[i][na].lv2[j].b_type   = (unsigned int *) malloc ((gp[i][na].n1[j] * 2 + 1) * sizeof(unsigned int));
                    gp[i][na].lv2[j].type     = 1;
                    gp[i][na].lv2[j].n        = 1;
                    gp[i][na].lv2[j].rule     = (int *) calloc ((gp[i][na].n1[j] * 2 + 1) , sizeof(int)); //child
                    gp[i][na].lv2[j].r        = 0; // bucket size

                    for (k = 0; k < (gp[i][na].n1[j] * 2 + 1); k++) {
                        gp[i][na].lv2[j].b[k] = (struct bucket *)malloc(sizeof(struct bucket));
                        gp[i][na].lv2[j].b_type[k] = 1;
                    }
                }
            }
        }
    }

    for (i = 0; i < 6; i++) {
        for (na = 0; na < 65536; na++) {
            for (j = 0; j < gp[i][na].r; j++) {
                ruleID = gp[i][na].rule[j];

                ip = table[ruleID].srcIP;
                len = table[ruleID].srclen;
            }
        }
    }

    //printf("finish\n");
}

void second_level() {
    int i, j, k, count, g, N, T, na;

    char s[] = "start computing second level ...";
    //printf("%-40s", s);

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
                    //一樣找出cover最多rule的interval , dim 2
                }
            }
            for (j = 1; j < N; j++) {
                for (k = 1; k < gp[i][na].lv2[j].n; k++) {
                    gp[i][na].lv2[j].b[k]->rule  = (int *) malloc (thres2[i] * sizeof(int)); //bucket
                    gp[i][na].lv2[j].b[k]->rule2 = (int *) malloc (thres2[i] * sizeof(int)); //bucket 2
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
                    //把rule放入對應的endpoints child bucket裡面
                }
            }
        }
    }
    // SPST 完成
    //printf("finish\n");
}

void convert() {
    int i, j, k, l, m, n = 0, N, nn;
    int ruleID;

    //char s[] = "start converting to new rule ID ...";
    //printf("%-40s", s);

    table3 = (struct ENTRY3 *) malloc (10000 * sizeof(struct ENTRY3)); // 3-field sub-rules table

    for (i = 0; i < num_entry; i++) {

        for (j = 0; j < n; j++) {
            if (table[i].srcPort[0] == table3[j].Port[0] && table[i].srcPort[1] == table3[j].Port[1] && table[i].dstPort[0] == table3[j].Port[2] && table[i].dstPort[1] == table3[j].Port[3] && table[i].proto == table3[j].proto && table[i].group == table3[j].group) {
                table[i].rule = j; //new rule ID
                break;
            }
        } //Rule ID mapping

        if (j == n) { //代表找不到存在的mapping, 新增進去
            table[i].rule = n; // new rule ID
            table3[n].n = 1; // map到這個New Rule的數量?
            table3[n].type = table[i].type; // type不知道要幹嘛
            table3[n].Port[0] = table[i].srcPort[0];
            table3[n].Port[1] = table[i].srcPort[1];
            table3[n].Port[2] = table[i].dstPort[0];
            table3[n].Port[3] = table[i].dstPort[1];
            table3[n].group = table[i].group;
            table3[n++].proto = table[i].proto;
        }
    }

    numcombine = n;
    //printf("number of combination of port & protocol: %d\n\n", n);
    nn = n;

    thres2[0] = 0;
    thres2[1] = 0;
    thres2[2] = 0;

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
                    //change rule, rule2, r, r2
                    //gp[m][na].lv2[i].b[j]->r2 = n;
                    gp[m][na].lv2[i].b[j]->r2 = gp[m][na].lv2[i].b[j]->r;
                    gp[m][na].lv2[i].b[j]->r = n;
                    tmp = gp[m][na].lv2[i].b[j]->rule;
                    gp[m][na].lv2[i].b[j]->rule = gp[m][na].lv2[i].b[j]->rule2;
                    gp[m][na].lv2[i].b[j]->rule2 = tmp;

                    qsort(gp[m][na].lv2[i].b[j]->rule, n, sizeof(int), cmp_r);

                    if (n > thres2[m]) thres2[m] = n;
                }
            }
        }
        //printf("group %c bucket size: %-4d\n", m + 65, thres3[m]);
    }

    //printf("finish\n");
}
void l1_bucket_share() {

    int i, j, k, na;
    int N, ruleID;

    int *uni[500000];
    int uni_l[500000] = {0};
    int uni_N = 0;

    int *now;
    int l;
    for (i = 0; i < 6; i++) {
        for (na = 0; na < 65536; na++) {
            N = gp[i][na].n;
            if(N < 2) continue;

            for (j = 1; j < N; j++) {
                if (gp[i][na].lv2[j].r == 0) continue;

                now = gp[i][na].lv2[j].rule;
                l = gp[i][na].lv2[j].r;
                for (k = 0; k < uni_N; k++) {
                    if (l != uni_l[k]) continue;

                    if (rule_check_exact(now, uni[k], l, l)) {
                        free(gp[i][na].lv2[j].rule);
                        gp[i][na].lv2[j].rule = uni[k];
                        gp[i][na].lv2[j].type = 0;
                        break;
                    }
                }
                if (k == uni_N) {
                    uni[uni_N] = now;
                    uni_l[uni_N++] = l;
                    gp[i][na].lv2[j].type = 1;
                }
            }
        }
    }

    return;
}
void l2_bucket_share() {
    //printf("start bucket_share\n");
    int i, j, k, l, na;
    int N, ruleID;

    //uni_bucket = (struct bucket **) calloc(500000 , sizeof(struct bucket *));

    struct bucket *now;
    for (i = 0; i < 6; i++) {
        for (na = 0; na < 65536; na++) {
            N = gp[i][na].n;
            if(N < 2) continue;

            for (j = 1; j < N; j++) {
                for (k = 0; k < gp[i][na].lv2[j].n; k++) {
                    now = gp[i][na].lv2[j].b[k];
                    if (now->r == 0) continue;
                    for (l = 0; l < uni_num; l++) {
                        if (now->r != uni_bucket[l]->r ) continue;
                        
                        if (rule_check_exact(now->rule, uni_bucket[l]->rule, now->r, now->r)) {
                            //free(gp[i][na].lv2[j].b[k]);
                            gp[i][na].lv2[j].b[k] = uni_bucket[l];
                            gp[i][na].lv2[j].b_type[k] = 0;
                            break;
                        }
                    }
                    if (l == uni_num) {
                        uni_bucket[uni_num++] = now;
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
    //printf("start bucket_merge\n");
    //qsort(uni_bucket, uni_num, sizeof(struct bucket *), cmp);


    int i, j, k, l, na;
    int N, ruleID;
    /*
    for(i=0; i<150; i++){
        printf("%d %p\n", i, uni_bucket[i]);
    }*/
    //sort();
    qsort(uni_bucket, uni_num, sizeof(struct bucket *), cmp);

    struct bucket *now;
    for (i = 0; i < uni_num; i++) {
        now = uni_bucket[i];

        for (j = 0; j < mrg_num; j++) {
            if (table[now->rule[0]].group != table[merge_bucket[j]->rule[0]].group) continue;
            int t = thres2[table[now->rule[0]].group];
            int r = rule_check_merge(now->rule, merge_bucket[j]->rule, now->r, merge_bucket[j]->r, t);
            if (r) {
                merge_bucket[j]->r = r;
                break;
            }
        }
        if (j == mrg_num) {
            merge_bucket[mrg_num++] = now;
        }
    }
    /*
    for(i=0; i<mrg_num; i++){
        printf("bucket %d: ", i);
        for(j=0; j<merge_bucket[i]->r; j++){
            printf("%d ", merge_bucket[i]->rule[j]+1);
        }
        printf("\n");
    }*/
    //printf("finish bucket_merge\n");
}

int cmp(const void *a, const void *b) {
    struct bucket *aa, *bb;
    aa = *(struct bucket **)a;
    bb = *(struct bucket **)b;

    return bb->r - aa->r;
}

void sort() {
    int i, j;

    struct bucket *tmp;
    for (i = 0; i < uni_num - 1; i++) {
        int max = i;
        for (j = i + 1; j < uni_num; j++) {
            if (uni_bucket[j] == 0 || uni_bucket[max] == 0)
                printf("fuck\n");
            if (uni_bucket[j]->r > uni_bucket[max]->r)
                max = j;
        }
        tmp = uni_bucket[max];
        uni_bucket[max] = uni_bucket[j];
        uni_bucket[j] = tmp;
    }
}