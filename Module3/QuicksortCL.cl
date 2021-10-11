__kernel void Quicksort(__global int *array, __global int *stack)

{
    const int chunk_size = get_global_id(0);

    int left = 0; // start index
    int right = chunk_size; // end index
    int top = -1; // top of stack
    int temp; // temp variable for swaping

    stack[++top] = left; // left pair  - > stack
    stack[++top] = right; // right pair  - > stack

    while (top >= 0) // Iterate until stack is empty
    {
        //Get subarray using the first left:right pair
        right = stack[top--]; 
        left = stack[top--];

        int pivot = array[right]; // set pivot as right element
        int idx = left; // element < pivot movr to left of index

        for (int i = left; i < right; i++) // iterate whilst < right, if element < = pivot move element before index
        {
            if (array[i] <= pivot)
            {
                temp = array[i];
                array[i] = array[idx];
                array[idx] = temp;
                idx++;
            }
        }
      
        // Swap procedure
        temp = array[idx];
        array[idx] = array[right];
        array[right] = temp;

        if (idx - 1 > left) // element index < pivot move to stack 
        {
            stack[++top] = left;
            stack[++top] = idx - 1;
        }

        if (idx + 1 < right) // element index > pivot move to stack 
        {
            stack[++top] = idx + 1;
            stack[++top] = right;
        }
    }
}