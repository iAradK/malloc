#ifndef MALLOC2
#define MALLOC2

#include <unistd.h>
#include <iostream>
#include <stdio.h>
#include <string.h>


using namespace std;

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

size_t _num_free_blocks() {
    size_t num = 0;
    MallocMetadata* cur = _head;
    while (cur != NULL) {
        if (cur->is_free) num++;
        cur = cur->next;
    }
    return num;
}

size_t _num_free_bytes() {
    size_t num = 0;
    MallocMetadata* cur = _head;
    while (cur != NULL) {
        if (cur->is_free) num += cur->size;
        cur = cur->next;
    }
    return num;
}

size_t _num_allocated_blocks() {
    size_t num = 0;
    MallocMetadata* cur = _head;
    while (cur != NULL) {
        num++;
        cur = cur->next;
    }
    return num;
}

size_t _num_allocated_bytes() {
    size_t num = 0;
    MallocMetadata* cur = _head;
    while (cur != NULL) {
        num += cur->size;
        cur = cur->next;
    }
    return num;
}

size_t _size_meta_data() {
    return sizeof(MallocMetadata);
}

size_t _num_meta_data_bytes() {
    return _num_allocated_blocks()*_size_meta_data();
}

/*
 * We have a linked list of the metadata sorted by address
 */
void _insert(MallocMetadata* element) {
    if (_head == NULL) {
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
    if (element_next) element_next->prev = cur;
}

bool _check(size_t size) {
    if (size == 0 || size > (10 << 8) ) return false;
    return true;
}

/*
 * create new metadata at the beginning of the block
 */
void _createNewMetaData(void* addr, size_t size, MallocMetadata** new_meta2) {
    *new_meta2 = (MallocMetadata*) addr;
    MallocMetadata* new_meta = *new_meta2;
    (new_meta->is_free) = true;
    new_meta->size = size;
    _insert(new_meta);
}

/*
 * Gets a block and returns its metadata
 */
void _getMetaData(void* addr, MallocMetadata** meta) {
    void* meta_addr = (void*) ((int64_t)addr - sizeof(MallocMetadata));
    *meta = (MallocMetadata*) meta_addr;
}

void* smalloc(size_t size) {
    if (_check(size) == false) return NULL;
    MallocMetadata* empty_block = _find(size);
    if (empty_block != NULL) // if found an existing empty block
    {
        empty_block->is_free = false;
        return (void *) ((int64_t) empty_block + sizeof(MallocMetadata));
    }


    void* sbrk_ret = sbrk(size + sizeof(MallocMetadata)); // increase heap
    int val = *((int*)sbrk_ret);
    if (val == -1) return NULL;

    MallocMetadata* new_meta;
    _createNewMetaData(sbrk_ret, size, &new_meta);
    new_meta->is_free = false;

    void* ret_addr = (void*) ((int64_t)sbrk_ret + sizeof(MallocMetadata)); // Get the actual data address
    return ret_addr;
}

void* scalloc(size_t num, size_t size) {
    void* ret_addr = smalloc(size*num);
    if (ret_addr == NULL) return NULL;
    memset(ret_addr, 0, size*num);
    return ret_addr;
}

void sfree(void* p) {
    if (p == NULL) return;;
    MallocMetadata* meta;
    _getMetaData(p, &meta);
    meta->is_free = true;
}

void* srealloc(void* oldp, size_t size) {
    if (oldp == NULL)
        return smalloc(size);

    if (!_check(size))
        return NULL;
    MallocMetadata* meta;
    _getMetaData(oldp, &meta);
    if (meta->size >= size) return oldp;

    void* new_addr = smalloc(size);
    if (new_addr == NULL) return NULL;

    memcpy(new_addr, oldp, size);
    sfree(oldp);
    return new_addr;
}

#endif // MALLOC2