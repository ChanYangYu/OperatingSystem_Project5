#include "alloc.h"
#define DEBUG 1
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
	used_size = 0;
	if((page = (char*) mmap(NULL,0x1000, PROT_READ | PROT_WRITE | PROT_EXECUTE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0)) == -1){
		return -1;
	else
		return 0;
}

int cleanup()
{
	struct page_manage *cur, *tmp;

	cur = head->next;
	free(head);
	while(cur != NULL){
		tmp = cur;
		cur = cur->next;
		free(tmp);
	}
	if(munmap(page,0x1000) == -1)
		return -1;
	else
		return 0;
}
int choose_pos(int size)
{
	struct page_manage *cur, *new_node;
	int pos = 0;
	int usable_size;

	if(size > PAGE_SIZE - used_size || size % MINALLOC != 0)
		return -1;
	cur = head;
	if(cur == NULL)
	{
		new_node = (struct page_manage*) malloc(sizeof(struct page_manage));
		new_node->start = pos;
		new_node->size = size;
		new_node->next = NULL;
		new_node->prev = NULL;
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
					if(cur == head){
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
			if(cur->next == NULL && pos + size <= PAGE_SIZE){
				new_node = (struct page_manage*) malloc(sizeof(struct page_manage));
				new_node->start = pos;
				new_node->size = size;
				new_node->next = NULL;
				new_node->prev = cur;
				cur->next = new_node;
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
	if((res = choose_pos()) == -1){
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
	struct manage_page *cur, *tmp;

	cur = head;
	while(cur != NULL)
	{
		if(cur->start == pos){
			if(cur == head){
				head = cur->next;
				if(head != NULL)
					head->prev = NULL;
			}
			else{
			}
		}
	}
	
}
void dealloc(char *start)
{
	int pos;

	pos = start - page;
	delete_pos(pos);
}

