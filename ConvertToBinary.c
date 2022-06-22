#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>

/*
    For some reason numbers over 1,999,999,999 fail to produce output.
    
    Everything is shifted to the right by one bit.
    
    If you discover the reason for these or any other bugs please leave a comment.
*/

#define BYTE_LEN 8 // Modify this directive for alternate byte architectures.

#define SIGNED_FLAG 0x0001
#define EDIAN_FLAG 0x0010

unsigned char GEN_FLAG = 0x0000;

short isNumber(const char val)
{
    if(val > '9' || val < '0') return 0;
    return 1;
}

short isNumeric(const char* string, const size_t length) 
{
    if(strlen(string) == 0 || (string[0] == '-' && strlen(string) == 1)) return 0;
    
    for(short i = (string[0] == '-') ? 1 : 0; i < length; i++) if(!isNumber(string[i])) return 0;
    
    return 1;
}

// Uses goto to avoid spighettification.
short isAcceptableInput(const char* string)
{
    if(!isNumeric(string, strlen(string))) return 0;

    char* int_max = malloc((sizeof(char)) * 32);
    snprintf(int_max, 32, "%d", INT_MAX);
    
    
    if(strlen(int_max) < strlen(string)) goto FAIL;
    if(strlen(int_max) > strlen(string)) goto SUCCESS;
    
    for(short i = 0; i < strlen(string); i++)
    {
        if(string[i] > int_max[i]) goto FAIL;
        if(string[i] < int_max[i]) goto SUCCESS;
    }
    
    SUCCESS:
    free(int_max);
    return 1;
    
    FAIL:
    free(int_max);
    return 0;
}

short isSigned() {return GEN_FLAG & SIGNED_FLAG;}

short isLittleEdian() {return GEN_FLAG & EDIAN_FLAG;}

// Sets 'string' to 'length' 0s separated by spaces.
void zero(char* string, const size_t length) {for(short i = 0; i < length; i++) string[i] = (!(i % (BYTE_LEN + 1))) ? ' ' : '0';}

// Sets the most significant bit to '1'.
void flipSignBit(char* string, const size_t length) 
{
    if(!isLittleEdian()) string[1] = '1';
    else string[length] = '1';
}

void absolute(int* numb) {*numb *= -1;}

void negative(char* string, int* numb)
{
    if(numb <= 0) return;
    
    GEN_FLAG ^= SIGNED_FLAG;
    
    flipSignBit(string, strlen(string));
    absolute(numb);
}

// Simulates a single left bitshift; any optimization seems to cause undefined behavior.
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

void bitshiftRight(char* string, const char add, const size_t length)
{
    for(short i = (length - 1); i > 0; i--)
    {
        if(((i % (BYTE_LEN + 1)) == 0))
        {
            string[i + 1] = string[i - 1];
            i--;
            continue;
        }
        string[i + 1] = string[i];
    }
    string[1] = add;
}

void bitshift(char* string, const char add, const size_t length)
{
    if(isLittleEdian()) bitshiftRight(string, add, length);
    else bitshiftLeft(string, add, length);
}

// Simulates the bitwise 'or' operator
void bitwiseOr(char* string_one, const char* string_two, const size_t length) {for(short i = 0; i < length; i++) if(string_two[i] == '1') string_one[i] = '1';}

char* getBinaryRep(int numb)
{
    const size_t length = ((sizeof(int) * BYTE_LEN) + ((isLittleEdian()) ? 3 : 4));
    char* string = malloc(length);
    
    zero(string, length);
    
    if(numb < 0) negative(string, &numb);
    
    if(numb == 0 || numb == -0) return string;
    
    bitshift(string, '1', length);
    if(numb == 1 || numb == -1) return string;
    
    short complete = (numb == 0) ? 1 : 0;
    int remaining = numb, multiple = 1;
    while(complete != 1)
    {
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
    
    char* remaining_str = getBinaryRep(remaining);
    
    bitwiseOr(string, remaining_str, length);
    
    free(remaining_str);
    
    if(isSigned()) flipSignBit(string, length);
    
    return string;
}

void edianness(char* string, const size_t length)
{
    if(length < 1) return;
    if(string[0] != 'L') return;
    
    GEN_FLAG ^= EDIAN_FLAG;
    
    for(short i = 0; i < length; i++) string[i] = 0;
    
    fgets(string, 32, stdin);
}

int main(void) 
{
    char* input = malloc((sizeof(char)) * 32);
    
    printf("For little-edian format please make your first input an 'L' character.\nIf a negative value is supplied a signed representation will be supplied; otherwise unsigned.\n\n");
    
    fgets(input, 32, stdin);
    edianness(input, strlen(input));
    
    if(!isAcceptableInput(input))
    {
        printf("Error: Unacceptable input: %s.", input);
        return 1;
    }
    
    int input_num = atoi(input);
    free(input);
    
    char* binary = getBinaryRep(input_num);

    printf("%d=\n%s", input_num, binary);
    
    free(binary);
    return 0;
}
