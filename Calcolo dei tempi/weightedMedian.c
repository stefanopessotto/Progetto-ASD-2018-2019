#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include "weightedMedian.h"

int fixedPartition( double* array, int start, int end, double *pivot );

double* DeterministicSelect( int k, double* array, int start, int end );

void swap(double* first, double* second)
{
	double tmp;
	if( (first != NULL) && (second != NULL) )
	{
		tmp = *first;
		*first = *second;
		*second = tmp;
	}
}


void quickSort( double* array, int start, int end )
{
	if( (end - start) > 1 )
	{
		int half = floor((end - start) / 2);
		double* pivot = DeterministicSelect(half, array, start, end);
		int pivotPosition = fixedPartition(array, start, end, pivot);
		quickSort(array, start, start + pivotPosition);
		quickSort(array, start + pivotPosition + 1, end);
	}
}

double weightedMedianQuickSort( double* array, int size ){
	double sum = 0, target, prov = 0;
	int i;

	quickSort( array, 0, size );

	for( i = 0; i < size; i++ ){
		sum += array[i];
	}

	i = -1;
	target = sum / 2;

	while( (float)prov < (float)target){
		i++;
		prov = prov + array[i];
	}

	return array[i];
}

void insertionSort(double* array, int start, int size)
{
	double current;
	int i, j;

	for( j = start + 1 ; j < size; j++ ){
		current = array[j];
		i = j - 1;

		while( ( i >= start ) && ( current < array[i] ) )
		{
			array[i + 1] = array[i];
			i = i - 1;
		}

		array[i + 1] = current;
	}
}

double weightedMedianInsertionSort( double* array, int size ){
	double sum = 0, target, prov = 0;
	int i;

	insertionSort( array, 0, size );

	for( i = 0; i < size; i++ ){
		sum += array[i];
	}

	i = -1;
	target = sum / 2;

	while( (float)prov < (float)target){
		i++;
		prov = prov + array[i];
	}

	return array[i];
}

/**
 * Fixed implementation of partition
*/
int fixedPartition( double* array, int start, int end, double *pivot )
{
	short moveToStart = 0;
	int i = start - 1;

	for( int k = start; k < end; k++ )
	{
		if( array[k] < (*pivot) ){
			i++;
			swap(&(array[i]), &array[k]);
		}
		if( array[k] == (*pivot) )
		{
			if( (moveToStart) || (&(array[k]) == pivot) ) 											//always swap if it is the pivot so we know it is in the left part of the array
			{
				i++;
				swap( &(array[i]), &(array[k]));
				if( &(array[k]) == pivot ) 															//it was the pivot! update its position
					pivot = &(array[i]);
			}
			moveToStart = !moveToStart;
		}
	}

	swap(&(array[i]),pivot);																		//swap pivot in the correct position

	return i - start;
}

double* DeterministicSelect( int k, double* array, int start, int end )
{
	int missingValues, i, j, numberOfBlocks, pivotPosition, medianStart, medianEnd;
	double *pivot = NULL, *maxValue = NULL;

	int size = (end - start);

	if( size < SELECT_BASE_SIZE )																	//recursion base case
	{
		insertionSort( array, start, end);
		return &(array[start + k]);
	}


	missingValues = ( SELECT_BLOCKSIZE - ( size % SELECT_BLOCKSIZE) ) % SELECT_BLOCKSIZE;			//how much values are needed to fill the last block?

	numberOfBlocks = (size + missingValues) / SELECT_BLOCKSIZE;

	for( i = 0; i < numberOfBlocks; i++ )															//sort each block
	{
		if( i == (numberOfBlocks - 1) ) 															//if it's the last block, sort it to the end
			insertionSort( array, (start + (i * SELECT_BLOCKSIZE)), end);
		else
			insertionSort( array, (start + (i * SELECT_BLOCKSIZE)), (start + (i * SELECT_BLOCKSIZE) + SELECT_BLOCKSIZE));
	}

	if( missingValues > 2 )																			//size issues occur only if the last block doesn't reach the median position
	{																								//lookup for the last occurrence of the highest value
		maxValue = &(array[start]);
		for( i = start; i < end; i++ )
			if( array[i] >= *maxValue )
			{
				maxValue = &(array[i]);
			}
	}
																									//move medians to the center
	j = 1;
	i = (start + 2 + ((numberOfBlocks - 1) /2) * SELECT_BLOCKSIZE);									//i starts with the center median
	medianStart = i;
	medianEnd = i + 1;

	while( j <= (numberOfBlocks / 2))
	{
		if( ( i + ( j * SELECT_BLOCKSIZE) ) < end )													//get medians located after i near it
		{
			swap( &(array[i + j ]), &(array[ ( i + (j * SELECT_BLOCKSIZE) ) ]));

			if( &(array[i + j ]) == maxValue )														//update maxValue if swapped
				maxValue = &(array[ ( i + (j * SELECT_BLOCKSIZE) ) ]);

		}else 																						//last block doesn't have a median, get the max calculated before
		{
			swap( &(array[i + j ]), maxValue);
		}
		medianEnd++;

		if( ( i - ( j * SELECT_BLOCKSIZE) ) > start )												//get medians located before i near it
		{
			swap( &(array[i - j ]), &(array[ ( i - (j * SELECT_BLOCKSIZE) ) ]));

			if(  &(array[i + j ]) == maxValue )														//update maxValue if swapped
				maxValue = &(array[ ( i - (j * SELECT_BLOCKSIZE) ) ]);

			medianStart--;																			//at the beginning i could be not the exact center. So there may be less blocks in the left part.
		}
		j++;
	}
																									//lookup for median of medians
	pivot = DeterministicSelect( (int)floor( (double)size /(double)(SELECT_BLOCKSIZE * 2) ) , array, medianStart, medianEnd);
																									//partition around it
	pivotPosition = fixedPartition( array, start, end, pivot );

	if( pivotPosition == k )
	{
		return &(array[ pivotPosition + start ]);
	}else
	{
		if( pivotPosition < k )
		{

			pivot = DeterministicSelect( (k - (pivotPosition + 1)), array, ( start + pivotPosition + 1 ), end);

			return pivot;
		}else
		{

			pivot = DeterministicSelect( k, array, start, start + pivotPosition);

			return pivot;
		}
	}
}

/**
 * Recursive support for the Weighted median algorithm. It applies a binary search algorithm on the target
*/
double weightedMedianRecDeterministic(double* array, int p, int q, double target)
{
	double *pivot, weightBeforeMedian = 0, weightAfterMedian = 0;

	if( (q - p ) == 1 ) 																			//if there's only one element it's the wm.
	{
		return array[p];
	}else
	{
		int half = ((q - p) / 2), pivotPosition;
																									//find pivot & partition
		pivot = DeterministicSelect(half, array, p, q);
		pivotPosition = half; //fixedPartition(array, p, q, pivot);
																									//count weights
		for( int i = p; i <= (p + pivotPosition); i++ )
		{
			if( i < (p + pivotPosition) )
				weightBeforeMedian += array[i];
			else
				if( i == (p + pivotPosition) )														//weight of the pivot is added in another variable
					weightAfterMedian = weightBeforeMedian + array[i];
		}

																									//check with downcast
		if(((float)weightBeforeMedian < (float)target) && ((float)weightAfterMedian >= (float)target))
		{
			return *pivot;
		}else
		{
			if( (float)weightBeforeMedian >= (float)target )										//check with downcast
			{
				return weightedMedianRecDeterministic(array, p, ( p + pivotPosition ),target);
			}else
			{
				 return weightedMedianRecDeterministic(array, ( p + pivotPosition + 1 ), q, ( target - weightAfterMedian ));
			}
		}
	}
}

/**
 * Main algorithm for the wm. Calculate the target and call the recursive procedure.
*/
double weightedMedianDeterministic(double* array, int size)
{
	double target = 0;

	for( int i = 0; i < size; i++ )
		target += array[i];

	target /= 2;

	return weightedMedianRecDeterministic(array, 0, size, target);
}

double* randomSelect( int k, double* array, int start, int end )
{
	int pivotPosition, lookedPosition;
	double *pivot = NULL;

	int size = (end - start);

	if( size < SELECT_BASE_SIZE )																	//recursion base case
	{
		insertionSort( array, start, end);
		return &(array[start + k]);
	}

	lookedPosition = start + (rand() % ((end - start) - 1));
	pivot = &(array[ lookedPosition ]);
																									//partition around it
	pivotPosition = fixedPartition( array, start, end, pivot );

	if( pivotPosition == k )
	{
		return &(array[ pivotPosition + start ]);
	}else
	{
		if( pivotPosition < k )
		{

			pivot = randomSelect( (k - (pivotPosition + 1)), array, ( start + pivotPosition + 1 ), end);

			return pivot;
		}else
		{

			pivot = randomSelect( k, array, start, start + pivotPosition);

			return pivot;
		}
	}
}

double weightedMedianRecRandom(double* array, int p, int q, double target)
{
	double *pivot, weightBeforeMedian = 0, weightAfterMedian = 0;

	if( (q - p ) == 1 ) 																			//if there's only one element it's the wm.
	{
		return array[p];
	}else
	{
		int half = ((q - p) / 2), pivotPosition;
																									//find pivot & partition
		pivot = randomSelect(half, array, p, q);
		pivotPosition = fixedPartition(array, p, q, pivot);
																									//count weights
		for( int i = p; i <= (p + pivotPosition); i++ )
		{
			if( i < (p + pivotPosition) )
				weightBeforeMedian += array[i];
			else
				if( i == (p + pivotPosition) )														//weight of the pivot is added in another variable
					weightAfterMedian = weightBeforeMedian + array[i];
		}

																									//check with downcast
		if(((float)weightBeforeMedian < (float)target) && ((float)weightAfterMedian >= (float)target))
		{
			return *pivot;
		}else
		{
			if( (float)weightBeforeMedian >= (float)target )										//check with downcast
			{
				return weightedMedianRecRandom(array, p, ( p + pivotPosition ),target);
			}else
			{
				 return weightedMedianRecRandom(array, ( p + pivotPosition + 1 ), q, ( target - weightAfterMedian ));
			}
		}
	}
}

double weightedMedianRandom(double* array, int size)
{
	double target = 0;

	for( int i = 0; i < size; i++ )
		target += array[i];

	target /= 2;

	return weightedMedianRecRandom(array, 0, size, target);
}

