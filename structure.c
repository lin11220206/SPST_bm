#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "header.h"
#include "structure.h"
#include "function.h"

struct level1 gp[3][65536];
int thres[3], thres2[3], thres3[4], count2[4], numcombine;
int group[3][65536], pnp[4], rnr[4], groupp[3], num_bucket[4], uni_bucket[4];
// group: Group A~D, SrcIP 5-bit Seg後各root的rule數, groupp: 該組裡的rule數
// max[i]: Group A~D中擁有最多rules的root
//gp level 1 roots
struct ENTRY3 *table3;

void groupping() {
    int i, j, max[3] = {0};
    unsigned int ip, len;

    char s[] = "start groupping ...";
    //printf("%-40s", s);

    for (i = 0; i < num_entry; i++) { // count rule number of segmentation roots
        if (table[i].group == 4) continue; //group A

        ip = table[i].srcIP;
        len = table[i].srclen;

        if (len >= 16) {
            group[table[i].group][ip >> 16]++;
            groupp[table[i].group]++;
        }
        else {
            for (j = 0; j < (1 << 16 - len); j++) {
                group[table[i].group][(ip >> 16) + j]++;
                groupp[table[i].group]++;
            }
        }
    }

    //printf("finish\n");
}

void first_level() {
    int i, j, g, N, na, k;

    char s[] = "start computing first level ...";
    //printf("%-40s", s);

    for (i = 0; i < 3; i++) {
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
        if (table[i].group == 4) continue; //group A

        ip = table[i].srcIP;
        len = table[i].srclen;


        if (len >= 16) {
            na = ip >> 16;
            gp[table[i].group][na].rule[gp[table[i].group][na].r++] = i;
        }
        else {
            for (j = 0; j < (1 << 16 - len); j++) {
                na = (ip>>16) + j;
                gp[table[i].group][na].rule[gp[table[i].group][na].r++] = i;
            }
        }
    }
    
    int ruleID;
    for(i=0; i<3; i++){
        for(na=0; na<65536; na++)
        {
            for(j=0; j<gp[i][na].r; j++)
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

    for(i=0; i<3; i++){
        for(na=0; na<65536; na++)
        {
            for(j=0; j<gp[i][na].r; j++)
            {
                ruleID = gp[i][na].rule[j];

                ip = table[ruleID].srcIP;
                len = table[ruleID].srclen;


                interval_operation(1, ruleID, ip, len, 0, 0, gp[i][na].n, gp[i][na].endpoint, gp[i][na].n1, NULL, 0);
            }
        }
    }
    
    for (i = 0; i < 3; i++) {
        for (na = 0; na < 65536; na++) {
            for (j = 1; j < gp[i][na].n; j++) {
                if (gp[i][na].n1[j] > thres[i]) thres[i] = gp[i][na].n1[j]; // 找出cover最多rule的interval
            }
        }

        //printf("group %c bucket size(lv1): %d\n", i + 65, thres[i]);
    }

    for (i = 0; i < 3; i++) {
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
                    gp[i][na].lv2[j].rule     = (int *) calloc ((gp[i][na].n1[j] * 2 + 1) ,sizeof(int)); //child
                    gp[i][na].lv2[j].r        = 0; // bucket size

                    for(k=0; k< (gp[i][na].n1[j] * 2 + 1); k++){
                        gp[i][na].lv2[j].b[k] = (struct bucket *)malloc(sizeof(struct bucket));
                        gp[i][na].lv2[j].b_type[k] = 1;
                    }
                }
            }
        }
    }

    for(i=0; i< 3; i++){
        for(na=0; na<65536; na++){
            for(j=0; j<gp[i][na].r; j++){
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
    for(i=0; i< 3; i++){
        for(na=0; na<65536; na++){
            for(j=0; j<gp[i][na].r; j++){
                ruleID = gp[i][na].rule[j];

                ip = table[ruleID].srcIP;
                len = table[ruleID].srclen;

                ip2 = table[ruleID].dstIP;
                len2 = table[ruleID].dstlen;

                interval_operation(2, ruleID, ip, len, ip2, len2, gp[i][na].n, gp[i][na].endpoint, NULL, gp[i][na].lv2, 0);
            }
        }
    }

    for(i=0; i<3; i++){
        for(na=0; na<65536; na++)
        {
            N = gp[i][na].n;

            for(j=1; j<N; j++){
                for(k=0; k<gp[i][na].lv2[j].r; k++){
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
            for(j = 1; j < N; j++){
                for(k=1; k<gp[i][na].lv2[j].n; k++){
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
/*
void convert() {
    int i, j, k, l, m, n = 0, N, nn;

    char s[] = "start converting to new rule ID ...";
    printf("%-40s", s);

    table3 = (struct ENTRY3 *) malloc (2500 * sizeof(struct ENTRY3)); // 3-field sub-rules table

    for (i = 0; i < num_entry; i++) {
        if (table[i].srcPort[0] == table[i].srcPort[1])
            table[i].type += 4;
        else if (table[i].srcPort[0] != 0 || table[i].srcPort[1] != 65535)
            table[i].type += 8;

        if (table[i].dstPort[0] == table[i].dstPort[1])
            table[i].type += 1;
        else if (table[i].dstPort[0] != 0 || table[i].dstPort[1] != 65535)
            table[i].type += 2;

        for (j = 0; j < n; j++) {
            if (table[i].srcPort[0] == table3[j].Port[0] && table[i].srcPort[1] == table3[j].Port[1] && table[i].dstPort[0] == table3[j].Port[2] && table[i].dstPort[1] == table3[j].Port[3] && table[i].proto == table3[j].proto) {
                table[i].rule = j + 1; //why +1 ??
                break;
            }
        } //Rule ID mapping

        if (j == n) { //代表找不到存在的mapping, 新增進去
            table[i].rule = n + 1; // 還是不懂
            table3[n].n = 1; // map到這個New Rule的數量?
            table3[n].type = table[i].type; // type不知道要幹嘛
            table3[n].Port[0] = table[i].srcPort[0];
            table3[n].Port[1] = table[i].srcPort[1];
            table3[n].Port[2] = table[i].dstPort[0];
            table3[n].Port[3] = table[i].dstPort[1];
            table3[n++].proto = table[i].proto;
        }
    }

    for (i = 0; i < n; i++) {
        switch (table3[i].type) {
        case  1: // dst port l, r相同; src port 不同, src port l == 0, r == 65535
            table3[i].Port[0] = table3[i].Port[2];
            break;
        case  2: //dst port l, r不同; src port 不同, src port l == 0, r == 65535
            table3[i].Port[0] = table3[i].Port[2];
            table3[i].Port[1] = table3[i].Port[3];
            break;
        case  5: //dst port l, r相同; src port l, r相同
            table3[i].Port[1] = table3[i].Port[2];
            break;
        case  6: //dst port l, r不同; src port 不同
            table3[i].Port[0] = table3[i].Port[2];
            table3[i].Port[1] = table3[i].Port[3];
            table3[i].Port[2] = table3[i].Port[0];
            break;
        default:
            break;
        }
    }

    numcombine = n;
    //printf("number of combination of port & protocol: %d\n\n", n);
    nn = n;

    int *tmp, na;
    for (m = 0; m < 4; m++) {
        for (na = 0; na < 32; na++) {
            N = gp[m][na].n; // dim 1 的endpoints

            for (i = 1; i < N; i++) {
                for (j = 1; j < gp[m][na].lv2[i].n; j++) {
                    n = 0;

                    for (k = 0; k < gp[m][na].lv2[i].b[j].r; k++) { //traverse child bucket
                        for (l = 0; l < n; l++) {
                            if (gp[m][na].lv2[i].b[j].rule2[l] == table[gp[m][na].lv2[i].b[j].rule[k]].rule) break;
                        }

                        if (l == n) gp[m][na].lv2[i].b[j].rule2[n++] = table[gp[m][na].lv2[i].b[j].rule[k]].rule; // merge bucket
                    }
                    //change rule, rule2, r, r2
                    gp[m][na].lv2[i].b[j].r2 = gp[m][na].lv2[i].b[j].r;
                    gp[m][na].lv2[i].b[j].r = n;
                    tmp = gp[m][na].lv2[i].b[j].rule;
                    gp[m][na].lv2[i].b[j].rule = gp[m][na].lv2[i].b[j].rule2;
                    gp[m][na].lv2[i].b[j].rule2 = tmp;

                    if (n > thres3[m]) thres3[m] = n;
                }
            }
        }
        //printf("group %c bucket size: %-4d\n", m + 65, thres3[m]);
    }

    printf("finish\n");
}*/
void l1_bucket_share() {

    int i, j, k, na;
    int N, ruleID;

    int *uni[500000];
    int uni_l[500000] = {0};
    int uni_N = 0;

    int *now;
    int l;
    for(i=0; i<3; i++){
        for(na=0; na<65536; na++){
            N = gp[i][na].n;
            for(j=1; j<N; j++){
                if(gp[i][na].lv2[j].r == 0) continue;

                now = gp[i][na].lv2[j].rule;
                l = gp[i][na].lv2[j].r;
                for(k=0; k<uni_N; k++){
                    if(l != uni_l[k]) continue;
                    
                    if(rule_check_exact(now, uni[k], l, l)){
                        free(gp[i][na].lv2[j].rule);
                        gp[i][na].lv2[j].rule = uni[k];
                        gp[i][na].lv2[j].type = 0;
                        break;
                    }
                }
                if(k==uni_N) {
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
    printf("start bucket_share\n");
    int i, j, k, l, na;
    int N, ruleID;

    struct bucket *uni[500000];
    int uni_N = 0;

    struct bucket *now;
    for(i=0; i<3; i++){
        for(na=0; na<65536; na++){
            N = gp[i][na].n;
            for(j=1; j<N; j++){
                for(k=0; k<gp[i][na].lv2[j].n; k++){
                    now = gp[i][na].lv2[j].b[k];
                    if(now->r == 0) continue;
                    for(l=0; l<uni_N; l++){
                        if(now->r != uni[l]->r ) continue;
                        
                        if(rule_check_exact(now->rule, uni[l]->rule, now->r, now->r)){
                            free(gp[i][na].lv2[j].b[k]);
                            gp[i][na].lv2[j].b[k] = uni[l];
                            gp[i][na].lv2[j].b_type[k] = 0;
                            break;
                        }
                    }
                    if(l==uni_N) {
                        uni[uni_N++] = now;
                        gp[i][na].lv2[j].b_type[k] = 1;
                    }
                }
            }
        }
    }
    printf("finist bucket share\n");
    /*printf("uni_bucket num: %d\n", uni_N);
    for(i=0; i<uni_N; i++){
        for(j=0; j<uni[i]->r; j++){
            printf("%d ", uni[i]->rule[j]+1);
        }
        printf("\n");
    }*/
}/*
void bucket_merge() {
    
}/*
/*
void software_compress() {
    int i, j, k, l, m, T, *pn, *rn, nor1, nor2, N; // nor = num of rule
    int aa, bb, zz, xx;
    unsigned int b;
    struct bucket *p[500000]; //distinct bucket
    int *R[250000]; //pointer to bucket rule array
    int na;

    char s[] = "start compressing bucket ...";
    //printf("%-40s", s);

    for (m = 0; m < 4; m++) {
        pn = &pnp[m]; // wait to combine num
        rn = &rnr[m]; // bucket num
        T = thres3[m];
        //printf("compress group %c ...\n", m + 65);

        int same = 0;

        for (na = 0; na < 32; na++) {

            N = gp[m][na].n;

            for (i = 1; i < N; i++) {
                for (j = 1; j < gp[m][na].lv2[i].n; j++) {
                    nor1 = gp[m][na].lv2[i].b[j].r;
                    if (nor1 == 0) continue;

                    num_bucket[m]++; //總bucket使用

                    if (nor1 == T) { // 目前bucket 就是最大bucket
                        gp[m][na].lv2[i].b[j].set = 1;
                        for (k = 0; k < *rn; k++)
                            if (rule_check(R[k], gp[m][na].lv2[i].b[j].rule, T, T, 2) != 2) break;

                        //rule_check(a,b,n1,n2,2) 如果b cover a, return 0, 不然就return 2

                        if (k == *rn) { // 第0次 or 都沒有break(代表每次check結果都不一樣)
                            R[(*rn)++] = gp[m][na].lv2[i].b[j].rule;
                        }
                    }

                    for (k = 0; k < *pn; k++) {
                        if (gp[m][na].lv2[i].b[j].r != (*p[k]).r) { //略過那些明顯不一樣der
                            continue;
                        }

                        if (rule_check((*p[k]).rule, gp[m][na].lv2[i].b[j].rule, (*p[k]).r, (*p[k]).r, 2) != 2) {
                            gp[m][na].lv2[i].b[j].set = 1;
                            same++;
                            break;
                        }
                    }

                    //第0次 or 沒有break(代表每次check結果都不一樣)
                    //把bucket位置存起來
                    if (k == *pn) p[(*pn)++] = &(gp[m][na].lv2[i].b[j]);
                }
            }
        }
        //找完unique bucket
        uni_bucket[m] = num_bucket[m] - same;
        //printf("same buckets: %d\n", same);
        //printf("remain %-6d buckets to compress ...\n", *pn);

        for (i = 0; i < *pn; i++) {
            if ((*p[i]).set != 0) //找到符合mbsize的buckets
                continue;

            for (j = 0; j < *pn; j++) {
                if ((*p[j]).set == 2 || i == j)
                    continue;

                if (rule_check((*p[j]).rule, (*p[i]).rule, (*p[j]).r, (*p[i]).r, 2) != 2) {
                    (*p[i]).set = 2; //有cover別人
                    break;
                }
            }
        } // 計算bucket是否cover其他bucket, 有的話set=2

        int count = 0;
        for (i = 0; i < *pn; i++) {
            if ((*p[i]).set == 0) count++;
        }
        //printf("remain %-6d buckets to compress ...\n", count);

        int max, max2, maxn, sum;
        for (i = 0; i < *pn; i++) {
            if ((*p[i]).set != 0) continue;

            maxn = -1;
            max = 0;
            max2 = 0;
            for (j = 0; j < *pn; j++) {
                if ((*p[j]).set != 0 || i == j) continue;

                sum = rule_check((*p[j]).rule, (*p[i]).rule, (*p[j]).r, (*p[i]).r, 3);
                if (sum <= T) {
                    if ((*p[i]).r + (*p[j]).r - sum > max2) {
                        max2 = (*p[i]).r + (*p[j]).r - sum;
                        max = sum;
                        maxn = j;
                    }
                    if ((*p[i]).r + (*p[j]).r - sum == max2) {
                        if (sum > max) {
                            max = sum;
                            maxn = j;
                        }
                    }
                }
            }

            if (maxn == -1) {
                (*p[i]).set = 3; //怎麼組合 sum都大於T, 或是 ...?  (找不到人配QQ)
                continue;
            }

            (*p[i]).set = 2;    //
            for (j = 0; j < (*p[i]).r; j++) {
                l = 0;

                for (k = 0; k < (*p[maxn]).r; k++) {
                    if ((*p[i]).rule[j] == (*p[maxn]).rule[k]) {
                        l = 1;
                        break;
                    }
                }

                if (l == 0) (*p[maxn]).rule[(*p[maxn]).r++] = (*p[i]).rule[j];
            }

            if ((*p[maxn]).r == T) {
                (*p[maxn]).set = 1;
                R[(*rn)++] = (*p[maxn]).rule;
            }
        }

        for (i = 0; i < *pn; i++) {
            if ((*p[i]).set == 0 || (*p[i]).set == 3) R[(*rn)++] = (*p[i]).rule;
        }

        count = 0;

        for (i = 0; i < *pn; i++) {
            if ((*p[i]).set == 0 || (*p[i]).set == 3) {
                count++;
                count2[m] += T - (*p[i]).r;
            }
        }

        //printf("total buckets after compress: %d\n", *rn);
        //printf("bucket do not well compress: %d %d\n", count, count2[m]);
    }

    printf("finish\n\n");
}*/