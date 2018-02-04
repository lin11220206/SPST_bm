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
    //l1_bucket_share();
    l2_bucket_share();
    level2_duplication('B');
    level2_duplication('C');
    level2_duplication('D');
    //level1_duplication('B');
    //level1_duplication('C');
    //level1_duplication('D');
    //level2_duplication(argv[2][0]);
    //convert();
    //software_compress();
    
    return 0;
}
