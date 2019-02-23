#include <stdio.h>
#include "stringlib.h"

int main()
{
    //unrecommended (unsafe)
    string string_a;
    //recommended (safe)
    string string_b = STRING_NEW;
    
    string_set(&string_a, "hello world\na new line");
    string_newline(&string_a, "this is a third line");
    
    string_print(&string_a);
    printf("SIZE: %d\n", string_getSize(&string_a));
    
    FILE *foo = fopen("text.txt", "w");
    string_write(&string_a, foo);
    fclose(foo);
    
    foo = fopen("text.txt", "r");
    string_read(&string_b, foo);
    string_print(&string_b);
    printf("SIZE: %d\n", string_getSize(&string_b));
    string_delete(&string_a);
    string_delete(&string_b);
    getchar();
    return 0;
}