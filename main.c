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
    //level1_duplication('B');
    //level1_duplication('C');
    //level1_duplication('D');
    //convert();
    //l1_bucket_share();
    //l2_bucket_share();
    //bucket_merge();
    //merge_duplication('B');
    //merge_duplication('C');
    //merge_duplication('D');
    //level2_duplication('B');
    //bucket_merge();
    //merge_duplication('B');
    //merge_duplication('C');
    //merge_duplication('D');
    result2('B');
    result2('C');
    result2('D');

    //convert();
    //software_compress();
    
    return 0;
}
