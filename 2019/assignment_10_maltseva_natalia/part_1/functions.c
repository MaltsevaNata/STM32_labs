#include "functions.h"
#include <stdlib.h>
#include <stdio.h>


uint32_t partition(int32_t * a, uint32_t left, uint32_t right, uint32_t pivotIndex) {
	// Pick pivotIndex as pivot from the array
	int32_t pivot = a[pivotIndex];
	// Move pivot to end
	int32_t temp = a[pivotIndex];
	a[pivotIndex] = a[right];
	a[right] = temp;
	// elements less than pivot will be pushed to the left of pIndex
	// elements more than pivot will be pushed to the right of pIndex
	// equal elements can go either way
	uint32_t pIndex = left;
	uint32_t i;
	// each time we find an element less than or equal to pivot, pIndex
	// is incremented and that element would be placed before the pivot.
	for (i = left; i < right; i++)	{
		if (a[i] <= pivot) {
			temp = a[i];
			a[i] = a[pIndex];
			a[pIndex] = temp;
			pIndex++;
		}
	}

	// Move pivot to its final place
	temp = a[pIndex];
	a[pIndex] = a[right];
	a[right] = temp;
	return pIndex;
}



int32_t quickselect(int32_t * arr, uint32_t left, uint32_t right, uint32_t k) {
	// If the array contains only one element, return that element
	if (left == right)
		return arr[left];

	// select a pivotIndex between left and right
	uint32_t pivotIndex = left + rand() % (right - left + 1);

	pivotIndex = partition(arr, left, right, pivotIndex);

	// The pivot is in its final sorted position
	if (k == pivotIndex)
		return arr[k];

	// if k is less than the pivot index
	else if (k < pivotIndex)
		return quickselect(arr, left, pivotIndex - 1, k);

	// if k is more than the pivot index
	else
		return quickselect(arr, pivotIndex + 1, right, k);
}


void sort( int32_t * arr, uint32_t size ) { //Gnome sort 
	uint32_t curr_el = 1;
	uint32_t return_el = 2;
	while (curr_el < size) { 
		if (arr[curr_el-1] < arr[curr_el]) { //going forward
			
			curr_el = return_el;
			return_el++;
		}	else { 				//going backward with swap
			int32_t temp = arr[curr_el];
			arr[curr_el] = arr[curr_el-1];
			arr[curr_el-1] = temp;
			curr_el--;
			if (curr_el==0)	{
				curr_el = return_el;
				return_el++;
			}
		}
	}

}

int32_t getMedian( int32_t * arr, uint32_t size ) { 
	return quickselect(arr, 0, size-1, (size-1)/2);
}

int32_t getOrderStatistic( int32_t * arr, uint32_t size, uint32_t j ) {
	return quickselect(arr, 0, size-1, j);
}

ArrayOf11 sortStruct( ArrayOf11 array ) {
	sort(array.a, 11 );
	return array;
}





