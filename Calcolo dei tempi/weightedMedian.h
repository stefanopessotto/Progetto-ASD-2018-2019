#define SELECT_BASE_SIZE		5
#define SELECT_BLOCKSIZE		5 //must be odd, otherwise median of the block will be unknown (like : median of 6 is 2 or 3?)

#define ARRAY_BASE_SIZE			100

#define BUFFER_LEN				100

double weightedMedianDeterministic(double* array, int size);

double weightedMedianRandom(double* array, int size);

double weightedMedianInsertionSort( double* array, int size );

double weightedMedianQuickSort( double* array, int size );
