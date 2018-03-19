#include <stdio.h>
#include "header.h"
#include "structure.h"
#include "function.h"
#include "result.h"
#include "clock.h"
#include "global.h"

int main(int argc, char *argv[]) {
    set(argv);
    print_setting();
    set_table(argv[1]);

    groupping();
    first_level();
    second_level();
    //convert();
    l1_bucket_share();
    printf("===================\n\n");
    printf("dim1 duplication\n");
    
    level1_duplication(setting.group);
    level1_duplication(setting.group+3);

    printf("===================\n\n");
    l2_bucket_share();
    printf("dim2 duplication\n");
    
    level2_duplication(setting.group);
    level2_duplication(setting.group+3);
    printf("===================\n\n");
    bucket_merge();
    printf("merge duplication\n");
    
    merge_duplication(setting.group);
    merge_duplication(setting.group+3);
    printf("===================\n\n");

    printf("EIs\n");
    
    result2(setting.group);
    result2(setting.group+3);
    printf("===================\n\n");

    //convert();
    //software_compress();
    
    return 0;
}
