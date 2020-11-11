#include "alloc.h"
#define DEBUG 1
char * page;
struct page_manage{
	int toal_size;
	int left_used;
	int right_used;
	int full_used;
	int start;
	struct page_manage *left;
	struct page_manage *right;
	struct page_manage *parent;
};
struct page_manage *head;

int init_alloc()
{
	head = (struct page_manage*) malloc(sizeof(struct page_manage));
	head->total_size = 4096;
	head->left_used = 0;
	head->right_used = 0;
	head->full_used = 0;
	head->start = 0;
	head->left = NULL;
	head->right = NULL;
	head->parent = NULL;
	if((page = (char*) mmap(NULL,0x1000, PROT_READ | PROT_WRITE | PROT_EXECUTE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0)) == -1){
		return -1;
	else
		return 0;
}

void delete_link(struct page_manage *cur)
{
	if(cur->left != NULL)
		delete_link(cur->left);
	if(cur->right != NULL)
		delete_link(cur->right);
	free(cur);
}

int cleanup()
{
	delete_link(head);
	if(munmap(page,0x1000) == -1)
		return -1;
	else
		return 0;
}
struct page_manage* create_link()
{
	struct page_manage *new_node;
	new_node = (struct page_manage*) malloc(sizeof(struct page_manage));
	new_node->total_size = 0;
	new_node->left_used = 0;
	new_node->right_used = 0;
	new_node->full_used = 0;
	new_node->start = 0;
	new_node->left = NULL;
	new_node->right = NULL;
	new_node->parent = NULL;
	
	return new_node;
}
int choose_link(struct page_manage *cur, int size)
{
	int usable_size = cur->total_size - (cur->left_size + cur->right_size);
	int res;
	if(size > usable_size && size % 8 != 0)
		return -1;
	else{

		//512바이트가 들어있는 상태에서 2048 + 1024바이트의 메모리 요청이 들어올때?


		//자식노드보다 작을때
		if(size <= cur->total_size/2){
			//왼쪽 자식노드가 없을때
			if(cur->left == NULL){
				cur->left = create_link();
				cur->left->start = cur->start;
				cur->left->parent = cur;
				cur->left->total_size = cur->total_size/2;
				if((res = choose_link(cur->left, size))){
					cur->left_used += size;
					return res;
				}
			}
			//왼쪽 자식노드에 적당한 크기가 있을때
			if((cur->total_size/2 - left_used) >= size){
				if((res = choose_link(cur->left, size))){
					cur->left_used += size;
					return res;
				}
			}
			//오른쪽 자식노드가 없을때
			else if(cur->right == NULL){
				cur->right = create_link();
				cur->right->start = cur->start + cur->total_size/2;
				cur->right->parent = cur;
				cur->right->total_size = cur->total_size/2;
				if(choose_link(cur->right, size))
					cur->right_used += size;
			}
			//오른쪽 자식노드에 적당한 크기가 있을때
			else if((cur->total_size/2 - right_used) >= size){
				if(choose_link(cur->right, size))
					cur->right_used += size;
			}
		}





}
char *alloc(int size)
{
	int res;
	if((res = choose_link(head))){
#if DEBUG
	fprintf(stderr,"alloc failed\n");
	exit(1);
#endif
		return NULL;
	}
	else
		return page + res;
}
void dealloc(char *); 

