#ifndef TWIT_HASH_H
#define TWIT_HASH_H

uint32_t twit_hash(const void *key, /* the key to hash */
size_t length, /* length of the key */
const uint32_t initval); /* initval */

#endif
