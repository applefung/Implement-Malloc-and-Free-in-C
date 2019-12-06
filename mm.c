#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
/**
 * Data structure of meta_data (32 bytes in 64-bit Linux OS)
 */
struct meta_data 
{
    size_t size;             // 8 bytes
    int free;                // 8 bytes 
    struct meta_data *next;  // 8 bytes
    struct meta_data *prev;  // 8 bytes
};

// calculate the meta data size and store as a constant
const size_t meta_data_size = sizeof(struct meta_data); 

/**
 * Global variables
 */
void *start_heap = NULL; // pointing to the start of the heap, initialize in main()
struct meta_data dummy_head_node; // dummy head node of a doubly linked list, initaiize in main()
struct meta_data *head = &dummy_head_node;

// This helper function should be used after sbrk()
// Given the pointer returned after calling sbrk(), it checks whether sbrk() is failed or not
// Reference: http://man7.org/linux/man-pages/man2/brk.2.html
//
// Usage:
//   void *p = sbrk(size);
//   if ( !sbrk_fail(p) ) {
//       // okay!
//   } else {
//      // failed
//   }
inline int sbrk_fail(void *ret) 
{
    return ret == sbrk(0);
}

// This helper function shift the sizeof(meta_data) bytes and cast the pointer to (void*)
//
// Usage:
//    struct meta_data *mb = // suppose you have a pointer pointing to a meta_data struct
//    void *data = meta_to_data(mb); // this helper function helps shift by sizeof(meta_data) bytes and cast to void*
//
inline void *meta_to_data(struct meta_data *mb) {
    return (void*) (mb+1);
}

// The implementation of the following functions are given:
struct meta_data *find_free_block(struct meta_data *head, size_t size);
void list_add(struct meta_data *new, struct meta_data *prev, struct meta_data *next);
void list_add_tail(struct meta_data *new, struct meta_data *head);
void init_list(struct meta_data *list);
void mm_print();

// Students are required to implement these functions below
void *mm_malloc(size_t size);
void mm_free(void *p);

int main()
{
    // initialization of start_heap and the linked list
    start_heap = sbrk(0);
    init_list(head);

    // step 1: allocate 1000 bytes
    void *p = mm_malloc(1000);

    printf("=== After step 1 ===\n");
    mm_print();

    // step 2: free 1000 bytes
    mm_free(p);

    printf("=== After step 2 ===\n");
    mm_print();

   // step 3: allocate 500 bytes, expected to split the free slot into 2
    void *q = mm_malloc(500);

    printf("=== After step 3 ===\n");
    mm_print();


    // step 4: allocate 462 bytes (not enough to split, occupy the whole free block = 468 bytes)
    void *r  = mm_malloc(462);
    printf("=== After step 4 ===\n");
    mm_print();

    // step 5: allocate 5000 bytes
    void *s = mm_malloc(5000);
    printf("=== After step 5 ===\n");
    mm_print();

    // step 6: free "r" pointer 
    mm_free(r);
    printf("=== After step 6 ===\n");
    mm_print();

    // step 7: free "q" pointer 
    mm_free(q);
    printf("=== After step 7 ===\n");
    mm_print();

    // step 8: free "s" pointer
    mm_free(s);
    printf("=== After step 8 ===\n");
    mm_print();

    return 0;
}


void init_list(struct meta_data *list) 
{
    list->next = list;
    list->prev = list;
}

void list_add(struct meta_data *new,
            struct meta_data *prev,
            struct meta_data *next)
{
    next->prev = new;
    new->next = next;
    new->prev = prev;
    prev->next = new;
}

void list_add_tail(struct meta_data *new,
                    struct meta_data *head)
{
    list_add(new, head->prev, head);
}

struct meta_data *find_free_block(struct meta_data *head, size_t size)
{
    struct meta_data *cur = head->next;
    while ( cur != head ) {
        if ( cur->free && cur->size >= size)
            return cur;
        cur = cur->next;
    }
    return NULL;
}

/** 
 * TODO: Clearly explain how you implement mm_malloc in point form. For example:
    Step 1: ....
    Step 2: ....
        Step 2.1: .....
        Step 2.2: .....
            Step 2.2.1: .....
    Step 3: ....
 */
/*function void *mm_malloc(size_t size)
 * Step 1: check whether the size is 0 or not.
 * 			step 1.1: if it is 0, return NULL. Otherwise, keep running the remaining code
 *
 * Step 2: variable definition
 * 			memory_location: the address of each block
 *
 * Step 3: find whether there is an enough block
 * 			step 3.1: if it can find it, it will return the address of that free block
 * 			step 3.2: if it cannot find, it will return NULL
 *
 * Step 4: variable definition
 * 			getTemp: to get a current free block
 * 			remainder: the size of the current free block
 * 			origin_size: the size of all the occupied blocks
 *
 * Step 5: if condition to whether there is only head in the block
 * 			step 5.1: if the condition is true, then do nothing
 * 			step 5.2: else the condition is false, remainder will be assigned the size of the current free block,
 * 				origin_size will be assigned the size of all the occupied blocks
 * 			step 5.3: additionally, getTemp will get a current free block
 *
 * Step 6: if condition to check whether it needs to open a new area for the user or not
 * 			step 6.1: it does not need to open a new area for the user
 * 				step 6.1.1: if condition to check whether it needs to split the block or not
 * 				step 6.1.2: if it needs to split (means it have enough space to split)
 * 						step 6.1.2.1: let memory_location points to start_heap
 * 						step 6.1.2.2: memory_location will be increased to the required size (the size of occupied size
 * 							and the required size because memory_location is pointing to start_heap)
 * 						step 6.1.2.3: set getTemp->free to 0 and make the block is occupied and not available to access
 * 						step 6.1.2.4: set getTemp->size to required size
 * 						step 6.1.2.5: variable definition
 * 										split_location: the location which needs to split
 * 						step 6.1.2.6: set getTemp->free to 1 and make the block is free and available to access
 * 						step 6.1.2.7: set getTemp->size to unused size (let the remainder - required size - the size of
 * 							struct meta_data)
 * 						step 6.1.2.8: add the new split block to the tail of the link list
 * 						step 6.1.2.9: return memory_location
 * 				step 6.1.3: if it does not need to split (means it does not have enough space to split)
 * 						step 6.1.3.1: let memory_location points to start_heap
 * 						step 6.1.3.2: memory_location will be increased to the size of the remaining block (because
 * 							memory_location is pointing to start_heap)
 * 						step 6.1.3.3: set getTemp->free to 0 and make the block is occupied and not available to access
 * 						step 6.1.3.4: set getTemp->size to the size of the remainding block
 * 						step 6.1.3.5: return memory_l
 * 		step 6.2: it needs to open a new area for the user
 * 				step 6.2.1: let memory_location points to start_heap
 * 				step 6.2.2: open a new area for the user and the size is the required size
 * 				step 6.2.3: let getTemp points to memory_location
 * 	 			step 6.2.4: set getTemp->free to 0 and make the block is occupied and not available to access
 * 				step 6.2.5: set getTemp->size to required size
 * 				step 6.2.6: variable definition
 * 								checkHasThing: check whether head is the only thing in the heap
 * 					step 6.2.6.1: if condition to check whether head is the only thing in the heao
 * 						step 6.2.6.1.1: if the condition is true
 * 							step 6.2.6.1.1.1: variable definition
 * 												tempHead: point to the next of head and then used as a counter
 * 												cumulated_size: sum of the size of occupied blocks
 * 							step 6.2.6.1.1.2: while loop to count the sum of the size of occupied blocks, if the
 * 								tempHead == head, then the loop will be broken
 * 								step 6.2.6.1.1.2.1: cumulated_size will be increased by continuously adding the size
 * 									of the occupied blocks
 * 								step 6.2.6.1.1.2.2: counter increased
 * 							step 6.2.6.1.1.3: memory_location = start_heap + cumulated_size + size
 * 				step 6.2.7: add the new block to the tail of the link list
 * 				step 6.2.8: return meomory_location
 * */

void *mm_malloc(size_t size)
{
    /* TODO: Implement mm_malloc */
	if(size==0)
	{
		return NULL;
	}
	void *memory_location = NULL;

	memory_location=find_free_block(head,size);
	struct meta_data* getTemp=head->next;
	size_t remainder=0;
	size_t origin_size=0;
	if(getTemp==head)
	{

	}
	else
	{
		while(getTemp!=head)
		{
			if(getTemp->free==1)
			{
				remainder=getTemp->size;
				break;
			}
			if(getTemp->free==0)
			{
				origin_size+=getTemp->size;
			}
			getTemp=getTemp->next;
		}
	}
	if(memory_location != NULL)
	{

		if(remainder-size>sizeof(struct meta_data))
		{
			memory_location=start_heap;

			memory_location+=origin_size+size;
			getTemp->free=0;
			getTemp->size=size;

			struct meta_data* split_location;
			split_location=memory_location;
			split_location->free=1;
			split_location->size=remainder-size-sizeof(struct meta_data);

			list_add(split_location,getTemp,head);
			return memory_location;
		}
		else
		{
			memory_location = start_heap;
			memory_location+=origin_size+remainder;
			getTemp->free = 0;
			getTemp->size = remainder;
			return memory_location;
		}
	}
	if(memory_location == NULL)
	{
		memory_location = start_heap;
		memory_location=sbrk(size);
		getTemp = memory_location;
		getTemp->free = 0;
		getTemp->size = size;
		struct meta_data* checkHasThing=head;
		if(checkHasThing->next!=head)
		{
			struct meta_data* tempHead=head->next;
			size_t cumulated_size=0;
			while(tempHead != head)
			{
				cumulated_size+=tempHead->size;
				tempHead=tempHead->next;
			}
			memory_location=start_heap+cumulated_size+size;
		}
		list_add_tail(getTemp,head);
		return memory_location;
	}
	return NULL;
}

/** 
 * TODO: Clearly explain how you implement mm_free in point form. For example:
    Step 1: ....
    Step 2: ....
        Step 2.1: .....
        Step 2.2: .....
            Step 2.2.1: .....
    Step 3: ....
 */
/*step 1: variable definition
 * 			find_location: find_location is a counter (let find_location points to the next of the block)
 * 			memory_find: let memory_find points to start_heap
 * 			required_size: assign 0 to required_size
 * step 2: use while loop to find the block which the user wants to free
 * 		step 2.1: memory_find will be increased continuously by adding the size of the occupied blocks
 * 		step 2.2: if condition to determine whether the block is found or not
 * 			step 2.2.1: if the condition is true, the loop will be broken
 * 		step 2.3: counter increased
 * step 3: if condition to determine whether the find_location is pointing to head or not
 * 		step 3.1: if the condition is true, then set find_location->free to 1 (set the found block to be freed)
 *
 */
void mm_free(void *p)
{
	struct meta_data* find_location=head->next;
	void* memory_find=start_heap;
	size_t required_size=0;
	while(p!=memory_find && find_location != head)
	{
		memory_find+=find_location->size;
		if(p==memory_find)
		{
			break;
		}

		find_location=find_location->next;
	}
	if(find_location != head)
	{
		find_location->free=1;
	}
}

void mm_print()
{
    printf("start_heap = %p\n", start_heap);
    printf(">>>\n");
    struct meta_data *cur = head->next;
    int i = 1;
    while ( cur != head ) {
        printf("Block %02d: [%s] size = %d bytes\n", i, cur->free ? "FREE" : "OCCP", cur->size );
        i = i+1;
        cur = cur->next;
    }
    printf(">>>\n");
    printf("brk = %p\n", sbrk(0));
}
