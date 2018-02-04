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
    //level1_duplication(argv[2][0]);
    level2_duplication(argv[2][0]);
    //convert();
    //software_compress();
    
    return 0;
}
