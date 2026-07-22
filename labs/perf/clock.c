#include "clock.h"
#include <stdint.h>
#include <stdio.h>
#include <sys/times.h>
#include <time.h>
#include <unistd.h>
#include <x86intrin.h>

static clock_t start_cyc = 0;

void start_counter() { start_cyc = __rdtsc(); }

double get_counter() { return (double)(__rdtsc() - start_cyc); }

double ovhd() {
  /* Do it twice to eliminate cache effects */
  int i;
  double result;

  for (i = 0; i < 2; i++) {
    start_counter();
    result = get_counter();
  }
  return result;
}

/* $begin mhz */
/* Estimate the clock rate by measuring the cycles that elapse */
/* while sleeping for sleeptime seconds */
double mhz_full(int verbose, int sleeptime) {
  double rate;

  start_counter();
  sleep(sleeptime);
  rate = get_counter() / (1e6 * sleeptime);
  if (verbose)
    printf("Processor clock rate ~= %.1f MHz\n", rate);
  return rate;
}
/* $end mhz */

/* Version using a default sleeptime */
double mhz(int verbose) { return mhz_full(verbose, 2); }

/** Special counters that compensate for timer interrupt overhead */

static double cyc_per_tick = 0.0;

#define NEVENT 100
#define THRESHOLD 1000
#define RECORDTHRESH 3000

/* Attempt to see how much time is used by timer interrupt */
static void callibrate(int verbose) {
  struct tms t;
  times(&t);
  clock_t oldc = t.tms_utime;

  start_counter();
  double oldt = get_counter();
  int e = 0;

  while (e < 100) {
    double newt = get_counter();
    if (newt - oldt >= 1000) {
      times(&t);
      clock_t newc = t.tms_utime;
      if (newc > oldc) {
        double cpt = (newt - oldt) / (newc - oldc);
        if ((cyc_per_tick == 0.0 || cyc_per_tick > cpt) && cpt > 3000)
          cyc_per_tick = cpt;
        e++;
        oldc = newc;
      }
      oldt = newt;
    }
  }
}

static clock_t start_tick = 0;

void start_comp_counter() {
  struct tms t;

  if (cyc_per_tick == 0.0)
    callibrate(1);
  times(&t);
  start_tick = t.tms_utime;
  start_counter();
}

double get_comp_counter() {
  double time = get_counter();
  struct tms t;
  times(&t);
  return time - (t.tms_utime - start_tick) * cyc_per_tick;
}
