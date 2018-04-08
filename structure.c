#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "header.h"
#include "structure.h"
#include "function.h"
#include "clock.h"
#include "global.h"

struct level1 gp[8][65536];
int thres[8], thres2[8], thres3[8], count2[8], numcombine;
int group[8][65536], groupp[8];
int seg_bit[8];
struct bucket *uni_bucket[500000];
int uni_num = 0;
struct bucket merge_bucket[8][100000];
int mrg_num[8] = {0};

// group: Group A~D, SrcIP 5-bit Seg後各root的rule數, groupp: 該組裡的rule數
// max[i]: Group A~D中擁有最多rules的root
//gp level 1 roots
struct ENTRY *table3;

void rebuild() {
    int i, na, j, k, r;

    printf("start rebuild ...\n");
    int *count = calloc(num_entry , sizeof(int));

    for (i = 0; i < num_entry; i++)
        count[i] = 0;

    int ruleID, N;

    for (i = 0; i < 8; i++) {
        for (na = 0; na < 65536; na++) {
            N = gp[i][na].n;
            for (j = 1; j < N; j++) {
                for (k = 1; k < gp[i][na].lv2[j].n; k++) {
                    if (gp[i][na].lv2[j].b_type[k] != 1) continue;
                    for (r = 0; r < gp[i][na].lv2[j].b[k]->r; r++) {
                        ruleID = gp[i][na].lv2[j].b[k]->rule[r];

                        count[ruleID]++;
                    }
                }
            }
        }
    }

    for (i = 0; i < num_entry; i++) {
        if (count[i] > 1 && table[i].group < 4 && setting[table[i].group].rebuild) {
            table[i].group += 4;
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

    for (i = 0; i < 8; i++)
        for (na = 0; na < 65536; na++)
            group[i][na] = 0;

    memset(thres2, 0, sizeof(int) * 8);
    for (i = 0; i < 500000; i++) {
        uni_bucket[i] = 0;
    }
    uni_num = 0;
}

void groupping() {
    int i, j, segment, g;
    unsigned int ip, len;

    printf("start groupping ...\n");

    for (i = 0; i < 4; i++) {
        seg_bit[i] = setting[i].bit1;
        seg_bit[i + 4] = setting[i].bit2;
    }

    for(i=0; i<8; i++){
        groupp[i] = 0;
        for(j=0; j<65536; j++)
            group[i][j] = 0;
    }

    for (i = 0; i < num_entry; i++) { // count rule number of segmentation roots

        ip = table[i].srcIP;
        len = table[i].srclen;
        g = table[i].group;

        if (setting[g < 4 ? g : g - 4].ignore) continue;

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

void first_level() {
    int i, j, g, N, na, k, segment, ruleID;

    printf("start computing first level ...\n");

    for (i = 0; i < 8; i++) {
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
        //if (table[i].group != setting[0].group) continue; //don't need because similar code in header.c

        ip = table[i].srcIP;
        len = table[i].srclen;
        g = table[i].group;

        if (setting[g < 4 ? g : g - 4].ignore) continue;

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

    int cccc = 0;
    for (i = 0; i < 8; i++) {
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
                if (i == 5) {
                    //printf("%d %X %X\n", gp[i][na].n, l, r);
                    cccc++;
                }
            }
        }
    }

    //printf("TTTTTTTT    %d TTTTTTTTTTTTTTt\n", cccc);
    for (i = 0; i < 8; i++) {
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

    for (i = 0; i < 8; i++) {
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
}

void second_level() {
    int i, j, k, count, g, N, T, na;

    printf("start computing second level ...\n");

    unsigned int l, r, ip, l2, r2, ip2;
    int len, len2;
    int ruleID;
    for (i = 0; i < 8; i++) {
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

    for (i = 0; i < 8; i++) {
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

    table3 = (struct ENTRY *) malloc (10000 * sizeof(struct ENTRY)); // 3-field sub-rules table

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

    for (i = 0; i < 8; i++)
        thres2[i] = 0;

    int *tmp, na;
    for (m = 0; m < 8; m++) {
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
    printf("start l1_bucket_share ...\n");
    int i, j, k, na;
    int N, ruleID;

    int *uni[500000];
    int uni_l[500000] = {0};
    int uni_N = 0;

    int *now;
    int l;
    for (i = 0; i < 8; i++) {
        for (na = 0; na < 65536; na++) {
            N = gp[i][na].n;

            for (j = 1; j < N; j++) {
                if (gp[i][na].lv2[j].r == 0) continue;

                now = gp[i][na].lv2[j].rule;
                l = gp[i][na].lv2[j].r;
                for (k = 0; k < uni_N; k++) {
                    if (l != uni_l[k]) continue;

                    if (table[now[0]].group != table[uni[k][0]].group) continue;

                    if (rule_check_exact(now, uni[k], l, l)) {
                        //free(gp[i][na].lv2[j].rule);
                        //gp[i][na].lv2[j].rule = uni[k];
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
    printf("start l2_bucket_share ...\n");
    int i, j, k, l, na;
    int N, ruleID;

    //uni_bucket = (struct bucket **) calloc(500000 , sizeof(struct bucket *));

    uni_num = 0;
    struct bucket *now;
    for (i = 0; i < 8; i++) {
        for (na = 0; na < 65536; na++) {
            N = gp[i][na].n;

            for (j = 1; j < N; j++) {
                for (k = 0; k < gp[i][na].lv2[j].n; k++) {
                    now = gp[i][na].lv2[j].b[k];
                    if (now->r == 0) continue;
                    for (l = 0; l < uni_num; l++) {
                        if (now->r != uni_bucket[l]->r ) continue;
                        if (table[now->rule[0]].group != table[uni_bucket[l]->rule[0]].group) continue;

                        if (rule_check_exact(now->rule, uni_bucket[l]->rule, now->r, now->r)) {
                            //free(gp[i][na].lv2[j].b[k]);
                            //gp[i][na].lv2[j].b[k] = uni_bucket[l];
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
    printf("start bucket_merge ...\n");
    //qsort(uni_bucket, uni_num, sizeof(struct bucket *), cmp);


    int i, j, k, l, na, g;
    int N, ruleID;
    /*
    for(i=0; i<150; i++){
        printf("%d %p\n", i, uni_bucket[i]);
    }*/
    //sort();

    for (i = 0; i < 8; i++) {
        for (j = 0; j < 100000; j++) {
            merge_bucket[i][j].rule = (int *) calloc(thres2[i], sizeof(int));
            merge_bucket[i][j].r = 0;
        }
    }

    qsort(uni_bucket, uni_num, sizeof(struct bucket *), cmp);

    struct bucket *now;

    for (i = 0; i < uni_num; i++) {
        now = uni_bucket[i];
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