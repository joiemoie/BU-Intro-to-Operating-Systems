#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <stdio.h>
#include <pthread.h>
#include <sys/mman.h>

typedef struct thread_local_storage
{
	pthread_t tid;
	unsigned int size; /* size in bytes */
	unsigned int page_num; /* number of pages */
	struct page **pages; /* array of pointers to pages */
} TLS;

struct page {
	unsigned int address; /* start address of page */
	int ref_count; /* counter for shared pages */
};

struct hash_element
{
	pthread_t tid;
	TLS *tls;
	struct hash_element *next;
};
struct hash_element* hash_table;

int initialized = 0;
int tls_create(unsigned int size);
int tls_write(unsigned int offset, unsigned int length, char * buffer);
int tls_read(unsigned int offset, unsigned int length, char * buffer);
int tls_destroy();
void tls_protect(struct page *p);
void tls_unprotect(struct page *p);
void tls_handle_page_fault(int sig, siginfo_t *si, void *context);
int page_size;

void tls_init()
{
	struct sigaction sigact;
	hash_table = (struct hash_element*)calloc(1, sizeof(struct hash_element));
	/* get the size of a page */
	page_size = getpagesize();
	/* install the signal handler for page faults (SIGSEGV, SIGBUS) */
	sigemptyset(&sigact.sa_mask);
	sigact.sa_flags = SA_SIGINFO; /* use extended signal handling */
	sigact.sa_sigaction = tls_handle_page_fault;
	sigaction(SIGBUS, &sigact, NULL);
	sigaction(SIGSEGV, &sigact, NULL);
	initialized = 1;
}

int tls_create(unsigned int size) {
	if (!initialized) {
		tls_init();
	}
	pthread_t tid = pthread_self();
	struct hash_element * current_hash = hash_table;
	while (current_hash->next != 0 && current_hash->next->tid != tid) {
		current_hash = current_hash->next;
	}
	if (current_hash->next == 0) {
		TLS* tls = (TLS *)calloc(1, sizeof(TLS));
		tls->tid = tid;
		tls->size = size;
		tls->page_num = (size%page_size == 0) ? size / page_size : size / page_size + 1;
		tls->pages = (struct page **)calloc(tls->page_num, sizeof(struct page*));
		for (int i = 0; i < tls->page_num; i++) {
			struct page *p = (struct page *)calloc(1, sizeof(struct page));
			p->address = (unsigned int)mmap(0, page_size, 0, MAP_ANON | MAP_PRIVATE, 0, 0);
			p->ref_count = 1;
			tls->pages[i] = p;
		}
		struct hash_element * hash = (struct hash_element*)calloc(1, sizeof(struct hash_element));
		hash->tid = tid;
		hash->tls = tls;
		current_hash->next = hash;
		return 0;
	}
	else {
		return -1;
	}

}

int tls_destroy() {
	pthread_t tid = pthread_self();
	struct hash_element * current_hash = hash_table;
	while (current_hash != 0 && current_hash->next != 0 && current_hash->next->tid != tid) {
		current_hash = current_hash->next;
	}
	if (current_hash == 0 || current_hash->next == 0) return -1;
	TLS* tls = current_hash->next->tls;
	for (int i = 0; i < tls->page_num; i++) {
		munmap((void *)tls->pages[i]->address, page_size);
	}
	free(tls);
	current_hash->next = 0;
	return 1;
}

void tls_protect(struct page *p)
{
	if (mprotect((void *)p->address, page_size, 0)) {
		fprintf(stderr, "tls_protect: could not protect page\n");
		exit(1);
	}
}

void tls_unprotect(struct page *p)
{
	if (mprotect((void *)p->address, page_size, PROT_READ | PROT_WRITE))
	{
		fprintf(stderr, "tls_unprotect: could not unprotect page\n");
		exit(1);
	}
}

void tls_handle_page_fault(int sig, siginfo_t *si, void *context)
{
	unsigned int p_fault = ((unsigned int)si->si_addr) & ~(page_size - 1);

	pthread_t tid = pthread_self();
	struct hash_element * current_hash = hash_table;
	while (current_hash->next != 0 && current_hash->next->tid != tid) {
		current_hash = current_hash->next;
	}
	TLS* tls = current_hash->next->tls;
	for (int i = 0; i < tls->page_num; i++) {
		if (tls->pages[i]->address == p_fault) {
			pthread_exit(NULL);
		}
	}
	signal(SIGSEGV, SIG_DFL);
	signal(SIGBUS, SIG_DFL);
	raise(sig);
}

int tls_read(unsigned int offset, unsigned int length, char *buffer) {
	pthread_t tid = pthread_self();
	struct hash_element * current_hash = hash_table;
	while (current_hash != 0 && current_hash->next != 0 && current_hash->next->tid != tid) {
		current_hash = current_hash->next;
	}
	if (current_hash == 0 || current_hash->next == 0) {
		return -1;
	}

	TLS * tls = current_hash->next->tls;
	if (offset + length > page_size * tls->page_num) return -1;
	for (int i = 0; i < tls->page_num; i++) {
		tls_unprotect(tls->pages[i]);
	}

	int cnt = 0; int idx = offset;

	for (cnt = 0, idx = offset; idx < (offset + length); ++cnt, ++idx) {
		struct page *p;
		unsigned int pn, poff;
		poff = idx % page_size;
		pn = idx / page_size;
		p = tls->pages[pn];
		
		char * src = ((char *)p->address) + poff;
		buffer[cnt] = *src;
	}
	for (int i = 0; i < tls->page_num; i++) {
		tls_protect(tls->pages[i]);
	}
	return 0;
}

int tls_write(unsigned int offset, unsigned int length, char *buffer) {
	pthread_t tid = pthread_self();
	struct hash_element * current_hash = hash_table;
	while (current_hash != 0 && current_hash->next != 0 && current_hash->next->tid != tid) {
		current_hash = current_hash->next;
	}
	if (current_hash == 0 || current_hash->next == 0) {
		return -1;
	}
	
	TLS * tls = current_hash->next->tls;
	if (offset + length > page_size * tls->page_num) return -1;
	for (int i = 0; i < tls->page_num; i++) {
		tls_unprotect(tls->pages[i]);
	}

	
	int cnt = 0; int idx = offset;
	
	for (cnt = 0, idx = offset; idx < (offset + length); ++cnt, ++idx) {		
		struct page *p, *copy;
		unsigned int pn, poff;
		pn = idx / page_size;
		poff = idx % page_size;
		p = tls->pages[pn];
		
		if (p->ref_count > 1) {
			copy = (struct page *) calloc(1, sizeof(struct page));
			copy->address = (unsigned int)mmap(0,
				page_size, PROT_WRITE,
				MAP_ANON | MAP_PRIVATE, 0, 0);
			copy->ref_count = 1;
			tls_unprotect(copy);
			tls->pages[pn] = copy;
			p->ref_count--;
			for (int i = 0; i < page_size; i++) {
				*((char*)copy->address + i) = *((char*)p->address + i);
			}
			tls_protect(p);
			p = copy;
		}
		char * dst = (char *)p->address + poff;
		*dst = buffer[cnt];
		
	}

	for (int i = 0; i < tls->page_num; i++) {
		tls_protect(tls->pages[i]);
	}
	return 0;
}

int tls_clone(pthread_t tid) {
	
	struct hash_element * current_hash = hash_table;
	while (current_hash != 0 && current_hash->next != 0 && current_hash->next->tid != pthread_self()) {
		current_hash = current_hash->next;
	}
	if (current_hash == 0 || current_hash->next != 0) return -1;
	current_hash = hash_table;
	while (current_hash->next != 0 && current_hash->next->tid != tid) {
		current_hash = current_hash->next;
	}
	if (current_hash->next == 0) return -1;

	TLS* tls = (TLS *)calloc(1, sizeof(TLS));
	tls->tid = pthread_self();
	tls->size = current_hash->next->tls->size;
	tls->page_num = current_hash->next->tls->page_num;
	tls->pages = current_hash->next->tls->pages;
	for (int i = 0; i < tls->page_num; i++) {
		struct page * p = tls->pages[i];
		p->ref_count++;
	}

	struct hash_element * hash = (struct hash_element*)calloc(1, sizeof(struct hash_element));
	hash->tid = tls->tid;
	hash->tls = tls;
	while (current_hash != 0 && current_hash->next != 0 && current_hash->next->tid != pthread_self()) {
		current_hash = current_hash->next;
	}
	current_hash->next = hash;
	return 0;
}
