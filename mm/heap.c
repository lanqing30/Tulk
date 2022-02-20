#include "debug.h"
#include "pmm.h"
#include "vmm.h"
#include "heap.h"

// 申请内存块
static void alloc_chunk(uint32_t start, uint32_t len);

// 释放内存块
static void free_chunk(header_t *chunk);

// 切分内存块
static void split_chunk(header_t *chunk, uint32_t len);

// 合并内存块
static void glue_chunk(header_t *chunk);

static uint32_t heap_max = HEAP_START;

// 内存块管理头指针
static header_t *heap_first;

void init_heap()
{
	heap_first = 0;
}

void *kmalloc(uint32_t len)
{

	len += sizeof(header_t);

	header_t *cur_header = heap_first;
	header_t *prev_header = 0;

	while (cur_header) {

		if (cur_header->allocated == 0 && cur_header->length >= len) {

			split_chunk(cur_header, len);
			cur_header->allocated = 1;

			return (void *)((uint32_t)cur_header + sizeof(header_t));
		}

		prev_header = cur_header;
		cur_header = cur_header->next;
	}

	uint32_t chunk_start;


	if (prev_header) {
		chunk_start = (uint32_t)prev_header + prev_header->length;
	} else {
		chunk_start = HEAP_START;
		heap_first = (header_t *)chunk_start;
	}


	alloc_chunk(chunk_start, len);
	cur_header = (header_t *)chunk_start;
	cur_header->prev = prev_header;
	cur_header->next = 0;
	cur_header->allocated = 1;
	cur_header->length = len;

	if (prev_header) {
		prev_header->next = cur_header;
	}

	return (void*)(chunk_start + sizeof(header_t));
}

void kfree(void *p)
{

	header_t *header = (header_t*)((uint32_t)p - sizeof(header_t));
	header->allocated = 0;


	glue_chunk(header);
}

void alloc_chunk(uint32_t start, uint32_t len)
{

	while (start + len > heap_max) {
		struct PageInfo* page = pmm_alloc_page(1);
		map_region(pgd_kern, heap_max, PGSIZE, page->physical_addr, PTE_P | PTE_W);
		heap_max += PGSIZE;
	}
}

void free_chunk(header_t *chunk)
{
	if (chunk->prev == 0) {
		heap_first = 0;
	} else {
		chunk->prev->next = 0;
	}


	while ((heap_max - PGSIZE) >= (uint32_t)chunk) {
		heap_max -= PGSIZE;
		//get_mapping(pgd_kern, heap_max, &page);
		//unmap(pgd_kern, heap_max);
		//pmm_free_page(page);
		page_remove(pgd_kern, (void*)heap_max);
	}
}

void split_chunk(header_t *chunk, uint32_t len)
{

	if (chunk->length - len > sizeof (header_t)) {
		header_t *newchunk = (header_t *)((uint32_t)chunk + len);
		newchunk->prev = chunk;
		newchunk->next = chunk->next;
		newchunk->allocated = 0;
		newchunk->length = chunk->length - len;

		chunk->next = newchunk;
		chunk->length = len;
	}
}

void glue_chunk(header_t *chunk)
{

	if (chunk->next && chunk->next->allocated == 0) {
		chunk->length = chunk->length + chunk->next->length;
		if (chunk->next->next) {
			chunk->next->next->prev = chunk;
		}
		chunk->next = chunk->next->next;
	}


	if (chunk->prev && chunk->prev->allocated == 0) {
		chunk->prev->length = chunk->prev->length + chunk->length;
		chunk->prev->next = chunk->next;
		if (chunk->next) {
			chunk->next->prev = chunk->prev;
		}
		chunk = chunk->prev;
	}


	if (chunk->next == 0) {
		free_chunk(chunk);
	}
}



