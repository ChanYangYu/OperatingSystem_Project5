#include "ealloc.h"
#define MAXPAGE 4
#define CHUNKNUM PAGESIZE/MINALLOC

char* pages[MAXPAGE];
int check_pages[MAXPAGE][CHUNKNUM];
int used_size[MAXPAGE];
int pages_num;

void init_alloc(void)
{
	int i, j;

	pages_num = 0;
	for(i = 0; i < MAXPAGE; i++){
		for(j = 0; j < CHUNKNUM; j++)
			check_pages[i][j] = 0;
		used_size[i] = 0;
	}
}

int choose_pos(int page_num, int size)
{
    int pos = 0;
    int usable_size, chunk_size, idx, start, i;

    usable_size = PAGESIZE - used_size[page_num];
    if(size > usable_size || size % MINALLOC != 0)
        return -1; 
    chunk_size = size / MINALLOC;
    idx = 0;

    //search optimal space
    while(idx < CHUNKNUM){
        //is empty chunk
        if(check_pages[page_num][idx] == 0){ 
            start = idx;
            //count empty chunks
            while(idx < CHUNKNUM && (idx - start) < chunk_size){
                if(check_pages[page_num][idx] != 0)
                    break;
                idx++;
            }
            //check empty chunks
            if((idx - start) == chunk_size){
                for(i = 0; i < chunk_size; i++){
                    check_pages[page_num][start+i] = chunk_size;
				}
                used_size[page_num] += size;
                return start * MINALLOC;
            }
            else
                continue;
        }
        //jump
        else
            idx += check_pages[page_num][idx];
    }   
    return -1; 
}

char *alloc(int size)
{
	int res, i;

	if(size > PAGESIZE || size % 256 != 0)
		return NULL;
	for(i = 0; i < pages_num; i++){
		if((res = choose_pos(i, size)) != -1)
			return pages[i] + res;
	}
	//Does not exist sapce
	if(pages_num == MAXPAGE)
		return NULL;
	//create page
	pages[pages_num] = (char*) mmap(NULL,0x1000, PROT_READ | PROT_WRITE | PROT_EXEC, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
	res = choose_pos(pages_num, size);
	pages_num++;
	return pages[pages_num-1] + res;
}

int delete_pos(int page_num, int pos)
{
    int start = pos / MINALLOC;
    int size = check_pages[page_num][start];
    int i;

    if(pos + size > PAGESIZE)
        return -1;
	//clear data
    memset(pages[page_num] + pos, 0, size);

	//cleanup managing data
    for(i = 0; i < size; i++)
        check_pages[page_num][start + i] = 0;
    used_size[page_num] -= size * MINALLOC;
}

void dealloc(char *start)
{
	int i;

	for(i = 0; i < pages_num; i++){
		//search page_num
		if(start >= pages[i] && start < pages[i] + PAGESIZE){
			delete_pos(i, start - pages[i]);
			return;
		}
	}
}
void cleanup(void)
{
	int i, j;

	for(i = 0; i < pages_num; i++)
	{
		memset(pages[i], 0, PAGESIZE);
		for(j = 0; j < CHUNKNUM; j++)
			check_pages[i][j] = 0;
		used_size[i] = 0;
	}
}

