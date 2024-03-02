#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define NUM 128

struct line {
	int address;
	int valid;
	int modified;
};

struct node {
	int cacheIndex;
	struct node *next;
	struct node *prev;
};

// init
struct node list_of_nodes[NUM];
struct node headFreeList;
struct node headBusyList;
struct line cache[NUM];

void initCache() {
	for (int i = 0; i < NUM; i++) {
		cache[i].valid = 0;
		cache[i].modified = 0;
	}
}

void initFreeList() {
  struct node *myNode = &headFreeList;
  myNode->cacheIndex = -1;
  myNode->next = &(list_of_nodes[0]);  // RECALL: '&' means "address of"
  for (int i = 0; i < NUM-1; i++){
    myNode = &list_of_nodes[i];
    myNode->prev = NULL;
    myNode->cacheIndex = i;
    myNode->next = &(list_of_nodes[i+1]);
  }
  myNode = &list_of_nodes[127];
  myNode->prev = NULL;
  myNode->cacheIndex = NUM-1;
  myNode->next = NULL;
}

void initBusyList() {
	headBusyList.cacheIndex = -1;
	headBusyList.next = NULL;
	headBusyList.prev = NULL;
}

int readNum() {
	int num = 0;
	while (1) {
		char mychar;
		int rc = read(0, &mychar, 1);
			if (rc == 0) { // if end-of-file
				return -1; 
			}
		if (mychar == ' ' || mychar == '\n') { break; }
		num = 10 * num + (mychar - '0'); // ASCII 0, 1, 2, ... == '0', '1', '2'
	}
	// 'num' is the number read.	
	return num;
}

int calculateTagIndexOffset(int num, int *tag, int *index, int *offset) {
	*offset = num % 4;
	int x = num / 4;
	*index = x % NUM;
	*tag = x / NUM;
}

struct node *getBusyListTail() {
	struct node *current = &headBusyList;
    while (current->next != NULL) {
        current = current->next;
    }
	return current;
}

void insertNodeToBusyList(struct node *insNode) {
	struct node *tail = getBusyListTail();
	tail->next = insNode;
	insNode->next = NULL;
}

void insertFreeNodeToBusyList() {
	struct node *temp = headFreeList.next;
	headFreeList.next = headFreeList.next->next;
	struct node *tail = getBusyListTail();
	tail->next = temp;
	temp->next = NULL;
}

struct node *removeLeastRecentlyUsed() {
	struct node *temp = headBusyList.next;
	headBusyList.next = temp->next;
	return temp;
}

// return 1: HIT, return 0: MISS
int updateFullyAssociative(int num, int tag) {
	struct node *prev = NULL;
	struct node *current = headBusyList.next;
	while (current != NULL) {
		// look for x in busy list
		// if found = HIT
		// 		remove struct from busy list
		// 		insert onto tail of busy list
		int addr = cache[current->cacheIndex].address;
		if (addr == tag) {
			struct node *temp = current;
			current = current->next;
			prev->next = temp->next;
			insertNodeToBusyList(temp);
			return 1; // HIT
		}
		prev = current;
		current = current->next;
	}
	// if not found = MISS
	if (headFreeList.next == NULL) {
		// No more room in free list -> MUST EVICT	
		//      remove LRU from busy list
		//      insert that node onto busy list tail
		struct node *nodeLRU = removeLeastRecentlyUsed();
		insertNodeToBusyList(nodeLRU);
		cache[nodeLRU->cacheIndex].address = tag;
	} else {
		// take headFreeList and use it for new data
		// insert that node onto busy list tail
		cache[headFreeList.next->cacheIndex].address = tag;
		cache[headFreeList.next->cacheIndex].valid = 1;
		insertFreeNodeToBusyList();
	}
	return 0; // MISS
}

void displayFullyAssociative() {
	printf("Cache contents (for each cache row index):\n");
	for (int i = 0; i < NUM; i++) {
		int tag = cache[i].address;
		printf("%4d: Valid: %-5s;  Tag: %d  (Set #:  0)\n", i, cache[i].valid ?"True" :"False", tag); 
	}
}


void simulateFullyAssociative() {
	initFreeList();
	initBusyList();
	int number = readNum();	
	while (number != -1) {
		int tag = number / 4;
		int hit = updateFullyAssociative(number, tag);
		printf("%4d: %-4s (Tag/-/Offset: %d/0/0)\n", number, hit ?"HIT" :"MISS", tag);
		number = readNum();
	}
	displayFullyAssociative();
}

int updateDirectMapped(int num, int index) {
	if (cache[index].valid && cache[index].address == num) {
		return 1; // HIT
	} else {
		// MISS
		cache[index].address = num;
		cache[index].valid = 1;
		return 0;
	}
}

void displayDirectMapped() {
    printf("Cache contents (for each cache row index):\n");
    for (int i = 0; i < NUM; i++) {
        char *tagString;
        int tag, index, offset;
        calculateTagIndexOffset(cache[i].address, &tag, &index, &offset);
        printf("%4d: Valid: %-5s;  Tag:  ", i, cache[i].valid ?"True" :"False");
        if (cache[i].valid == 0) {
            printf("-");
        } else {
            printf("%d", tag);
        }
        printf("  (Index: %d)\n", i);
    }
}

void simulateDirectMapped() {
	int number = readNum();
	while (number != -1) {
		int tag, index, offset;
		calculateTagIndexOffset(number, &tag, &index, &offset);
		int hit = updateDirectMapped(number, index);
		printf("%4d: %-4s (Tag/Index/Offset: %d/%d/%d)\n", number, hit ?"HIT" :"MISS", tag, index, offset);
		number = readNum();
	}
	displayDirectMapped();
}

int main(int argc, char *argv[]) {
	if (argc != 2) {
		fprintf(stderr, "Usage: %s -a|-d\n", argv[0]);
		exit(EXIT_FAILURE);
	}		
	initCache();	
	if (argv[1][1] == 'a') {
		simulateFullyAssociative();
	} else if (argv[1][1] == 'd') {
		simulateDirectMapped();
	}
}
