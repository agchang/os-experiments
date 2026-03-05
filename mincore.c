#include <sys/mman.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <stdint.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <time.h>

static double now_ns(void) {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return ts.tv_sec * 1e9 + ts.tv_nsec;
}

void print_mincore(void *ptr, size_t length, unsigned char *vec, size_t vec_size) {
  int status = mincore(ptr, 4096, vec);
  printf("status: %d\n", status);
  if (status == -1) {
    printf("%d\n", errno);
    printf("%s\n", strerror(errno));
  }
  for (int i = 0; i < vec_size; i++) {
    printf("%d\n", vec[i]);
  }
}

int main() {
  int x = 0;
  size_t vec_size = (4096+4095)/4096; // 1
  printf("vec_size: %ld\n", vec_size);
  char vec[vec_size];
  uintptr_t addr = (uintptr_t)&x;
  addr = (addr/4096) * 4096;
  void* ptr = (void*)addr;
  print_mincore(ptr, 4096, vec, vec_size);

  // Now try a mmap file
  int fd = open("output.bin", O_RDONLY);
  void* fptr = mmap(NULL, 4096*2, PROT_READ, MAP_PRIVATE, fd, 0);
  // interesting part - this will return 1 if the os has it in the page cache, for example when I first created the file
  // run echo 3 > /proc/sys/vm/drop_caches to flush the page cache
  print_mincore(fptr, 4096, vec, vec_size);
  // access it
  double start_ts = now_ns();
  volatile char val = *(char*)fptr;
  double elapsed = now_ns() - start_ts;
  // now if you run this program again, it's in the page cache!
  printf("%d\n", val);
  printf("%f\n", elapsed);
  
  // access time once it's in the page table
  start_ts = now_ns();
  val = *(volatile char*)fptr;
  elapsed = now_ns() - start_ts;
  printf("%d\n", val);
  printf("%f\n", elapsed);

  return 0;
}
