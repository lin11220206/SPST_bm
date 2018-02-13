#include <stdio.h>
#include "header.h"
#include "structure.h"
#include "function.h"
#include "result.h"

int main(int argc, char *argv[]) {
    set_table(argv[1]);

    groupping();
    first_level();
    second_level();
    convert();
    //l1_bucket_share();
    l2_bucket_share();
    //bucket_merge();
    //merge_duplication('B');
    //merge_duplication('C');
    //merge_duplication('D');
    //level2_duplication('B');
    bucket_merge();
    merge_duplication('B');
    merge_duplication('C');
    merge_duplication('D');


    //convert();
    //software_compress();
    
    return 0;
}
