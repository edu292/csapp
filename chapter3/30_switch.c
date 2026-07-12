void switch1(short x, short *dest) {
  short val = 0;
  switch (x) {
  case -2:
  case -1 | 6:
  case 0:
  case 1 | 3:
  case 4:
  }
  *dest = val;
}

/*
void switch1(short x, short *dest)
x in %rdi
switch2:
  addq $2, %rdi
  cmpq $8, %rdi
  ja .L2
  jmp *.L4(,%rdi,8)
L4:
  .quad .L9
  .quad .L5
  .quad .L6
  .quad .L7
  .quad .L2
  .quad .L7
  .quad .L8
  .quad .L2
  .quad .L5
*/
