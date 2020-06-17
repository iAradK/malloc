#ifndef MALLOC2
#define MALLOC2

#include <unistd.h>

struct MallocMetadata {
    size_t size;
    bool is_free;
    MallocMetadata* next;
    MallocMetadata* prev;

    MallocMetadata (size_t size) {
        this->is_free = true;
        this->next = NULL;
        this->prev = NULL;
        this->size = size;
    }
};

MallocMetadata* _head; // Global variable to the head of the linked list

/*
 * Searches for an empty block that fits the size
 */
MallocMetadata* _find(size_t size) {
    MallocMetadata* cur = _head;
    while (cur != NULL) { // the list is sorted by the elements address
        if (cur->is_free && cur->size >= size)
            return cur;
        cur = cur->next;
    }

    return NULL;
}

/*
 * We have a linked list of the metadata sorted by address
 */
void _insert(MallocMetadata* element) {
    if (_head = NULL) {
        _head = element;
        return;
    }

    MallocMetadata* cur = _head;
    while (cur->next != NULL && cur->next < element) { // the list is sorted by the elements address
        cur = cur->next;
    }
    MallocMetadata* element_next = cur->next;
    cur->next = element;
    element->prev = cur;
    element->next = element_next;
    element_next->prev = cur;
}

bool _check(size_t size) {
    if (size == 0 || size > (10 << 8) ) return false;
    return true;
}

/*
 * create new metadata at the beginning of the block
 */
void _createNewMetaData(void* addr, size_t size, MallocMetadata* new_meta) {
    new_meta = (MallocMetadata*) addr;
    new_meta->is_free = true;
    new_meta->size = size;
    _insert(new_meta);
}

/*
 * Gets a block and returns its metadata
 */
void _getMetaData(void* addr, MallocMetadata* meta) {
    void* meta_addr = (void*) ((__int64_t)addr - sizeof(MallocMetadata));
    meta = (MallocMetadata*) meta_addr;
}

void* smalloc(size_t size) {
    if (_check(size) == false) return NULL;
    MallocMetadata* empty_block = _find(size);
    if (empty_block != NULL) // if found an existing empty block
        return empty_block;

    void* sbrk_ret = sbrk(size+ sizeof(MallocMetadata)); // increase heap
    int val = *((int*)sbrk_ret);
    if (val == -1) return NULL;

    MallocMetadata* new_meta;
    _createNewMetaData(sbrk_ret, size, new_meta);

    void* ret_addr = (void*) ((__int64_t)sbrk_ret + sizeof(MallocMetadata)); // Get the actual data address
    return ret_addr;
}

void* scalloc(size_t size) {
    void* ret_addr = smalloc(size);
    if (ret_addr == NULL) return NULL;
    for (int i = 0; i < size; i++) *((int*)ret_addr + i) = 0; // Check this...
    return ret_addr;
}

void sfree(void* p){
    if (p == NULL) return;;
    MallocMetadata* meta;
    _getMetaData(p, meta);
    meta->is_free = false;
}

#endif // MALLOC2