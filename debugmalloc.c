#include <stdlib.h>
#include <string.h>
#include "debugmalloc.h"
#include "dmhelper.h"
#include <stdio.h>





/* Wrappers for malloc and free */
struct header {
	int checksum;
	size_t size;
	char* filename;
	int linenumber;
	struct header* next;
};

struct header* head_block = NULL;
struct header* tail_block = NULL;

#define FENCE_VALUE 0xC9E1456A
#define BASIC_SIZE_HEADER sizeof(struct header) / 4
#define BASIC_SIZE_FENCE sizeof(FENCE_VALUE) / 4
#define BASIC_SIZE_META ((BASIC_SIZE_FENCE + BASIC_SIZE_HEADER) * 2)

void initLinkedList() {
	if (head_block) {
		return;
	}
	head_block = malloc(sizeof(struct header));
	head_block-> next = NULL;
	tail_block = head_block;
}

int computeChecksum(struct header* ptr) {
	return ptr->size ^ ptr->linenumber; // Changed checksum calculation
}

int* getHeaderFenceOfChunk(struct header* ptr) {
	return (int*)((char*)ptr + BASIC_SIZE_HEADER);
}

int* getFooterFenceOfChunk(struct header* ptr) {
	return (int*)((char*)ptr + BASIC_SIZE_HEADER + BASIC_SIZE_FENCE + ptr->size);
}

void* getPayloadAddress(struct header* ptr) {
	return (void*)((char*)ptr + BASIC_SIZE_HEADER + BASIC_SIZE_FENCE);
}

void* MyMalloc(size_t size, char* filename, int linenumber) {
	initLinkedList();
	struct header* new = malloc(BASIC_SIZE_META + size);
	new-> size = size;
	new-> filename = filename;
	new-> linenumber = linenumber;
	new-> checksum = computeChecksum(new);
	tail_block-> next = new;
	tail_block = new;
	tail_block-> next = NULL;
	*getHeaderFenceOfChunk(new) = FENCE_VALUE;
	*getFooterFenceOfChunk(new) = FENCE_VALUE;
	return getPayloadAddress(new);
}


int getErrorCode(struct header* ptr) {
	if (ptr == NULL) {
		return 0;
	}
	else {
		int flag;

		flag = (*getHeaderFenceOfChunk(ptr) == FENCE_VALUE);
		if (!flag) {
			return 1;
		}
		flag = (*getFooterFenceOfChunk(ptr) == FENCE_VALUE);
		if (!flag) {
			return 2;
		}
		flag = (ptr->checksum == computeChecksum(ptr));
		if (!flag) {
			return 3;
		}
	}
	return 0;
}

void MyFree(void* ptr, char* filename, int linenumber) {
	if (!head_block) {
		error(4, filename, linenumber);
		return;
	}

	struct header* preFree = head_block;
	struct header* toFree = preFree->next;

	while (toFree) {
		if (getPayloadAddress(toFree) == ptr) {
			int errorCode = getErrorCode(toFree);
			if (errorCode) {
				errorfl(errorCode, toFree->filename, toFree->linenumber, filename, linenumber);
				return;
			}
			else {
				break;
			}
		}
		else {
			preFree = toFree;
			toFree = preFree->next;
		}
	}

	if (!toFree) {
		error(4, filename, linenumber);
		return;
	}

	preFree->next = toFree->next;
	free(toFree);
}


/* returns number of bytes allocated using MyMalloc/MyFree:
	used as a debugging tool to test for memory leaks */
int AllocatedSize() {
	if (!head_block) {
		return 0;
	}

	int sum = 0;
	struct header* temp = head_block->next;
	while (temp) {
		sum += temp->size;
		temp = temp->next;
	}
	return sum;
}

void PrintAllocatedBlocks() {
	if (!head_block) {
		return;
	}

	struct header* temp = head_block->next;
	int sum = 0;
	while (temp) {
		printf("Allocated block %d: %zu bytes, at line %d of the file %s.\n", ++sum, temp->size, temp->linenumber, temp->filename);
		temp = temp->next;
	}
}
  int HeapCheck() {
	int status = 0;

	if (!head_block) {
		return status; // Linked list doesn't exist
	}

	struct header* temp = head_block->next;

	while (temp) {
		int errorCode = getErrorCode(temp);

		if (errorCode) {
			status = -1;
			char* msg = getMsg(errorCode); // Assuming getMsg returns an error message

			printf("Error: %s\n\tin block allocated at %s, line %d\n", msg, temp->filename, temp->linenumber);
		}

		temp = temp->next;
	}

	return status;
}
	

/* Optional functions */

/* Prints a list of all allocated blocks with the
	filename/line number when they were MALLOC'd */
/*void PrintAllocatedBlocks() {
	return;
}
*/
/* Goes through the currently allocated blocks and checks
	to see if they are all valid.
	Returns -1 if it receives an error, 0 if all blocks are
	okay.
*/




