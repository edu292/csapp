short sum_even() {
  short sum = 0;
  for (short i = 0; i < 10; i++) {
    if (i & 1)
      continue;
    sum += i;
  }
  return sum;
}

short sum_even_naive_goto() {
    short sum = 0;
    short i = 0;
loop:
        i++;
    if (i & 1) goto loop;
    sum += i;
condition:
    if (i <= 10) goto loop;
    return sum;
}
