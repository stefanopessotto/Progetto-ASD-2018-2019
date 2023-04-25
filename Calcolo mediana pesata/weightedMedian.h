#define SELECT_BASE_SIZE 5
// should be odd, otherwise median of the block will be unknown
// (like : median of 6 is 2 or 3?)
#define SELECT_BLOCKSIZE 5

#define ARRAY_BASE_SIZE 100

#define BUFFER_LEN 100
#define TMPBUFFER_LEN 100

double weightedMedianDeterministic(double *array, int size);
