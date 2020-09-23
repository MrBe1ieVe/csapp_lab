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
static void *extend_heap(size_t size);


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
/* 
 * mm_init - initialize the malloc package.
 */
int mm_init(void)
{
    return 0;
}

/* 
 * mm_malloc - Allocate a block by incrementing the brk pointer.
 *     Always allocate a block whose size is a multiple of the alignment.
 */
void *mm_malloc(size_t size)
{
    int newsize = ALIGN(size + SIZE_T_SIZE);
    void *p = mem_sbrk(newsize);
    if (p == (void *)-1)
	return NULL;
    else {
        *(size_t *)p = size;
        return (void *)((char *)p + SIZE_T_SIZE);
    }
}

/*
 * mm_free - Freeing a block does nothing.
 */
void mm_free(void *ptr)
{
}

/*
 * mm_realloc - Implemented simply in terms of mm_malloc and mm_free
 */
void *mm_realloc(void *ptr, size_t size)
{
    void *oldptr = ptr;
    void *newptr;
    size_t copySize;
    
    newptr = mm_malloc(size);
    if (newptr == NULL)
      return NULL;
    copySize = *(size_t *)((char *)oldptr - SIZE_T_SIZE);
    if (size < copySize)
      copySize = size;
    memcpy(newptr, oldptr, copySize);
    mm_free(oldptr);
    return newptr;
}














