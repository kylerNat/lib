#ifndef HASHMAP_COMMON
#define HASHMAP_COMMON

#include "logging.h"
#include "str.h"

#define MAP_FULL 0xFFFFFFFF

uint djb2(char* key) //djb2
{
    uint out = 5381;
    while(*key) out += ((out << 5) + out) ^ *key++;
    return out;
}

uint djb2(strand key) //djb2
{
    uint out = 5381;
    for(int s = 0; s < key.len; s++) out += ((out << 5) + out) ^ key.str[s];
    return out;
}

uint bsd_checksum(char* key)
{
    uint checksum = 0;             /* The checksum mod 2^16. */

    while(*key)
    {
        checksum = (checksum >> 1) + ((checksum & 1) << 0xF);
        checksum += *key++;
        checksum &= 0xffff;       /* Keep it within bounds. */
    }
    return checksum;
}

#define traverse_hashmap_(key, key_hash, keys, max_size, exists_case, empty_case, full_case) \
    int hash_value = key_hash%max_size;                                 \
    int index = hash_value;                                             \
    for ever                                                            \
    {                                                                   \
        if(equal(keys[index], key)) {exists_case; break;}               \
                                                                        \
        if(keys[index] == 0) {empty_case; break;}                       \
        index = (index+1) % max_size;                                   \
        if(index == hash_value) {full_case; break;}                     \
    }

#endif //HASHMAP_COMMON
