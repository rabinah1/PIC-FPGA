#define PROT_READ 1
#define PROT_WRITE 3
#define MAP_SHARED 1
#define MAP_FAILED (void *)-1

void *mmap(void *addr, int size, int opts, int shared, int fd, unsigned int base);
