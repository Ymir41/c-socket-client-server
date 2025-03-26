//
// Created by janek on 3/24/25.
//

#include "common.h"
#include <stdint.h>
#include <endian.h>

double htond(double num){
    uint64_t  *num_cast = (uint64_t*) &num;
    uint64_t be = htobe64(*num_cast);
    return *(double *) &be;
}

double ntohd(double num){
    uint64_t  *num_cast = (uint64_t*) &num;
    uint64_t be = be64toh(*num_cast);
    return *(double *) &be;
}