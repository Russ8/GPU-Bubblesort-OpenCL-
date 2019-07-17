# GPU-Quicksort-OpenCL-
An implementation of the quicksort sorting algorithm that runs on a GPU using OpenCL  

Description:
Using GPU hardware to perform parallel sorting algorithms is a common research topic, as
many sorting algorithms are highly parallelisable. In this report, a parallel Bubblesort
algorithm is presented which is implemented using the OpenCl GPGPU framework.
Bubblesort when implemented sequentially is not an efficient algorithm, and there is much
that can be improved in the parallel form.  

Instructions:  

Make:  
```$ make```   
which will create the executable: "main"  

To Run:  
```./main n w```  
   where:  
      *2^n gives the array size (keep in mind the power sign)  
      *w gives local work items  
      *w must be greater than (2^n)/2 , else an error will be returned  
      *if n is excessively large, an error will be returned  

Results:  
