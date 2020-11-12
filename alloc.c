#include "alloc.h"
#define CHUNKNUM PAGESIZE/MINALLOC

char *page;
int used_size;
int check_page[CHUNKNUM];

int init_alloc()
{
	int i;

	used_size = 0;
	for(i = 0; i < CHUNKNUM; i++)
		check_page[i] = 0;
	if((page = (char*) mmap(NULL,0x1000, PROT_READ | PROT_WRITE | PROT_EXEC, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0)) == (void*)-1)
		return -1;
	else
		return 0;
}

int cleanup()
{
	int i;

	used_size = 0;
	for(i = 0; i < CHUNKNUM; i++)
		check_page[i] = 0;
	if(munmap(page,0x1000) == -1)
		return -1;
	else
		return 0;
}

int choose_pos(int size)
{
	int pos = 0;
	int usable_size, chunk_size, idx, start, i;

	usable_size = PAGESIZE - used_size;
	if(size > usable_size || size % MINALLOC != 0)
		return -1;
	chunk_size = size / MINALLOC;
	idx = 0;
	
	//search optimal space
	while(idx < CHUNKNUM){
		//is empty chunk
		if(check_page[idx] == 0){
			start = idx;
			//count empty chunks
			while(idx < CHUNKNUM && (idx - start) < chunk_size){
				if(check_page[idx] != 0)
					break;
				idx++;
			}
			//check empty chunks
			if((idx - start) == chunk_size){
				for(i = 0; i < chunk_size; i++)
					check_page[start+i] = chunk_size;
				used_size += size;
				return start * MINALLOC;
			}
			else
				continue;
		}
		//jump
		else
			idx += check_page[idx];
	}
	return -1;
}
char *alloc(int size)
{
	int res;
	if((res = choose_pos(size)) == -1)
		return NULL;
	else
		return page + res;
}

int delete_pos(int pos)
{
	int start = pos / MINALLOC;
	int size = check_page[start];
	int i;

	if(pos + size > PAGESIZE)
		return -1;
	memset(page + pos, 0, size);

	for(i = 0; i < size; i++)
		check_page[start + i] = 0;
	used_size -= size * MINALLOC;
}
void dealloc(char *start)
{
	int pos;

	pos = start - page;
	delete_pos(pos);
}
