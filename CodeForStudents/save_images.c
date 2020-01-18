#include <stdio.h> /* input, output */
#include "mnist.h"

#define VERBOSE 1

int main(int nargin, char **argv){
  int c;
  LecunDataType input;

  if(VERBOSE){
    printf("nargin=%d\n", nargin);
    for(c=0; c < nargin; c++)
      printf("argv[%d] = %s\n", c, argv[c]);
  }

  sprintf(input.filename, "%s", argv[1]);
  if(read_LecunData(&input)) return 0;


  write_all_imgs(argv[2], input);
  
  free(input.dimensions);
  free(input.data);

  return 0;
}
