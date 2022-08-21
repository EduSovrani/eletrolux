/**
* @file <main.c>
* @brief <Source File>
*
* <Question 3 - Eletrolux - Knowledge Assessment>
*
* @author <Eduardo Falchetti Sovrani>
*
*/

#include <stdio.h>
#include "main.h"

// -----------------------
// DEFINES
// -----------------------
#define INPUT_SIZE (40)

// -----------------------
// VARIABLES DECLARATION
// -----------------------

// INPUT VECTOR
int INPUT_DATA[INPUT_SIZE] = {
    34,  201, 190, 154, 8,   194, 2,   6,
    114, 88,  45,  76,  123, 87,  25,  23,
    200, 122, 150, 90,  92,  87,  177, 244,
    201, 6,   12,  60,  8,   2,   5,   67,
    7,   87,  250, 230, 99,  3,   100, 90
};

// OUTPUT VECTOR
int OUTPUT_DATA[INPUT_SIZE];
// OUTPUT VECTOR SIZE
unsigned int OUTPUT_SIZE = 0;

// -----------------------
// MAIN FUNCTION
// -----------------------
int main()
{
    float RESULT[3];

    //Call the function to calculate the statistics
    find_statistics(&INPUT_DATA[0], INPUT_SIZE, &RESULT[0], &OUTPUT_SIZE, &OUTPUT_DATA[0]);

    //Plot Input Vector
    printf("\n");
    printf("SIZE OF INPUT VECTOR = %d\n", INPUT_SIZE);
    printf("INPUT VECTOR = \n");
    for(int i = 0; i < INPUT_SIZE; i++){
       printf("%5d ", INPUT_DATA[i]);
       if( ((i+1) % 10) == 0 ){
          printf("\n", INPUT_DATA[i]);
       }
    }
    printf("\n");

    //Plot Statistics
    printf("STATISTICS \n");
    printf("Average Value: %f \nMaximum Value: %f \nMinimum Value: %f \n", RESULT[0], RESULT[1], RESULT[2]);
    printf("\n");

    //Plot Output vector
    printf("SIZE OF OUTPUT VECTOR = %d\n", OUTPUT_SIZE);
    printf("OUTPUT EVEN VECTOR = \n");
    for(int i = 0; i < OUTPUT_SIZE; i++){
       printf("%5d ", OUTPUT_DATA[i]);
       if( ((i+1) % 10) == 0 ){
          printf("\n", OUTPUT_DATA[i]);
       }
    }
    printf("\n");
    getchar();

    return 0;
}

// -----------------------
// FUNCTIONS
// -----------------------

/**
 * @brief Function that find statistics from an input integer vector
 *        - Average, Maximum, Minimum
 *        - Sort a new vector only with even values
 *
 * @param _array - Pointer to Input Array.
 * @param _size - Size of Input array
 * @param _result - Pointer to a output float vector with size 3 (Average, Maximum, Minimum)
 * @param _new_size - Pointer to store the size of the output event vector
 * @param _result_even - Pointer to output integer even vector
 *
 * @return void.
 */
void find_statistics(int *_array, unsigned int _size, float *_result, int *_new_size, int *_result_even){
   float aux_avg = 0, aux_max = _array[0], aux_min = _array[0];

   *_new_size = 0;

   for(int i = 0; i < _size; i++){
      // sum all array elements to calculate the average value
      aux_avg += _array[i];

      // Search the maximum value
      if(_array[i] >= aux_max){
        aux_max = _array[i];
      }

      // Search the minimum value
      if(_array[i] <= aux_min){
        aux_min = _array[i];
      }

      //Check Event Values
      _result_even[i] = 0;
      if( (_array[i] % 2) == 0){
         _result_even[*_new_size] = _array[i];
         *_new_size = *_new_size + 1;
      }
   }
   // calculate the average value
   aux_avg = aux_avg/(float)_size;

   // return the values
   _result[0] = aux_avg;
   _result[1] = aux_max;
   _result[2] = aux_min;
}
