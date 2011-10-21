/* Prints the command-line arguments.
   This program is used for all of the args-* tests.  Grading is
   done differently for each of the args-* tests based on the
   output. */

#include "tests/lib.h"

int
main (int argc, char *argv[]) 
{
  int i;

  test_name = "args";

  //msg ("begin");
  printf("begin\n");
  //msg ("argc = %d", argc);
  printf("argc = %d\n", argc);
  for (i = 0; i <= argc; i++)
    if (argv[i] != NULL)
      printf("argv[%d] = '%s'\n", i, argv[i]);
      //msg ("argv[%d] = '%s'", i, argv[i]);
    else
      printf("argv[%d] = null\n", i);
     // msg ("argv[%d] = null", i);
  //msg ("end");
  printf("end\n");
  return 0;
}
