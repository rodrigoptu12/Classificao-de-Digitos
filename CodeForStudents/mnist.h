#include<stdio.h> /* input, output */
#include<stdlib.h> /* malloc, calloc, atoi */
#include<sys/stat.h> /* mkdir */

#define NCLASSES 10

/* Data structures. */
typedef struct{
  char filename[256];
  unsigned char n_dimensions;
  unsigned char data_type;
  unsigned char data_bytes;
  int *dimensions;
  unsigned char *data;
  long unsigned int total_dims;
}LecunDataType;

typedef struct DT{
  double data;
  int idx;
}DistanceType;


/* Functions to deal with the MNIST dataset. */
int read_LecunData(LecunDataType *lecun);
int write_LecunData(LecunDataType lecun);
int reverseInt(int i);
void print_all_data(LecunDataType lecun);
int write_all_imgs(char *trg_directory, LecunDataType lecun);
int write_img(unsigned char *uchar_ptr, int n_rows, int n_cols, char* filename);

/* Classification functions. */
int classify(LecunDataType train, LecunDataType labels, LecunDataType test, LecunDataType *result, int K);
double distance(unsigned char *x1, unsigned char *x2, int dims);
double get_accuracy(LecunDataType gt, LecunDataType result);

/* Sorting (and auxiliary) functions. */
long unsigned int SortK(DistanceType *vector, int N, int K);
long unsigned int InsertionSort(DistanceType *vector, int N);
long unsigned int MergeSort(DistanceType *A, int n);
void TopDownSplitMerge(DistanceType *B, int iBegin, int iEnd, DistanceType *A, long unsigned int *operations);
void TopDownMerge(DistanceType *A, int iBegin, int iMiddle, int iEnd, DistanceType *B, long unsigned int *operations);
long unsigned int CopyArray(DistanceType *A, int iBegin, int iEnd, DistanceType *B);

