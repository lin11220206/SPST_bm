#include <stdio.h>
#include "header.h"
#include "structure.h"
#include "function.h"
#include "result.h"
#include "clock.h"

int main(int argc, char *argv[]) {
    set_table(argv[1]);

    groupping();
    first_level();
    second_level();
    //convert();
    l1_bucket_share();
    printf("dim1 duplication\n");
    level1_duplication(0);
    level1_duplication(1);
    level1_duplication(2);
    level1_duplication(3);
    level1_duplication(4);
    level1_duplication(5);
    l2_bucket_share();
    printf("dim2 duplication\n");
    level2_duplication(0);
    level2_duplication(1);
    level2_duplication(2);
    level2_duplication(3);
    level2_duplication(4);
    level2_duplication(5);
    bucket_merge();
    printf("merge duplication\n");
    merge_duplication(0);
    merge_duplication(1);
    merge_duplication(2);
    merge_duplication(3);
    merge_duplication(4);
    merge_duplication(5);

    printf("EIs\n");
    result2(0);
    result2(1);
    result2(2);
    result2(3);
    result2(4);
    result2(5);

    //convert();
    //software_compress();
    
    return 0;
}
