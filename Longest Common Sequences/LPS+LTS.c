
/*******************************************************************************
    Collaborating classmates: NONE
    Other collaborators: NONE

    Sources listed at end of code.

    NOTE:
    The LPS works for lengths up to 10000, and gives proper output.
    The LTS also returns a correct output, however it can be somewhat slow.
    Given enough time, it will find a correct solution.
*******************************************************************************/

#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <stdlib.h>

/*******************************************************************************
*                                  PROTOTYPES                                  *
*******************************************************************************/

int checkInt(char checkThisArray[]); // Checks if an array contains any non-digit characters
int findLength(char check[]); // Returns the length of a given array
void convertArray(char old_array[], int *new_array, int length); // Converts a char array to an int array
int max(int a, int b); // Finds the max value of two numbers
void findPalindrome(int array[], int length); // Finds the longest palindrome sequence
void findTandem(int input_array[], int length); // Finds the longest tandem sequence

/*******************************************************************************
*                                   CHECK INT                                  *
*******************************************************************************/

int checkInt( char checkThisArray[] ){

    int len = strlen(checkThisArray);

    while( len > 0 && isspace(checkThisArray[len - 1]) ){len--;}
       if (len > 0)
    for (int i = 0; i < len; ++i){
        if ( !isdigit(checkThisArray[i]) || isspace(checkThisArray[i]) ){
            printf("Error, non-digit character detected!\n\n");
            return 0;}
    }
}

/*******************************************************************************
*                                  FIND LENGTH                                 *
*******************************************************************************/

int findLength(char *array){

    int count = 0;
    do{count++;}
    while(array[count]!='\n');

    return count;
}

/*******************************************************************************
*                                CONVERT ARRAY                                 *
*******************************************************************************/

void convertArray(char old_array[], int *new_array, int length){

    for ( int i=0 ; i<length ; i++ ){
        int temp = old_array[i] - '0';
        new_array[i] =  temp;
    }
}

/*******************************************************************************
*                                     MAX                                      *
*******************************************************************************/

int max(int above, int left){
    if (above > left)
        return above;
    else
        return left;
}

/*******************************************************************************
*                                FIND PALINDROME                               *
*******************************************************************************/

void findPalindrome(int array[], int length){

    // Dynamically allocate a 2D array (table) to calculate the LPS
    int **table;
    table = malloc( sizeof(int*) * length );
    for(int i = 0; i < length; i++){
		table[i] = malloc(length * sizeof(int));
    }

    // Allocate a table to calculate the LPS
    // Initialize all values to zero, and the main diagonal to ones
    for ( int y=0 ; y<length ; y++ ){
        for ( int x=0 ; x<length ; x++ )
        table[y][x] = 0;
    }
    for ( int x=0 ; x<length ; x++ ){
        table[x][x]=1;
    }

    // Initialize required variables
    int x,y,z, compare1, compare2;

    // Only the top-half corner of the table is utilized
    // x and y represent the index of the integers we are comparing in the array
    // The main diagonal remains one (subsequences of length 1 have a LPS of length one)
    // When comparing lengths of two, if they match, input 2 in the table
    // If there is no match, the result is the max of the two "inner" subsequences
    // When comparing greater than two characters, if they match, it will be the max of the inner subsequences, plus two
    int count=0;
    for ( z=length-1 ; z >= 0 ; z-- ){
       for ( y=0, x=count ; y <= z ; y++, x++ ){
            if( x==y ){ table[y][x]=1; }
            else if ( array[x] != array[y] ){ compare1=table[y][x-1] ; compare2=table[y+1][x] ; table[y][x] = max(compare1,compare2);}
            else if ( y+1==x ){ table[y][x] = 2 ; }
            else { table[y][x] = table[y+1][x-1]+2 ; }
       }
       count++;
    }

    // The LPS length will be in the top right corner of the table
    int LPS_length = table[0][length-1];

    // Initialize an array that will hold the LPS sequence
    int *LPS_sequence =  malloc( sizeof(int) * LPS_length );
    for ( int i=0 ; i<LPS_length ; i++ ){
        LPS_sequence[i]=0;
    }

    // Trace back through the graph to obtain the LPS
    y = 0, x = length-1;
    int first=0, last = LPS_length-1;
    while ( x>0 && y<length-1 ){
        if( table[y][x] == table[y+1][x] ){y++;}
        else if ( table[y][x] == table[y][x-1] ){x--;}
        else{LPS_sequence[first]=array[y], LPS_sequence[last]=array[x], y++ , x--, first++, last-- ;}
    }

    // Output the result
    printf("\n\n# an LPS (length = %d) for the second sequence is:\n",LPS_length);
    for ( int i=0 ; i<LPS_length ; i++ ){
        printf("%d",LPS_sequence[i]);
    }
    printf("\n");

    free(table);
    free(LPS_sequence);

    // The following code can be used to print the table:
    /*
    for( int x=0 ; x<=length-1 ; x++ )
        {printf("\n");
        for( int y=0 ; y<=length-1 ; y++ )
            printf("%3d ",table[x][y]);}
    printf("\n");
    printf("\n\n");
    */
}

/*******************************************************************************
*                                  FIND TANDEM                                 *
*******************************************************************************/

void findTandem(int input_array[], int length){

    printf("\n\n");
    int LTS_length = 0;
    int *LCS =  malloc( sizeof(int) * 10050 );

    // Do this process for all possible sub-arrays of the passed in array
    for ( int i=1 ; i<length ; i++ ){

        // Dynamically create the first array and define its length
        int count = 0;
        int *array1 =  malloc( sizeof(int) * i );
        for( int x=0 ; x<i ; x++ ){
            array1[count] = input_array[x];
            count++;
        }
        int len1 = i;

        // Dynamically create the second array and define its length
        count = 0;
        int *array2 =  malloc( sizeof(int) * (length-i) );
        for ( int y=i ; y<length ; y++ ){
            array2[count] = input_array[y];
            count++;
        }
        int len2 = length-i;

        // The following code can be used to print the possible sub-arrays
        /*
        for( int m=0 ; m<len1 ; m++){
            printf("%d",array1[m]);
        }
        printf("+");
        for ( int m=0 ; m<len2 ; m++){
            printf("%d",array2[m]);
        }
        printf("\n");
        */

        // Dynamically allocate the two tables that will be used to find LCS
        int **table1;
        table1 = calloc( len1+1, sizeof(int*) );
        for(int i = 0; i < len1+1; i++){
            table1[i] = calloc(len2+1, sizeof(int));
        }
        int **table2;
        table2 = calloc( len1+1, sizeof(int*) );
        for(int i = 0; i < len1+1; i++){
            table2[i] = calloc(len2+1, sizeof(int));
        }

        // Fill the table using the method taught in class
        // If there is a match, that cell is equal to one plus the cell to the top-left
        // If no match, take the maximum value of either the cell above or below
        // Table2 is used to keep track of the traceback
        // The codes are: 1=match, 2=left, 3=up
        for( int x=1 ; x<=len1 ; x++ )
            for( int y=1 ; y<=len2 ; y++ )
            if ( array1[x-1]==array2[y-1] )
                {table1[x][y] = 1 + table1[x-1][y-1],
                table2[x][y] = 1;}
            else if ( table1[x-1][y] >= table1[x][y-1] ){
                table1[x][y] = table1[x-1][y],
                table2[x][y] = 2;
            }
            else
                table1[x][y] = table1[x][y-1],
                table2[x][y] = 3;

        // The following code can be used to print the table:
        /*
        for( int x=0 ; x<=len1 ; x++ )
            {printf("\n");
            for( int y=0 ; y<=len2 ; y++ )
                printf("%d ",table1[x][y]);}
        printf("\n");
        */

        // If a new maximum tandem is found, do the following:
        if(table1[len1][len2]*2 > LTS_length){

            // Define the new max length
            LTS_length = table1[len1][len2] * 2;

            // Traceback on table2, and store the (currently reversed) result in LCS
            free(LCS);
            int *LCS =  malloc( sizeof(int) * 10050 );
            int x = len1, y = len2, count=0;
            while ( x>0 && y>0 )
                {if (table2[x][y]==1)
                    (LCS[count]=array1[x-1]),x--,y--,count++;
                else if (table2[x][y] == 2)
                    x--;
                else
                    y--;}
        }
        // Clear the heap
        free(array1);
        free(array2);
        for (int i = 0; i < len1+1; i++){
            int* temp = table1[i];
            free(temp);
        }
        free(table1);
        for (int i = 0; i < len1+1; i++){
            int* temp = table2[i];
            free(temp);
        }
        free(table2);

    }

    // Prints the length and the sequence
    // Once the first half of the tandem is found, the second half can just be reprinted
    printf("# an LTS (length = %d) for the first sequence is:\n", LTS_length);
    for(int i=(LTS_length/2)-1 ; i>=0; i--){
        printf("%d", LCS[i]);}
    for(int i=(LTS_length/2)-1 ; i>=0; i--){
        printf("%d", LCS[i]);}



}

/*******************************************************************************
*                                     MAIN                                     *
*******************************************************************************/

int main(void){

    // Dynamically allocate arrays to hold user input
    char *i =  malloc( sizeof(char) * 10050 );
    char *j =  malloc( sizeof(char) * 10050 );

    // Get user input and store it in the created arrays
    input:
    printf("enter the first sequence: "); fgets(i, 10050, stdin);
    printf("enter the second sequence: "); fgets(j, 10050, stdin);

    // Check if user entered non-digit characters. If so, re-ask for input
    if(!checkInt(i) || !checkInt(j)){goto input;}

    // Find the length of each array
    int len1 = findLength(i);
    int len2 = findLength(j);


    // Dynamically allocate arrays that are the size of the number of inputted integers
    int *array1 = malloc( sizeof(int) * len1 );
    int *array2 = malloc( sizeof(int) * len2 );

    // This function gives us arrays, of the proper length, containing integers (not char)
    convertArray(i, array1, len1);
    convertArray(j, array2, len2);

    // Free the unneeded char array
    free(i);
    free(j);

    // Print the lengths of the inputted sequences. Print the inputted sequences.
    printf("\nTwo input sequences (length = %d, %d) are:\n", len1, len2);
    for ( int i=0 ; i<len1 ; i++ ){
        printf("%d", array1[i]);
    }
    printf("\n");
     for ( int i=0 ; i<len2 ; i++ ){
        printf("%d", array2[i]);
    }

    // Find the tandem sequence for the first array
    // Find the palindrome sequence for the second array
    findTandem(array1, len1);
    findPalindrome(array2, len2);

    return 0;

}

/*******************************************************************************
*                               SOURCES CONSULTED                              *
*******************************************************************************/
/*
https://www.youtube.com/watch?v=_nCsPn7_OgI
https://www.eskimo.com/~scs/cclass/int/sx9b.html
https://zhu45.org/posts/2017/Jan/08/modify-array-inside-function-in-c/
https://stackoverflow.com/questions/628761/convert-a-character-digit-to-the-corresponding-integer-in-c
https://www.geeksforgeeks.org/dynamically-allocate-2d-array-c/
https://link.springer.com/chapter/10.1007/978-3-540-30213-1_13
https://stackoverflow.com/questions/5666214/how-to-free-c-2d-array

Utilized my previous assignment to find LCS, and subsequently the LTS.
*/
