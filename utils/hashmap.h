//TODO: I should make define_hashmap macros instead of doing the hacky #define arguments and #include "hashmap.h"
//no include guards, since this file can be reused to define different types of hashmaps
#ifdef value_type

#include <utils/hashmap_common.h>

#ifndef hash_function
#define hash_function djb2
#endif //hash_function

#define traverse_hashmap(key, keys, max_size, exists_case, empty_case, full_case) traverse_hashmap_(key, hash_function(key), keys, max_size, exists_case, empty_case, full_case)

#ifndef key_type
#define key_type char*
#endif

#ifndef hashmapname
#define hashmapname CONCAT(value_type, _hashmap)
#endif

#define kash_t CONCAT(hashmapname, _kash_t)

struct kash_t
{
    key_type key;
    uint hash;
};

struct hashmapname
{
    key_type* keys;
    value_type* values;
    uint max_size;

    inline value_type & operator[](key_type key)
    {
        traverse_hashmap(key, keys, max_size,
                         {return values[index];},
                         {return values[index];},
                         {return values[index];}); //TODO: handle full case
    }

    //for slight perfomance optimization
    inline value_type & operator[](kash_t key_with_hash)
    {
        traverse_hashmap_(key_with_hash.key, key_with_hash.hash, keys, max_size,
                         {return values[index];},
                         {return values[index];},
                         {return values[index];}); //TODO: handle full case
        return values[hash_value%max_size];
    }
};

#define init_hashmapname init_hashmapname_(hashmapname)
#define init_hashmapname_(hmn) init_hashmapname__(hmn)
#define init_hashmapname__(hmn) init_##hmn

hashmapname init_hashmapname(uint max_size)
{ //TODO: real memory management
    hashmapname map = {
        (key_type*) calloc(max_size, sizeof(key_type)),
        (value_type*) calloc(max_size, sizeof(value_type)),
        max_size
    };
    return map;
}

#undef init_hashmapname
#undef init_hashmapname_
#undef init_hashmapname__

uint hash_index(hashmapname map, key_type key)
{
    traverse_hashmap(key, map.keys, map.max_size,
                     return index;,
                     return -index-1;,
                     return MAP_FULL;);
}

uint add_entry(hashmapname map, key_type key, value_type value)
{
    auto& keys = map.keys;
    auto& max_size = map.max_size;
    traverse_hashmap(key, map.keys, map.max_size,
                     { //entry already exists for this key
                         //TODO:
                         log_warning("entry already exists for: ", key, ". overwriting old value ", map.values[index],
                                     " with ", value);
                         return index;
                     },
                     { //empty cell found
                         map.keys[index] = key;
                         map.values[index] = value;
                         return index;
                     },
                     { //hashmap is full
                         #if(EXPANDABLE_HASHMAP)
                         //TODO:
                         return MAP_FULL;
                         #else
                         log_error("non-expandable hashmap is full, cannot add entry: (", key, ", ", value, ")");
                         #endif
                     });
}

inline value_type hash_value(hashmapname map, key_type key)
{
    return map.values[hash_index(map, key)];
}

#undef value_type
#undef hash_function
#undef traverse_hashmap
#undef key_type
#undef EXPANDABLE_HASHMAP
#undef hashmapname
#undef kash_t
#endif //ifdef value_type
