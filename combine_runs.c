#include <stdio.h>
#include <string.h>
#include <stdlib.h>

// Very customized function, would return the last three unsigned long from a char*

// From net: https://stackoverflow.com/questions/784417/reversing-a-string-in-c
void inplace_reverse(char * str)
{
  if (str)
  {
    char * end = str + strlen(str) - 1;

    // swap the values in the two given variables
    // XXX: fails when a and b refer to same memory location
#   define XOR_SWAP(a,b) do\
    {\
      a ^= b;\
      b ^= a;\
      a ^= b;\
    } while (0)

    // walk inwards from both ends of the string, 
    // swapping until we get to the middle
    while (str < end)
    {
      XOR_SWAP(*str, *end);
      str++;
      end--;
    }
#   undef XOR_SWAP
  }
}

int extract_nums (char* ori_str, unsigned long* num1, unsigned long* num2, unsigned long* num3) {

    int ori_len = strlen(ori_str);
    char num_rev_arr[3][15];
    int comma_num = 0;
    int i = 0;
    int j = strlen(ori_str) -1;
    for (; j >=0, comma_num < 3; j--) {
        if (ori_str[j] == ',') {
            num_rev_arr[comma_num][i] = '\0';
            i = 0;
            comma_num++;
        } else {
            num_rev_arr[comma_num][i] = ori_str[j];
            i++;
        }
        
    }

    inplace_reverse(num_rev_arr[0]);
    inplace_reverse(num_rev_arr[1]);
    inplace_reverse(num_rev_arr[2]);
    
    
    *num1 = strtoul(num_rev_arr[2],NULL,10);
    *num2 = strtoul(num_rev_arr[1],NULL,10);
    *num3 = strtoul(num_rev_arr[0],NULL,10);
    return j + 1;

}

void main_func(char* file1_str, char* file2_str, char* outfile_str) {
    FILE* file1 = fopen(file1_str, "r");
    FILE* file2 = fopen(file2_str, "r");
    FILE* outfile = fopen(outfile_str, "w");

    char* line1 = (char* ) malloc(1024 * sizeof(char));
    char* line2 = (char* ) malloc(1024 * sizeof(char));

    int line_count = 0;
    size_t line1_size = 1024;
    size_t line2_size = 1024;

    char head_line[1024];
    while (fgets(line1, line1_size, file1)) {
        fgets(line2, line2_size, file2);
        // Remove the newline chars
        int len1 = strlen(line1);
        if (line1[len1 - 1] == '\n') {
            line1[len1 - 1] = '\0';
        }
       
        int len2 = strlen(line2);
        if (line2[len2 - 1] == '\n') {
            line2[len2 - 1] = '\0';
        }

        if (line_count == 0) {
            fprintf(outfile, "%s\n", line1);                
        } else {
            unsigned long f1_num1 = 0;
            unsigned long f1_num2 = 0;
            unsigned long f1_num3 = 0;
            unsigned long f2_num1 = 0;
            unsigned long f2_num2 = 0;
            unsigned long f2_num3 = 0;
           
            int left1_pos = extract_nums (line1, &f1_num1, &f1_num2, &f1_num3);
            int left2_pos = extract_nums (line2, &f2_num1, &f2_num2, &f2_num3);
            unsigned long num1 = f1_num1 + f2_num1; 
            unsigned long num2 = f1_num2 + f2_num2; 
            unsigned long num3 = f1_num3 + f2_num3; 
            char left_arr[left1_pos + 1];
            memcpy(left_arr, line1, left1_pos);
            left_arr[left1_pos] = '\0';
            char final_word[1024];
            sprintf(final_word, "%s,%lu,%lu,%lu", left_arr, num1, num2, num3);
            fprintf(outfile, "%s\n", final_word);
        }
        
        line_count++;
    }

    free(line1);
    free(line2);

    fclose(file1);
    fclose(file2);
    fclose(outfile);
    
}

int main(int argc, char** argv) {

    char* file1_str = argv[1];
    char* file2_str = argv[2];
    char* outfile_str = argv[3];
    main_func(file1_str, file2_str, outfile_str);

}

