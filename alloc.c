#include "alloc.h"
#define DEBUG 0

char *page;
int used_size;

struct page_manage{
	int start;
	int size;
	struct page_manage *next;
	struct page_manage *prev;
};
struct page_manage *head, *tail;

int init_alloc()
{
	head = NULL;
	tail = NULL;
	used_size = 0;
	if((page = (char*) mmap(NULL,0x1000, PROT_READ | PROT_WRITE | PROT_EXEC, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0)) == (void*)-1)
		return -1;
	else
		return 0;
}

int cleanup()
{
	struct page_manage *cur, *tmp;

	cur = head->next;
	free(head);
	head = NULL;
	while(cur != NULL){
		tmp = cur;
		cur = cur->next;
		free(tmp);
	}
	tail = NULL;
	if(munmap(page,0x1000) == -1)
		return -1;
	else
		return 0;
}

//Debug Function
void print()
{
	struct page_manage *cur = head;

	while(cur != NULL){
		printf("start : %d size : %d\n", cur->start, cur->size);
		cur = cur->next;
	}
	printf("==================\n");
}

int choose_pos(int size)
{
	struct page_manage *cur, *new_node;
	int pos = 0;
	int usable_size;

#if DEBUG
	print();
#endif
	if(size > PAGESIZE - used_size || size % MINALLOC != 0)
		return -1;
	cur = head;
	if(head == NULL && tail == NULL)
	{
		new_node = (struct page_manage*) malloc(sizeof(struct page_manage));
		new_node->start = pos;
		new_node->size = size;
		new_node->next = NULL;
		new_node->prev = NULL;
		head = new_node;
		tail = new_node;
		used_size += size;
		return pos;
	}
	else{
		while(cur != NULL){
			if(cur->start == pos)
				pos += cur->size;
			//사이의 빈공간이 있는경우
			else if(cur->start > pos){
				usable_size = cur->start - pos;
				if(size <= usable_size){
					new_node = (struct page_manage*) malloc(sizeof(struct page_manage));
					new_node->start = pos;
					new_node->size = size;
					new_node->next = cur;
					//head일 경우
					if(cur->prev == NULL){
						head = new_node;
						new_node->prev = NULL;
						cur->prev = new_node;
					}
					else{
						new_node->prev = cur->prev;
						cur->prev->next = new_node;
						cur->prev = new_node;
					}
					used_size += size;
					return pos;
				}
				//현재 chunk의 끝으로 pos이동
				else
					pos = cur->start + cur->size; 
			}
			//마지막 노드이고 공간이 있으면
			if(cur->next == NULL && pos + size <= PAGESIZE){
				new_node = (struct page_manage*) malloc(sizeof(struct page_manage));
				new_node->start = pos;
				new_node->size = size;
				new_node->next = NULL;
				new_node->prev = cur;
				cur->next = new_node;
				tail = new_node;
				used_size += size;
				return pos;
			}
			cur = cur->next;
		}
		return -1;
	}
}
char *alloc(int size)
{
	int res;
	if((res = choose_pos(size)) == -1){
#if DEBUG
	fprintf(stderr,"alloc failed\n");
	exit(1);
#endif
		return NULL;
	}
	else
		return page + res;
}

int delete_pos(int pos)
{
	struct page_manage *cur;

	cur = head;
	while(cur != NULL)
	{
		if(cur->start == pos){
			if(cur == head && cur == tail){
				used_size -= cur->size;
				free(cur);
				head = NULL;
				tail = NULL;
				return 0;
			}
			else if(cur == head){
				used_size -= cur->size;
				head = cur->next;
				head->prev = NULL;
				free(cur);
				return 0;
			}
			else if(cur == tail){
				used_size -= cur->size;
				tail = cur->prev;
				tail->next = NULL;
				free(cur);
				return 0;
			}
			else{
				used_size -= cur->size;
				cur->prev->next = cur->next;
				cur->next->prev = cur->prev;
				free(cur);
				return 0;
			}
		}
		else
			cur = cur->next;
	}
	return -1;
	
}
void dealloc(char *start)
{
	int pos;

	pos = start - page;
	if(delete_pos(pos) == -1){
#if DEBUG
	fprintf(stderr,"dealloc Fail\n");
	exit(1);
#endif
	}
}

