


//kernel function
__kernel void vector_add(__global int *A,uint order) {
 
    // Get the index of the current element to be processed
    int i = get_global_id(0) * 2;
 
	//compare the pair
	int  compareA = i + order;
	int  compareB = i + 1 + order;

	if(A[compareA] > A[compareB]) {
		int temp = A[compareA];
		A[compareA] = A[compareB];
		A[compareB] = temp;
	}

}
