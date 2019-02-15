#define _GNU_SOURCE 		/* mremap */
#include <sys/mman.h>
#include <linux/mman.h>		/* MAP_UNINITIALIZED */
#include <stdbool.h>
#include "alloc.h"

#define _dtor0_ 	__attribute__((destructor (110)))

#ifndef NDEBUG
	#include <stdio.h>
	#include <string.h>
	#include <errno.h>
	#define LOG(...) 	printf(__VA_ARGS__)
	#define WARN(...) 	fprintf(stderr, ##__VA_ARGS__)
#else
	#define LOG(...)
	#define WARN(...)
#endif

typedef struct my_header {
	size_t size;
	bool used;
	struct my_header *next;
} header_t;

static _dtor0_ void destroy(void);
static inline void *get_data(void *mem);
static inline header_t *get_header(void *mem);
static inline size_t get_real_size(int size);
#ifdef SAFE_ALLOC
	static inline void enable_prot(header_t *h, size_t size, int flags);
#else
	#define enable_prot(...)
#endif
static inline void *alloc(const size_t size, int flags);

static header_t *head = NULL; 	// our list of memory chunks

/** Private Interface **/

static _dtor0_ void destroy(void) {
	while (head) {
		header_t *tmp = head;
		head = head->next;
		/* Only unmap freed blocks */
		if (!tmp->used) {
			if (munmap(tmp, tmp->size) != 0) {
				WARN("%s\n", strerror(errno));
			}
		}
	} 
}

static inline void *get_data(void *mem) {
	return mem + sizeof(header_t);
}

static inline header_t *get_header(void *mem) {
	return mem - sizeof(header_t);
}

static inline size_t get_real_size(int size) {
	return sizeof(header_t) + size;
}

#ifdef SAFE_ALLOC
static inline void enable_prot(header_t *h, size_t size, int flags) {
	if (h && !h->used) {
		if (mprotect(h, size == 0 ? h->size : size, flags) != 0) {
			WARN("%s\n", strerror(errno));
		}
	}
}
#endif

static inline void *alloc(const size_t size, int flags) {
	if (size - sizeof(header_t) == 0) {
		return NULL;
	}
	
	header_t **tmp = &head;
	header_t *prev = NULL;
	while (*tmp) {
		if (!(*tmp)->used && (*tmp)->size >= size) {
			LOG("Suitable unused memory found: %p with size: %lu\n", (*tmp), (*tmp)->size);
			/* Re-enable read and write on memory block */
			enable_prot(*tmp, size, PROT_READ | PROT_WRITE);
			/* 
			 * Avoid calling mprotect(PROT_READ) before returning
			 * as we haven't touched prev->next ptr
			 */
			prev = NULL;
			goto found;
		}
		prev = *tmp;
		tmp = &(*tmp)->next;
	}

	/* We are modifying previous node -> next pointer! */
	enable_prot(prev, 0, PROT_READ | PROT_WRITE);
	
	void *ret = NULL;
	*tmp = mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS | flags, -1, 0);
	if (*tmp != MAP_FAILED) {
		LOG("Malloc'd %p with size: %lu\n", *tmp, size);
		(*tmp)->size = size;
		(*tmp)->next = NULL;

found:
		(*tmp)->used = true;
		ret = get_data(*tmp);
	} else {
		WARN("%s\n", strerror(errno));
	}
	
	/* We have modified previous node -> next pointer. We can protect it again now. */
	enable_prot(prev, 0, PROT_READ);
	return ret;
}

/** Public Interface **/

void *malloc_f(size_t size) {
	/* Avoid zeroing mapping, faster */
	return alloc(get_real_size(size), MAP_UNINITIALIZED);
}

void *calloc_f(int num, size_t size) {
	/* Require zeroing mapping, safer */
	return alloc(get_real_size(num * size), 0);
}

void free_f(void *ptr) {
	if (ptr) {
		header_t *priv = get_header(ptr);
		priv->used = false;
		/* Make this block's data protected (read-only) */
		enable_prot(priv, 0, PROT_READ);
	}
}

/* 
 * Algorithm:
 * * Store current node in priv ptr
 * * Call mremap
 * * If it fails, return NULL
 * * If it succeeds, store newly allocated mapping in place of old node.
 * 
 * Note: as we're modifying prev->next pointer when storing newly allocated mapping,
 * we should enable PROT_WRITE perm on prev mapping if needed.
 * 
 * This may seem overcomplicated; remember we cannot access old mapping after it is remapped.
 */
void *realloc_f(void *ptr, size_t size) {
	/* Mimic malloc if NULL ptr as param */
	if (!ptr) {
		return malloc_f(size);
	}
	
	/* Mimic free if 0 size is requested */
	if (size == 0) {
		free_f(ptr);
		return NULL;
	}
	
	header_t *priv = get_header(ptr);
	const size_t new_size = get_real_size(size);
	
	/* Find current node and store pointer to prev node */
	header_t **tmp = &head;
	header_t *prev = NULL;
	while (*tmp != priv) {
		prev = *tmp;
		tmp = &(*tmp)->next;
	}
	
	LOG("Realloc'ng %p with size %lu.\n", priv, new_size);
	void *temp = mremap(priv, priv->size, new_size, MREMAP_MAYMOVE);
	if (temp != MAP_FAILED) {
		if (temp != priv) {
			/* We are modifying previous node -> next pointer! Make it writable */
			enable_prot(prev, 0, PROT_READ | PROT_WRITE);
			*tmp = temp;
			/* We have modified previous node -> next pointer. We can protect it again now. */
			enable_prot(prev, 0, PROT_READ);
		}
		(*tmp)->size = new_size;
		return get_data(*tmp); 
	}
	WARN("%s\n", strerror(errno));
	return NULL;
}
