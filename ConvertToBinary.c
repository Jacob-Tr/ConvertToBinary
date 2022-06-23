#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>

/*
    For some reason numbers over 1,999,999,999 fail to produce output.
    
    Everything is shifted to the right by one bit (still producing accurate output).
    
    If you discover the reason for these or any other bugs please leave a comment.
    
    Does not support floating point numbers.
*/

#define BYTE_LEN 8 // Modify this directive for alternate byte architectures.

// Static flag values
#define SIGNED_FLAG 0x0001
#define EDIAN_FLAG 0x0010

// Dynamic flag setter
unsigned char GEN_FLAG = 0x0000;

// Character value of 'val' must be between that of '0' and '9'.
short isNumber(const char val) {return (val < '9' || val > '0');}

short isNumeric(const char* string, const size_t length) 
{
    // Check for empty string or just negative sign.
    if(strlen(string) == 0 || (string[0] == '-' && strlen(string) == 1)) return 0;
    
    // Ensure each character is numeric.
    for(short i = (string[0] == '-') ? 1 : 0; i < length; i++) if(!isNumber(string[i])) return 0;
    
    return 1;
}

// Properly uses 'goto' to avoid spighettification.
short isAcceptableInput(const char* string)
{
    if(!isNumeric(string, strlen(string))) return 0;

    // Allocate a string and set it to maximum integer value.
    char* int_max = malloc((sizeof(char)) * 32);
    snprintf(int_max, 32, "%d", INT_MAX);
    
    // Check length of string against max int length. If it's longer it's larger; if shorter it's smaller.
    if(strlen(int_max) < strlen(string)) goto FAIL;
    if(strlen(int_max) > strlen(string)) goto SUCCESS;
    
    // Compare character values of each digit. If a smaller char value is encountered the number is smaller; likewise if char value is higher then that of int max the number is invalid.
    for(short i = 0; i < strlen(string); i++)
    {
        if(string[i] > int_max[i]) goto FAIL;
        if(string[i] < int_max[i]) goto SUCCESS;
    }
    
    // Using 'goto' to ensure 'int_max' string has been freed back to the heap.
    SUCCESS:
    free(int_max);
    return 1;
    
    FAIL:
    free(int_max);
    return 0;
}

// Check sign bit of general flag.
short isSigned() {return GEN_FLAG & SIGNED_FLAG;}

// Check edian bit of general flag
short isLittleEdian() {return GEN_FLAG & EDIAN_FLAG;}

// Sets 'string' to 'length' 0s separated by spaces.
void zero(char* string, const size_t length) {for(short i = 0; i < length; i++) string[i] = (!(i % (BYTE_LEN + 1))) ? ' ' : '0';}

// Sets the most significant bit to '1'.
void flipSignBit(char* string, const size_t length) 
{
    if(!isLittleEdian()) string[1] = '1';
    else string[length] = '1';
}

// Multiply by negative one; negative by negative is positive.
void absolute(int* numb) {*numb *= -1;}

void negative(char* string, int* numb)
{
    if(*numb >= 0) return;
    
    // Set 'signed bit' section of general flag.
    GEN_FLAG ^= SIGNED_FLAG;
    
    // Flip the signed bit and set 'numb' to usable positive value.
    flipSignBit(string, strlen(string));
    absolute(numb);
}

// Simulates a single left bitshift; see comments on 'bitshiftRight()'
void bitshiftLeft(char* string, const char add, const size_t length)  
{
    for(short i = 2; i < length; i++) 
    {
         if(((i % (BYTE_LEN + 1)) == 0))
        {
            string[i - 1] = string[i + 1];
            i++;
            continue;
        }
        string[i - 1] = string[i];
    }
    string[length - 1] = add;
}

// Simulates right bitshift
void bitshiftRight(char* string, const char add, const size_t length)
{
    for(short i = (length - 1); i > 0; i--)
    {
        // Account for spaces between bytes.
        if(((i % (BYTE_LEN + 1)) == 0))
        {
            string[i + 1] = string[i - 1];
            i--;
            continue;
        }
        // Move bit right one space.
        string[i + 1] = string[i];
    }
    // Assign new low-order bit.
    string[1] = add;
}

// Determine bitshift based on edianness.
void bitshift(char* string, const char add, const size_t length)
{
    if(isLittleEdian()) bitshiftRight(string, add, length);
    else bitshiftLeft(string, add, length);
}

// Simulates the bitwise 'or'(^) operator by setting any '1' bits in string_one to match those in string_two while preserving those already in string_one.
void bitwiseOr(char* string_one, const char* string_two, const size_t length) {for(short i = 0; i < length; i++) if(string_two[i] == '1') string_one[i] = '1';}

// Convert 'numb' to is 32-bit binary representation.
char* getBinaryRep(int numb)
{
    // Determine length needed and allocate solution string to that length.
    const size_t length = ((sizeof(int) * BYTE_LEN) + ((isLittleEdian()) ? 3 : 4));
    char* string = malloc(length);
    
    // Initialize output memory to zeros.
    zero(string, length);
    
    // Determine negative values.
    if(numb < 0) negative(string, &numb);
    
    // The output is currently set to zero.
    if(numb == 0 || numb == -0) return string;
    
    // Start with one; multiplying by zero will get nowhere. If one is target value return output.
    bitshift(string, '1', length);
    if(numb == 1 || numb == -1) return string;
    
    // Create 'complete' to use as boolean check. 'Remaining' to store what's left to calculate. 'multiple' is a check to ensure we don't overflow.
    short complete = 0;
    int remaining = numb, multiple = 1;
    
    while(complete != 1)
    {
        // Bitshifting once multiplies output by two. Keep doing that until the result would be more then what is left to calculate; once that is true set 'remaining' to what's left and exit loop.
        if((multiple * 2) <= numb)
        {
            multiple *= 2;
            bitshift(string, '0', length);
        }
        else
        {
            remaining = numb - multiple;
            complete = 1;
        }
    }
    
    // Recursively run this method until every bit necessary is set. A subsequent call will not set a higher-order bit then it's previous.
    char* remaining_str = getBinaryRep(remaining);
    
    // Add set bits to previous call's set then free the memory address allocated by previous call's 'string' variable.
    bitwiseOr(string, remaining_str, length);
    free(remaining_str);
    
    // Ensure sign bit is still set and return value.
    if(isSigned()) flipSignBit(string, length);
    return string;
}

// Check and set up program to handle edianness.
void edianness(char* string, const size_t length)
{
    // If length is less then '1' or the input was not 'L' return to main as big edian
    if(length < 1) return;
    if(string[0] != 'L') return;
    
    GEN_FLAG ^= EDIAN_FLAG; // Set edian section of general program flag.
    
    zero(string, length); // Wipe input memory.
    
    fgets(string, 32, stdin); // Collect value from stdin provided after request of little-edianness.
}

int main(void) 
{
    // Allocate 32 characters of memory to input variable; we should never need more.
    char* input = malloc((sizeof(char)) * 32);
    
    printf("For little-edian format please make your first input an 'L' character.\nIf a negative value is supplied a signed representation will be supplied; otherwise unsigned.\n\n");
    
    // Take input of at most 32 chars and check if little-edianness has been requested by the user.
    fgets(input, 32, stdin);
    edianness(input, strlen(input));
    
    // Ensure 'input' contains a valid int_32.
    if(!isAcceptableInput(input))
    {
        printf("Error: Unacceptable input: %s.", input);
        return 1;
    }
    
    // Input has been sanitized; atoi() call should be safe. Then free 'input' memory back to heap.
    int input_num = atoi(input);
    free(input);
    
    // Get the representation and print to stdout.
    char* binary = getBinaryRep(input_num);
    printf("%d=\n%s", input_num, binary);
    
    // Free memory allocated to 'string' in 'getBinaryRep()' back to the heap.
    free(binary);
    return 0; // And we're done!
}
