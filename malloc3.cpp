#ifndef MALLOC3
#define MALLOC3

#define WITH_NOTHING 1
#define WITH_PREV 2
#define WITH_NEXT 3
#define WITH_BOTH 4
#define LAST 5

#include <unistd.h>
#include <sys/mman.h>
#include <iostream>
#include <stdio.h>
#include <string.h>
#include <math.h>

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
MallocMetadata* _tail; // Global variable to the last element of the linked list (used in challenge 3)
MallocMetadata* _m_list_head; // Global variable for head of list of 128kb (or bigger) blocks

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
    cur = _m_list_head;
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
    cur = _m_list_head;
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

void _insertToMLIST(MallocMetadata* element) {
    if (_m_list_head == NULL) {
        _m_list_head = element;
        return;
    }

    MallocMetadata* cur = _m_list_head;
    while (cur->next != NULL) {
        cur = cur->next;
    }

    cur->next = element;
    element->prev = cur;
}

/*
 * Removes an MallocMetadata from the mList linked list
 */
void _removeFromMLIST(MallocMetadata* element) {
    MallocMetadata* cur = _m_list_head;
    while (cur != NULL && cur != element) { // the list is sorted by the elements address
        cur = cur->next;
    }

    if (_m_list_head == NULL) {
        cout << "not suppose to get here222" << endl;
        return;
    }

    MallocMetadata* element_next = cur->next;
    MallocMetadata* element_prev = cur->prev;
    if (element_next) element_next->prev = element_prev;
    if (element_prev) element_prev->next = element_next;
    if (cur == _m_list_head) _m_list_head = cur->next;
}

/*
 * create new metadata at the beginning of the block
 */
void _createNewMetaDataForMLIB(void* addr, size_t size, MallocMetadata** new_meta2) {
    *new_meta2 = (MallocMetadata*) addr;
    MallocMetadata* new_meta = *new_meta2;
    (new_meta->is_free) = true;
    new_meta->size = size;
    _insertToMLIST(new_meta);
}

void* allocWithMmap(size_t size) {
    void* ret_addr = mmap(NULL, (size+ sizeof(MallocMetadata)), PROT_READ | PROT_WRITE,
                                                                        MAP_ANONYMOUS | MAP_PRIVATE, -1, 0);
    if (ret_addr == MAP_FAILED) return NULL;

    MallocMetadata* new_meta;
    _createNewMetaDataForMLIB(ret_addr, size, &new_meta);
    new_meta->is_free = false;

    void* ret_addr2 = (void*) ((int64_t)ret_addr + sizeof(MallocMetadata)); // Get the actual data address
    return ret_addr2;
}

/*
 * We have a linked list of the metadata sorted by address
 */
void _insert(MallocMetadata* element) {
    if (_head == NULL) {
        _head = _tail = element;
        return;
    }

    MallocMetadata* cur = _head;
    while (cur->next != NULL && cur->next < element) { // the list is sorted by the elements address
        cur = cur->next;
    }

    MallocMetadata* element_next = cur->next;
    cur->next = element;
    element->next = element_next;
    element->prev = cur;
    if (element_next) element_next->prev = element;
    if (cur == _tail || _tail == NULL) _tail = element; // update tail
    cur = _head;
    return;
}

bool _check(size_t size) {
    if (size == 0 || size > pow(10,8) ) return false;
    return true;
}

/*
 * create new metadata at the beginning of the block
 */
void _createNewMetaData(void* addr, size_t size, MallocMetadata** new_meta2) {
    *new_meta2 = (MallocMetadata*) addr;
    MallocMetadata* new_meta = *new_meta2;
    (new_meta->is_free) = false;
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

void _split_block(MallocMetadata* element, size_t size) {
    element->is_free = false;
    if (element->size - size < 128 + sizeof(MallocMetadata)) return; // If not big enough to split
    MallocMetadata* splited_part_meta;
    size_t new_block_size = element->size - size - sizeof(MallocMetadata);
    int aa = sizeof(MallocMetadata);
    void* start_adress = (void*) ( ((int64_t) element) + aa + size);
    element->size = size;
    if (new_block_size > 128*1024) {
        munmap(start_adress, new_block_size);
    }
    else {
        _createNewMetaData(start_adress, new_block_size, &splited_part_meta);
        splited_part_meta->is_free = true;
    }
}

void* _addToWilderness(size_t size) {
    void* sbrk_ret = sbrk(size - _tail->size); // increase heap
    int val = *((int*)sbrk_ret);
    if (val == -1) return NULL;
    _tail->is_free = false;
    _tail->size = size;
    return (void*) ((int64_t)_tail + sizeof(MallocMetadata));
}

void* smalloc(size_t size) {
    if (_check(size) == false) return NULL;

    if (size > 128*1024) return allocWithMmap(size);

    MallocMetadata* empty_block = _find(size);
    if (empty_block != NULL) { // if found an existing empty block
        _split_block(empty_block, size);
        return (void*) ((int64_t)empty_block + sizeof(MallocMetadata));
    }

    if (_tail && _tail->is_free) return _addToWilderness(size);

    void* sbrk_ret = sbrk(size + sizeof(MallocMetadata)); // increase heap
    int val = *((int*)sbrk_ret);
    if (val == -1) return NULL;

    MallocMetadata* new_meta;
    _createNewMetaData(sbrk_ret, size, &new_meta);
    new_meta->is_free = false;
    _split_block(new_meta, size);

    void* ret_addr = (void*) ((int64_t)sbrk_ret + sizeof(MallocMetadata)); // Get the actual data address
    return ret_addr;
}

void* scalloc(size_t num, size_t size) {
    void* ret_addr = smalloc(size*num);
    if (ret_addr == NULL) return NULL;
    memset(ret_addr, 0, size*num);
    return ret_addr;
}

static void glueTogether(MallocMetadata* toFree){
    MallocMetadata* prev = toFree->prev;
    MallocMetadata* next = toFree->next;
    MallocMetadata* start = toFree;
    size_t size = toFree->size;
    if (prev && prev->is_free){
        start = prev;
        size = size + prev->size + sizeof(MallocMetadata);
        start->next = next;
        start->size = size;
        if (_tail == toFree) _tail = start;
        if(next) next->prev = start;

    }
    if (next && next->is_free){
        size = size + next->size + sizeof(MallocMetadata);
        start->next = next->next;
        start->size = size;
        if (_tail == next) _tail = start;
        if (next->next) next->next->prev = start;
    }
    start->is_free = true;

}

void sfree(void* p) {
    if (p == NULL) return;;
    MallocMetadata* meta;
    _getMetaData(p, &meta);
    if (meta->is_free) return;
    if (meta->size >= 128*1024) {
        _removeFromMLIST(meta);
        munmap(meta, meta->size+ sizeof(MallocMetadata));
    } else glueTogether(meta);
}

static int check_realloc_case(MallocMetadata* meta, size_t size){
    if (_tail == meta) return LAST;
    if (meta->size >= size) return WITH_NOTHING;
    MallocMetadata* prev = meta->prev;
    if (prev && meta->size + prev->size >= size && prev->is_free) return WITH_PREV;
    MallocMetadata* next = meta->next;
    if (next && meta->size + next->size >= size && next->is_free) return WITH_NEXT;
    if (next && prev && meta->size + prev->size + next->size >= size && prev->is_free && next->is_free) return WITH_BOTH;
    return 0;
}

static void* srealloc_helper(MallocMetadata* orig, MallocMetadata* base, size_t size, size_t orig_size){
    void* copy_from = (void*) (((int64_t)orig) + sizeof(MallocMetadata));
    void* copy_to = (void*) ( ((int64_t)base) + sizeof(MallocMetadata));
    base->size = size;
    memcpy(copy_to, copy_from, orig_size);
    _split_block(base,orig_size);
    return copy_to;
}

void* srealloc(void* oldp, size_t size) {
    if (oldp == NULL)
        return smalloc(size);

    MallocMetadata* meta;
    _getMetaData(oldp, &meta);
    int situation = check_realloc_case(meta,size);
    switch (situation){
        case LAST:
            _addToWilderness(size);
        case WITH_NOTHING:
            _split_block(meta,meta->size);
            return oldp;
        case WITH_PREV:
            meta->prev->next = meta->next;
            if (meta->next) meta->next->prev = meta->prev;
            return srealloc_helper(meta,meta->prev,meta->size + meta->prev->size + sizeof(MallocMetadata), meta->size);
        case WITH_NEXT:
            meta->next = meta->next->next;
            if(meta->next->next) meta->next->next->prev = meta;
            return srealloc_helper(meta,meta,meta->size + meta->next->size + sizeof(MallocMetadata),meta->size);
        case WITH_BOTH:
            meta->prev->next = meta->next->next;
            if(meta->next->next)meta->next->next->prev = meta->prev;
            return srealloc_helper(meta,meta->prev,meta->size + meta->next->size + meta->prev->size
                                                                                   + 2*sizeof(MallocMetadata),meta->size);
        default:
            break;
    }

    void* new_addr = smalloc(size);
    if (new_addr == NULL) return NULL;
    MallocMetadata* meta2;
    // void* a = (void*) ((int64_t) new_addr + meta->size);
    _getMetaData(new_addr, &meta2);
    int aa = sizeof(MallocMetadata);
    void* a = (void*) ( ((int64_t) meta2) + aa + meta->size );
    memcpy(new_addr, oldp, size);
    _split_block(meta2, meta->size);
    sfree(oldp);
    return new_addr;
}

/*
void* srealloc(void* oldp, size_t size) {
    if (oldp == NULL)
        return smalloc(size);

    MallocMetadata* meta;
    MallocMetadata* meta2;
    _getMetaData(oldp, &meta);
    int oldSize = meta->size;
    if (meta->size >= size) return oldp;

    void* new_addr = smalloc(size);
    _getMetaData(new_addr, &meta2);
    _split_block(meta2, oldSize);
    if (new_addr == NULL) return NULL;

    memcpy(new_addr, oldp, size);
    sfree(oldp);
    return new_addr;
}
*/
#endif // MALLOC3