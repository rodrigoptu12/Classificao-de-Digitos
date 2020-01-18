#include <stdio.h> /* input, output */
#include "mnist.h"

#define VERBOSE 1

int main(int nargin, char **argv){
  int s, c, n_splits, split_size, n_pixels=1;
  LecunDataType input, output;
  long unsigned int din, dout;

  if(VERBOSE){
    printf("nargin=%d\n", nargin);
    for(c=0; c < nargin; c++)
      printf("argv[%d] = %s\n", c, argv[c]);
  }

  sprintf(input.filename, "%s", argv[1]);
  if(read_LecunData(&input)) return 0;
  n_splits = atoi(argv[2]);

  split_size = input.dimensions[0]/n_splits;
  printf("split_size=%d\n", split_size);
  output.n_dimensions = input.n_dimensions;
  output.data_type = input.data_type;
  output.data_bytes = input.data_bytes;
  output.n_dimensions = input.n_dimensions;
  printf("output.data_bytes=%d\n", output.data_bytes);
  output.dimensions = (int *) calloc(output.n_dimensions, sizeof(int));
  if(output.dimensions==NULL){
    printf("Failed to allocate dimensions vector.");
    return 0;
  }
  output.dimensions[0] = split_size;
  if(output.n_dimensions == 3){ /* images */
    output.dimensions[1] = input.dimensions[1];
    output.dimensions[2] = input.dimensions[2];
    n_pixels = input.dimensions[1]*input.dimensions[2];
  }
  output.total_dims = output.dimensions[0]*n_pixels;
  printf("output.total_dims=%lu\n", output.total_dims);
  output.data = (unsigned char *) calloc(output.total_dims, output.data_bytes);
  if(output.data == NULL){
    printf("Failed to allocate data array.\n");
    return 0;    
  }
  else{
    printf("Successfully allocated data array.\n");
  }

  din=0;
  for(s=0; s<n_splits; s++){
    sprintf(output.filename, "%s_split%02d", input.filename, s);
    printf("Writing %s\n", output.filename);
    for(dout=0; dout<output.total_dims; dout++){
      output.data[dout] = input.data[din];
      din++;
    }
    write_LecunData(output);
  }

  free(input.dimensions);
  free(input.data);
  free(output.dimensions);
  free(output.data);

  return 0;
}
