#include "structure.h"
#include "function.h"

void add_endpoint(unsigned int *array, int *n, unsigned int a, unsigned int b) {
    int i, j, c = 0;
    int p = *n;

    for (i = 0; i < *n; i++) {
        if (a == array[i]) {
            if (c == 0) {
                a = b;
                c = 1;
            }
            else break;
        }
        else if (a < array[i]) {
            for (j = *n; j > i; j--) {
                array[j] = array[j - 1];
            }

            array[i] = a;
            (*n)++;

            if (c == 0) {
                c = 1;
                a = b;
            }
            else break;
        }
        if (i == *n - 1) {
            array[(*n)++] = a;

            if (c == 0) {
                c = 1;
                a = b;
            }
            else break;
        }
    }
}

void interval_operation(int op, int ID, unsigned int ip1, int len1, int ip2, int len2, int N, unsigned int *endpoint, int *op1, struct level2 *lv2, int n) {
    unsigned int l1, r1, l2, r2;
    int i, j, c = 0;

    l1 = ip1;
    r1 = (len1 == 0) ? -1 : (((ip1 >> (32 - len1)) + 1) << (32 - len1)) - 1;

    if (op == 2) {
        l2 = (ip2 == 0)  ? 0  : ip2 - 1;
        r2 = (len2 == 0) ? -1 : (((ip2 >> (32 - len2)) + 1) << (32 - len2)) - 1;
    }

    for (i = 1; i < N; i++) {
        if (c == 0 && endpoint[i] >= l1) c = 1;

        if (c == 1) {
            if (op == 1) op1[i]++;
            if (op == 2) {
                lv2[i].rule[lv2[i].r++] = ID; // 這裡將rule放進第一層對應的child裡面
                add_endpoint(lv2[i].endpoint, &(lv2[i].n), l2, r2); // 將Dst l, r放進對應的child endpoints 裡面
            }
            if (op == 3) lv2[n].b[i]->rule[lv2[n].b[i]->r++] = ID;
        }

        if (c == 1 && endpoint[i] >= r1) break;
    }
}

int rule_check(int *a, int *b, int n1, int n2, int t) {
    int i, j, check;

    if (t == 1) { // index of a of b[na]
        for (i = 0; i < n1; i++) {
            if (a[i] == b[n2]) return i;
        }
    }

    if (t == 2) { // check if a cover all b
        for (i = 0; i < n2; i++) {
            check = 0;

            for (j = 0; j < n1; j++) {
                if (a[j] == b[i]) check = 1;
            }

            if (check != 1) return 2;
        }
    }

    if (t == 3) {
        check = n1 + n2;

        for (i = 0; i < n1; i++) {
            for (j = 0; j < n2; j++) {
                if (a[i] == b[j]) {
                    check--;
                    break;
                }
            }
        }

        return check;
    }

    return 0;
}

int count_bit(int n) {
    return (n) ? count_bit(n / 2) + 1 : 0;
}

int log_2(int n) {
    return count_bit(n) - 1;
}

int rule_check_exact(int *b1, int *b2, int r1, int r2) { //check if bucket b1 == b2
    int i;
    if(r1 != r2) return 0;

    for(i=0; i<r1; i++){
        if(b1[i] != b2[i]) return 0;
    }
    return 1;
}
int rule_check_cover(int *b1, int *b2, int r1, int r2) { //check if bucket b2 cover b1
    int i, j, check;
    for(i=0; i<r1; i++){
        check = 0;
        for(j=0; j<r2; j++){
            if(b2[j] == b1[i]) check = 1;
        }

        if(check != 1) return 0;
    }
    return 1;
}
int rule_check_merge(int *b1, int *b2, int r1, int r2, int max_size) {
    int i, j, sum;
    sum = r1+r2;
    for(i=0; i<r1; i++){
        for(j=0; j<r2; j++){
            if(b1[i] == b2[j]) {
                sum--;
                break;
            }
        }
    }
    if( sum <= max_size) { //merge two bucket
        for(i=0; i<r1; i++){
            for(j=0; j<r2; j++){
                if(b1[i] == b2[j]) break;
            }
            if(j==r2) {
                b2[r2++] = b1[i];
            }
        }
        qsort(b2, r2, sizeof(int), cmp_r);

        return r2;
    }
    return 0;
}

int cmp_r(const void *a, const void *b){
    return *(int *)a - *(int *)b;
}