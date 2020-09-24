/*
 * mm-naive.c - The fastest, least memory-efficient malloc package.
 *
 * In this naive approach, a block is allocated by simply incrementing
 * the brk pointer.  A block is pure payload. There are no headers or
 * footers.  Blocks are never coalesced or reused. Realloc is
 * implemented directly using mm_malloc and mm_free.
 *
 * NOTE TO STUDENTS: Replace this header comment with your own header
 * comment that gives a high level description of your solution.
 */
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <unistd.h>
#include <string.h>

#include "mm.h"
#include "memlib.h"

/*********************************************************
 * NOTE TO STUDENTS: Before you do anything else, please
 * provide your team information in the following struct.
 ********************************************************/
team_t team = {
    /* Team name */
    "ateam",
    /* First member's full name */
    "Harry Bovik",
    /* First member's email address */
    "bovik@cs.cmu.edu",
    /* Second member's full name (leave blank if none) */
    "",
    /* Second member's email address (leave blank if none) */
    ""
};

//store the free chunk
void *free_list[LISTMAX];

/* single word (4) or double word (8) alignment */
#define ALIGNMENT 8

/* rounds up to the nearest multiple of ALIGNMENT */
#define ALIGN(size) (((size) + (ALIGNMENT-1)) & ~0x7)


#define SIZE_T_SIZE (ALIGN(sizeof(size_t)))

//extend the heap when the free space is not enough
static void *extend_heap(size_t size);
//coalesce if the next or prev chunk is free
static void *coalesce(void *ptr);
//when the chunk size is larger than demanded,how to get the demanded size and the remainer how to place
static void *place(void *ptr, size_t size);
//insert the chunk node into the free list
static void insert_node(void *ptr, size_t size);
//remove the chunk node from the free list
static void delete_node(void *ptr);


static void *extend_heap(size_t size) {

    void *ptr;
    //align the size
    size = ALIGN(size);
    if((long)(ptr = mem_sbrk(size)) == -1) {
        return NULL;
    }

    //set the chunk's header and footer
    PUT(HDRP(ptr), PACK(size, 0));
    PUT(FTRP(ptr), PACK(size, 0));
    //set the epilogue header
    PUT(HDRP(NEXT_BLKP(ptr)), PACK(0, 1));

    //insert the new chunk into the free list
    insert_node(ptr, size);

    return coalesce(ptr);
}

static void insert_node(void *ptr, size_t size) {
    int index = 0;
    void *prev = NULL;
    void *next = NULL;

    //find the index of the free list
    while(size > 1 && index < LISTMAX - 1){
        size >>= 1;
        ++index;
    }

    //get the free_list[index]
    next = free_list[index];
    while((next != NULL) && (size > GET_SIZE(HDRP(next)))) {
        prev = next;
        next = PREV_BLKP(next);
    }

    if(next != NULL) {
        //insert into the middle of free list
        if(prev != NULL) {
            *(unsigned int *)PRED(ptr) = (unsigned int)next;
            *(unsigned int *)SUCC(ptr) = (unsigned int)prev;
            *(unsigned int *)PRED(prev) = (unsigned int)ptr;
            *(unsigned int *)SUCC(next) = (unsigned int)ptr;
        }
            //insert into the begin of free list
        else {
            SET_PTR(PRED(ptr), next);
            SET_PTR(SUCC(next), ptr);
            SET_PTR(SUCC(ptr), NULL);
            free_list[index] = ptr;
        }
    }
    else {
        //insert into the end of free list
        if(prev != NULL) {
            SET_PTR(PRED(prev), ptr);
            SET_PTR(SUCC(ptr), prev);
            SET_PTR(PRED(ptr), NULL);
        }
            //the free list is empty
        else {
            SET_PTR(PRED(ptr), NULL);
            SET_PTR(SUCC(ptr), NULL);
            free_list[index] = ptr;
        }
    }
}

static void delete_node(void *ptr) {
    int index = 0;
    size_t size = GET_SIZE(HDRP(ptr));

    while(size > 1 && index < LISTMAX - 1) {
        size >>= 1;
        ++index;
    }

    void *prev = SUCC_BLKP(ptr);
    void *next = PRED_BLKP(ptr);
    if(prev != NULL) {
        //remove from the middle of the free list
        if(next != NULL) {
            SET_PTR(PRED(prev), next);
            SET_PTR(SUCC(next), prev);
        }
            //remove from the
        else {
            SET_PTR(PRED(prev), NULL);
        }
    }
    else {
        //remove from the begin of the free list
        if(next != NULL) {
            SET_PTR(SUCC(next), NULL);
            free_list[index] = next;
        }
            //the free list only has one chunk
        else {
            free_list[index] = NULL;
        }
    }
}

static void *coalesce(void *ptr) {
    _Bool is_prev_alloc=  GET_ALLOC(HDRP(PREV_BLKP(ptr)));
    _Bool is_next_alloc = GET_ALLOC(HDRP(NEXT_BLKP(ptr)));

    size_t size = GET_SIZE(HDRP(ptr));

    //if the prev and next chunk is allocated, cant coalesce
    if(is_prev_alloc && is_next_alloc) {
        return ptr;
    }
        //if the prev chunk is allocated, the next chunk is free, coalesce the next chunk
    else if(is_prev_alloc && !is_next_alloc) {
        delete_node(ptr);
        delete_node(NEXT_BLKP(ptr));

        size += GET_SIZE(HDRP(NEZT_BLKP(ptr)));
        PUT(HDRP(ptr), PACK(size, 0));
        PUT(FTRP(ptr), PACK(size, 0));
    }
        //if the prev chunk is free, the next chunk is allocated, coalesce the prev chunk
    else if(!is_prev_alloc && is_next_alloc) {
        delete_node(ptr);
        delete_node(PREV_BLKP(ptr));

        size += GET_SIZE(HDRP(PREV_BLKP(ptr)));

        PUT(FTRP(ptr), PACK(size, 0));
        PUT(HDRP(PREV_BLKP(ptr)), PACK(size, 0));

        ptr = PREV_BLKP(ptr);
    }
        //the prev and next chunk is free, coalesce the next and prev chunk
    else {
        delete_node(ptr);
        delete_node(PREV_BLKP(ptr));
        delete_node(NEXT_BLKP(ptr));

        size += GET_SIZE(HDRP(NEXT_BLKP(ptr))) + GET_SIZE(HDRP(PREV_BLKP(ptr)));

        PUT(FTRP(NEXT_BLKP(ptr)), PACK(size, 0));
        PUT(HDRP(PREV_BLKP(ptr)), PACK(size, 0));

        ptr = PREV_BLKP(ptr);
    }

    //insert the new node into the free list
    insert_node(ptr, size);

    return ptr;
}

static void *place(void *ptr, size_t size) {
    size_t chunk_size = GET_SIZE(HDRP(ptr));
    size_t remainer = chunk_size - size;

    delete_node(ptr);

    //if the remainer size is smaller than the mini size, don't place
    if(remainer <= 2 * DSIZE) {
        PUT(HDRP(ptr), PACK(chunk_size, 1));
        PUT(FTRP(ptr), PACK(chunk_size, 1));
    }
        //
    else {
        PUT(HDRP(ptr), PACK(size, 1));
        PUT(FTRP(ptr), PACK(size, 1));
        PUT(HDRP(NEXT_BLKP(ptr)), PACK(remainer, 1));
        PUT(FTRP(NEXT_BLKP(ptr)), PACK(remainer, 1));
        insert_node(NEXT_BLKP(ptr), remainer);
    }
    return ptr;
}
/*
 * mm_init - initialize the malloc package.
 */
int mm_init(void)
{
    int listnumber;
    char *heap;

    //init the free list
    for(listnumber = 0; listnumber < LISTMAX; ++listnumber) {
        free_list[listnumber] = NULL;
    }

    //init the heap
    if((heap = mem_sbrk(4 * WSIZE)) == (void *)-1) {
        return -1;
    }

    //pro
    PUT(heap, 0);
    PUT(heap + WSIZE, PACK(DSIZE, 1));  //prologue header
    PUT(heap + WSIZE * 2, PACK(DSIZE,1)); //prologue footer
    PUT(heap + WSIZE * 3, PACK(0, 1));  //epilogue header

    //extend the heap, alloc a big memory
    if(extend_heap(CHUNKSIZE / WSIZE) == NULL) {
        return -1;
    }

    return 0;
}

/*
 * mm_malloc - Allocate a block by incrementing the brk pointer.
 *     Always allocate a block whose size is a multiple of the alignment.
 */
void *mm_malloc(size_t size)
{
    if(size == 0)
        return NULL;
    if(size < 2 * DSIZE) {
        size = 2 * DSIZE;
    }
    else {
        size = ALIGN(size + DSIZE);
    }

    int index = 0;
    size_t tmp_size = size;
    void *ptr = NULL;

    while(index < LISTMAX) {
        //find the proper free list
        if((tmp_size <= 1) && (free_list[index] != NULL)) {
            ptr = free_list[index];
            //find the proper chunk
            while((ptr != NULL) && (size > GET_SIZE(HDRP(ptr)))) {
                ptr = *PRED(ptr);
            }
            //find the chunk
            if(ptr != NULL) {
                break;
            }
        }

        tmp_size >>= 1;
        ++index;
    }

    //if there is no chunk can use, extend the heap memory
    if(ptr == NULL) {
        if((ptr = extend_heap(MAX(size, CHUNKSIZE))) == NULL) {
            return NULL;
        }
    }

    //place the chunk
    place(ptr, size);

    return ptr;
}

/*
 * mm_free - Freeing a block does nothing.
 */
void mm_free(void *ptr)
{
    size_t size = GET_SIZE(HDRP(ptr));

    PUT(HDRP(ptr), PACK(size, 0));
    PUT(FTRP(ptr), PACK(size, 0));

    insert_node(ptr, size);
    coalesce(ptr);
}

/*
 * mm_realloc - Implemented simply in terms of mm_malloc and mm_free
 */
void *mm_realloc(void *ptr, size_t size)
{
    void *new_ptr = ptr;
    size_t remainer;

    if(size == 0) {
        return NULL;
    }

    //calculate the align size
    if(size <= DSIZE){
        size = 2 * DSIZE;
    }
    else{
        size = ALIGN(size + DSIZE);
    }

    //if the new size <= chunk size, return the chunk
    if((remainer = GET_SIZE(HDRP(ptr)) - size) >= 0) {
        return ptr;
    }
        //check the prev and next of this chunk is free, if free coalesce the chunk
    else {
        new_ptr = mm_malloc(size);
        memcpy(new_ptr, ptr, GET_SIZE(HDRP(ptr)));
        mm_free(ptr);
    }

    return new_ptr;
}














