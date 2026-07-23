#include <stdio.h>
#include <threads.h>

#define NUM_THREADS 8
#define INCREMENTS_PER_THREAD 100000

typedef void (*increment_f)(int *counter);

void bad_incr(int *ptr) { (*ptr)++; }

void lock_incr(int *ptr) {
  asm("lock incl %[ptr]" : [ptr] "=m"(*ptr));
}

static int g_counter = 0;
static increment_f g_current_func = NULL;

int worker(void *arg) {
  (void)arg;
  for (int i = 0; i < INCREMENTS_PER_THREAD; i++) {
    g_current_func(&g_counter);
  }
  return 0;
}

int main(void) {
  struct {
    const char *name;
    increment_f func;
  } algos[] = {{"bad incr", bad_incr}, {"lock asm incr", lock_incr}};
  int num_algos = sizeof(algos) / sizeof(algos[0]);
  const int expected = NUM_THREADS * INCREMENTS_PER_THREAD;

  printf("=== TESTS (%d threads, %d inc/thread) ===\n\n", NUM_THREADS,
         INCREMENTS_PER_THREAD);

  for (int a = 0; a < num_algos; a++) {
    g_counter = 0;
    g_current_func = algos[a].func;

    thrd_t threads[NUM_THREADS];

    for (int i = 0; i < NUM_THREADS; i++) {
      if (thrd_create(&threads[i], worker, NULL) != thrd_success) {
        fprintf(stderr, "Failed to create thread %d\n", i);
        return 1;
      }
    }

    for (int i = 0; i < NUM_THREADS; i++) {
      thrd_join(threads[i], NULL);
    }

    printf("Algorithm: %s\n", algos[a].name);
    printf("  Expected: %d\n", expected);
    printf("  Actual:   %d\n", g_counter);
    printf("  Status:   %s\n\n",
           (g_counter == expected) ? "PASSED" : "FAILED (Data Race)");
  }

  return 0;
}
