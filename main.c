#include <stdio.h>
#include "header.h"
#include "structure.h"
#include "function.h"
#include "result.h"
#include "clock.h"
#include "global.h"

int main(int argc, char *argv[]) {
    set(argv[2]);
    //print_setting();
    set_table(argv[1]);

    groupping();
    first_level();
    second_level();
    l1_bucket_share();
    l2_bucket_share();
    
    if(setting[0].rebuild || setting[1].rebuild || setting[2].rebuild || setting[3].rebuild) {
        rebuild();
        groupping();
        first_level();
        second_level();
        l1_bucket_share();
        l2_bucket_share();
    }
    convert();
    bucket_merge();

    get_result();
    compute_memory_use();

    int i;

    printf("Duplication\n");
    for(i=0; i<4; i++) {
        show_duplication(i);
        show_duplication(i+4);
    }

    show_buckets_data();

    printf("nodes level\n");
    for(i=0; i<4; i++) {
        show_nodes_count(i);
        show_nodes_count(i+4);
    }

    printf("memory use\n");
    show_memory_use();
    
    return 0;
}
