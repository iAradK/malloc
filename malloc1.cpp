#ifndef MALLOC1
#define MALLOC1

#include <unistd.h>



/*
 * Checks if input is valid
 */
bool check(size_t size) {
    if (size == 0 || size > (10 << 8) ) return false;
    return true;
}

void* smalloc(size_t size) {
    if (check(size) == false) return NULL;
    void* sbrk_ret = sbrk(size);
    int val = *((int*)sbrk_ret); // Check if sbrk did not have an error (val is -1 if it had an error)
    return (val == -1) ? NULL : sbrk_ret;
}

#endif // MALLOC1