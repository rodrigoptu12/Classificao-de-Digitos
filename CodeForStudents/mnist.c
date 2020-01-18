#include<stdio.h> /* input, output */
#include<stdlib.h> /* malloc, calloc, atoi */
#include<sys/stat.h> /* mkdir */
#include "mnist.h"

#define VERBOSE 1

/*
  http://yann.lecun.com/exdb/mnist/ 
*/


/**********************************************************
 * Input/Output function for MNIST data in Lecun's IDX format */

/****************
 * Function to read data from a file IDX format: */
int read_LecunData(LecunDataType *lecun){
  FILE *fp;
  int magic_number, tmp;
  unsigned char d;
  
  fp = fopen(lecun->filename, "rb");
  if(!fp){
    printf("Could not open %s for reading.\n", lecun->filename);
    return 1;
  }
  printf("Reading IDX file %s\n", lecun->filename);
  if(fread(&magic_number, sizeof(magic_number), 1, fp)!=1){
    printf("Error reading magic number.\n");
    fclose(fp);
    return 2;
  }
  printf("magic_number=%X\n", magic_number);
  lecun->data_type = (magic_number >> 2*8) & 0x00FF;
  printf("data_type=%x\n", lecun->data_type);
  lecun->n_dimensions = magic_number >> 3*8;
  printf("n_dimensions=%d\n", lecun->n_dimensions);

  lecun->dimensions = (int *) calloc(lecun->n_dimensions, sizeof(int));
  if(lecun->dimensions == NULL){
    printf("Unable to allocate dimensions array.\n");
    fclose(fp);
    return 3;
  }    

  lecun->total_dims = 1;
  for(d=0; d<lecun->n_dimensions; d++){    
    if(fread(&tmp, sizeof(tmp), 1, fp) !=1){      
      printf("Error reading dimension %d.\n", d);
      fclose(fp);
      return 4;
    }
    lecun->dimensions[d] = reverseInt(tmp);
    lecun->total_dims *= lecun->dimensions[d];
    printf("dimensions[%d]=%d\n", d, lecun->dimensions[d]);
  }

  printf("total_dims=%lu\n", lecun->total_dims);
  switch(lecun->data_type){
  case 0x08:
  case 0X09:
    lecun->data_bytes = 1;
    break;
  case 0x0B:
    lecun->data_bytes = 2;
    break;
  case 0x0C:
  case 0x0D:
    lecun->data_bytes = 4;
    break;
  case 0X0E:
    lecun->data_bytes = 8;
    break;
  default:
    printf("Unknown data type.\n");
    fclose(fp);
    return 5;
  }
  printf("data_bytes=%d\n", lecun->data_bytes);

  lecun->data = (unsigned char *) calloc(lecun->total_dims, lecun->data_bytes);
  if(lecun->data == NULL){
    printf("Failed to allocated data\n.");
    fclose(fp);
    return 6;
  }
    
  if(fread(lecun->data, lecun->data_bytes, lecun->total_dims, fp) != lecun->total_dims){
    printf("Error reading data matrix.\n");
    fclose(fp);
    return 7;    
  }
  fclose(fp);  

  return 0; /* No error, i.e., success! */
}

/****************
 * Function to write data to a file IDX format: */
int write_LecunData(LecunDataType lecun){
  FILE *fp;
  int tmp, d;
  long unsigned int t;

  /* Convert info to magic number. */
  tmp = lecun.n_dimensions;
  tmp = tmp << 8;
  tmp |= lecun.data_type;
  tmp = tmp << 2*8;

  fp = fopen(lecun.filename, "wb");
  if(!fp){
    printf("Could not open %s for writing.\n", lecun.filename);
    return 1;
  }

  if(!fwrite(&tmp, sizeof(tmp), 1, fp)){
    printf("Could not write magic number to %s\n", lecun.filename);
    fclose(fp);
    return 1;
  }

  /* Dimensions */
  for(d=0; d<lecun.n_dimensions; d++){
    tmp = reverseInt(lecun.dimensions[d]);
    if(!fwrite(&tmp, sizeof(tmp), 1, fp)){
      printf("Could not write dimension %d to %s\n", d, lecun.filename);
      fclose(fp);
      return 1;
    }
  }
  t = fwrite(lecun.data, lecun.data_bytes, lecun.total_dims, fp); 
  if(t != lecun.total_dims){
    printf("Only wrote %lu out of %lu data blocks to %s.\n", t, lecun.total_dims, lecun.filename);    
    fclose(fp);
    return 1;
  }
  fclose(fp);
  return 0;
}


/****************
 * Function that reverses bytes in an integer number: */
int reverseInt(int i) {
  unsigned char c1, c2, c3, c4;
  /* printf("%X\n", i); */
  c1 = i & 255;
  c2 = (i >> 8) & 255;
  c3 = (i >> 16) & 255;
  c4 = (i >> 24) & 255;
  return ((int)c1 << 24) + ((int)c2 << 16) + ((int)c3 << 8) + c4;
}

/****************
 * Function to dump a dataset file as a directory tree containing all images: */
int write_all_imgs(char *trg_directory, LecunDataType lecun){
  int n, n_pixels;
  char img_dir[255], img_filename[20];

  n_pixels = lecun.dimensions[1]*lecun.dimensions[2];
  if(lecun.data_bytes == 1){
    mkdir(trg_directory, S_IRWXU); 
    for(n=0; n < lecun.dimensions[0]; n++){
      if(!(n%1000)){
        sprintf(img_dir, "%s/%02d", trg_directory, n/1000);
        if(VERBOSE>1) printf("Making dir %s ", img_dir);
        if(!mkdir(img_dir, S_IRWXU)) printf("OK.\n");
	else if(VERBOSE>1) printf("Failed.\n");
      }
      
      sprintf(img_filename, "%s/%03d.pgm", img_dir, n%1000);
      if(VERBOSE>2) printf("Writing %s\n", img_filename);
      if(write_img(&lecun.data[n*n_pixels],
		   lecun.dimensions[1], lecun.dimensions[2], img_filename)){
	printf("Failed to write sample %d.\n", n);
        return n;
      }
    }
  }
  return 0;
}

/****************
 * Function to write an image to a PGM file, for visualisation purposes: */
int write_img(unsigned char *uchar_ptr, int n_rows, int n_cols, char* filename){
  int row, col;
  FILE *fp = fopen(filename, "w");
  if(!fp){
    printf("Couldn't open %s\n", filename);
    return 1;
  }
  fprintf(fp, "P2\n");
  fprintf(fp, "%d %d\n255\n", n_cols, n_rows);
  for(row=0; row<n_rows; row++){
    for(col=0; col<n_cols; col++){
      uchar_ptr++;
      fprintf(fp, "%d", *uchar_ptr);
      if(col<n_cols-1) fprintf(fp, " ");
      else fprintf(fp, "\n");
    }
  }
  fclose(fp);
  return 0;
}

/****************
 * Function that prints on the screen everything that is in the raw
 * data part of the dataset file: */
void print_all_data(LecunDataType lecun){
  long unsigned int c;

  if(lecun.data_bytes == 1){
    /* Print the whole data matrix: */
  
    for(c=0; c < lecun.total_dims; c++)
      printf("%d ", lecun.data[c]);
    printf("\n");
  }
}


/**********************************************************
 * Classification helper functions */
/************************
 * distance between two vectors. */ 
double distance(unsigned char *x1, unsigned char *x2, int dims){
  /* This function computes the Manhattan distance, a.k.a.
     Minkowski distance or L1 norm of the difference. */
  int c;
  double d=0;
  for(c=0; c<dims; c++){
    if(VERBOSE > 3) printf("%d ", c);
    d += abs((double) x1[c] - (double) x2[c]);
  }
  if(VERBOSE > 3) printf("\n");
  return d;
}


/************************
 * Accuracy calculation */
double get_accuracy(LecunDataType gt, LecunDataType result){
  long unsigned int d;
  double accuracy = 0;
  if(gt.total_dims != result.total_dims) return 0;
  for(d=0; d<gt.total_dims; d++){
    accuracy += (gt.data[d]==result.data[d]);
  }
  return(100*accuracy/gt.total_dims);
}



/**********************************************************
 * Sorting functions. */
/******************
 * SortK
 * This is a simplification of a forward version of 
 * InsertionSort, which sorts only the first K elements. */
long unsigned int SortK(DistanceType *vector, int N, int K){
  int i, k;
  long unsigned int operations=0;
  DistanceType aux;
  aux.data = 3.402823e+38; /* FLT_MAX */

  for(k=0; k<K; k++){
    for(i=k+1; i<N; i++){
      operations++;
      if(vector[i].data < vector[k].data){
	aux = vector[i];
	vector[i] = vector[k];
	vector[k] = aux;
      }
    }
  }
  return operations;
}

/******************
 * InsertionSort
   https://en.wikipedia.org/wiki/Insertion_sort 
*/
long unsigned int InsertionSort(DistanceType *vector, int N){  
  int i, j;
  long unsigned int operations=0;
  DistanceType aux;
   
  for(i=1; i<N; i++){
    if(VERBOSE>4) printf("%d ", i);
    aux.data = vector[i].data;
    aux.idx = vector[i].idx;
    
    j = i-1;
    operations++;
    while((j>=0) && (vector[j].data>aux.data)){      
      vector[j+1].data = vector[j].data;
      vector[j+1].idx = vector[j].idx;
      j--;
      operations++;
    }

    vector[j+1].data = aux.data;
    vector[j+1].idx = aux.idx;
  }
  return operations;
}


/************
 * MergeSort
   Left  source half is A[ iBegin:iMiddle-1].
   Right source half is A[iMiddle:iEnd-1   ].
   Result is            B[ iBegin:iEnd-1   ]. */

/* MergeSort top function: triggers the process. */
long unsigned int MergeSort(DistanceType *A, int n){
  /* Array A[] has the items to sort; array B[] is a work array. */
  long unsigned int operations;
  DistanceType *B;

  if(VERBOSE>4) printf("TopDownMergeSort\n");
  B = (DistanceType *) calloc(n, sizeof(DistanceType));  
  operations = CopyArray(A, 0, n, B);     /*  Duplicate array A[] into B[] */
  TopDownSplitMerge(B, 0, n, A, &operations); /*  Sort data from B[] into A[]  */
  free(B);
  return operations;
}


/* MergeSort main function:
   Sort the given run of array A[] using array B[] as a source.
   iBegin is inclusive; iEnd is exclusive (A[iEnd] is not in the set). */
void TopDownSplitMerge(DistanceType *B, int iBegin, int iEnd, DistanceType *A, long unsigned int *operations){
  int iMiddle;

  if(VERBOSE>4) printf("TopDownSplitMerge\n");
  (*operations)++;
  if(iEnd - iBegin < 2)  /* If run size == 1, consider it sorted. */
    return;              
  
  /* Split the run longer than 1 item into halves. */
  iMiddle = (iEnd + iBegin) / 2;  /* iMiddle = mid point */
  /* Recursively sort both runs from array A[] into B[]  */
  TopDownSplitMerge(A, iBegin,  iMiddle, B, operations);  /* Sort the left  run. */
  TopDownSplitMerge(A, iMiddle,    iEnd, B, operations);  /* sort the right run. */
  /* Merge the resulting runs from array B[] into A[]   */
  TopDownMerge(B, iBegin, iMiddle, iEnd, A, operations);
}


/* MergeSort merging function: */
void TopDownMerge(DistanceType *A, int iBegin, int iMiddle, int iEnd, DistanceType *B, long unsigned int *operations)
{
  int i=iBegin, j=iMiddle, k;

  if(VERBOSE>4) printf("TopDownMerge\n");
   
  /* While there are elements in the left or right runs... */
  for(k=iBegin; k<iEnd; k++){
    (*operations)++;
    /* If left run head exists and is <= existing right run head. */
    if (i < iMiddle && (j >= iEnd || A[i].data <= A[j].data)) {
      B[k].data = A[i].data;
      B[k].idx = A[i].idx;
      i = i + 1;
    } else {
      B[k].data = A[j].data;
      B[k].idx = A[j].idx;      
      j = j + 1;    
    }
  }
}


/* Auxiliary function for MergeSort:
   Copy array A to array B, from indices iBetin to iEnd */
long unsigned int CopyArray(DistanceType *A, int iBegin, int iEnd, DistanceType *B){
  long unsigned int operations=0, k;

  if(VERBOSE>4) printf("CopyArray\n");
  for(k=iBegin; k<iEnd; k++) {
    B[k].data = A[k].data;
    B[k].idx = A[k].idx;
    operations++;
  }
  return operations;
}



