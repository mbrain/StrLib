#ifndef _STRINGLIB_H_
#define _STRINGLIB_H_

#include <stdint.h>
#include <stdio.h>

/*+------------+*\
 ||  OPTIONS   ||
\++------------+*/

//define _string as string (0=never, 1=if not defined, 2=redefine)
#define _STRINGLIB_DEFINE_ 1
//overload functions with generic selection (0=disabled, 1=compatibility check, 2=force enabled)
#define _STRINGLIB_OVERLOADING_ 1
//enables locks and thread safe functions (0=disabled+performance, 1=compatibility check, 2=force enabled)
#define _STRINGLIB_THREAD_SAFE_ 1
//buffers whole string before writing to file (0=never, 1=thread only, 2=always)
#define _STRINGLIB_IO_WRITE_COPY_ 1
//buffer size for read operations (BUFSIZ=default value)
#define _STRINGLIB_IO_READ_BUFSIZE_ BUFSIZ
//max fail count for memory allocation (0 means infinite tries - not recommended) (should never fail)
#define _STRINGLIB_MAX_ALLOC_FAILS_ 50


/*+------------+*\
 ||  TYPEDEF   ||
\++------------+*/

//string type (you can print it as a string)
typedef struct
{
    uintptr_t:8*sizeof(uintptr_t);
    size_t:8*sizeof(size_t);
    size_t:8*sizeof(size_t);
    uintptr_t:8*sizeof(uintptr_t);
    uintptr_t:8*sizeof(uintptr_t);
    #if (_STRINGLIB_THREAD_SAFE_ && !defined(__STDC_NO_ATOMICS__) && ((__STDC__==1 && __STDC_VERSION__>=201112L) || ((__GNUC__*10000+__GNUC_MINOR__*100+__GNUC_PATCHLEVEL__)>=40900) || ((__clang_major__*10000+__clang_minor__*100+__clang_patchlevel__)>=30800) || (__xlC__>=0x1301))) || _STRINGLIB_THREAD_SAFE_==2
    size_t:8*sizeof(size_t);
    _Atomic volatile struct
    {
        volatile uintptr_t:8*sizeof(volatile uintptr_t);
        volatile uintptr_t:8*sizeof(volatile uintptr_t);
    };
    #else
    #undef _STRINGLIB_THREAD_SAFE_
    #define _STRINGLIB_THREAD_SAFE_ 0
    #endif
}_string;

//assigning to STRING_NEW before passing a string to the library functions is the safest approach
//WARNING: do not assign to STRING_NEW after passing the string to library functions
#define STRING_NEW ((_string){})

//The 'string' type macro is defined (if not already defined)
#if (!defined(string) && _STRINGLIB_DEFINE_) || _STRINGLIB_DEFINE_==2
#undef string
#define string _string
#endif // string


/*+--------------+*\
 ||  FUNCTIONS   ||
\++--------------+*/

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

//Locks the string to other threads, locks can be nested
void string_lock(_string *string_a);
//Locks write access of the string to other threads, W-locks can be nested and acquired by multiple threads
///void string_lockWrite(_string *string_a);
//Unlocks string for the other threads or unlocks a level
void string_unlock(_string *string_a);
//Locks multiple strings safely, nested locks after a multi-lock can be singular locks
void string_multilock(int string_count, ...);
//returns string
const char *const string_getString(_string *string_a);
//checks if string is allocated
int string_isAllocated(_string *string_a);
//checks if string_a is identical to string_v
int string_equals(_string *string_a, const void *string_v);
//checks if string_a contains all characters (checkall=1/true) or one of the characters (checkall=0/false)
int string_containsChars(_string *string_a, int checkall, ...);
//checks if string_a contains string_b, returns position+1
size_t string_contains(_string *string_a, const char *string_b, ...);
//returns string size in bytes
size_t string_getSize(_string *string_a);
//initializes a string to size 1, no need to use this
size_t string_init(_string *string_a);
//writes a string to a text file, add extra parameter 0 or false to avoid writing a new line at the end
size_t string_write(_string *string_a, FILE *file_a, ...);
//writes a string to a binary file
size_t string_writeBin(_string *string_a, FILE *file_a);
//reads a string from a text file, add extra parameter 0 or false to ignore multi-line string
size_t string_read(_string *string_a, FILE *file_a, ...);
//reads a string from a text file with a custom buffer or known size; add extra parameter 0 or false to ignore multi-line string
size_t string_readBuffered(_string *string_a, FILE *file_a, size_t buffersize, ...);
//reads a string from a text file and appends it to string_a, add extra parameter 0 or false to ignore multi-line string
size_t string_readAppend(_string *string_a, FILE *file_a, ...);
//appends a string from a text file with a custom buffer or known size; add extra parameter 0 or false to ignore multi-line string
size_t string_readAppendBuffered(_string *string_a, FILE *file_a, size_t buffersize, ...);
//reads a string from a binary file
size_t string_readBin(_string *string_a, FILE *file_a);
//scans a string from user input, returns string size
size_t string_scan(_string *string_a);
//appends the user input to a string
size_t string_scanAppend(_string *string_a);
//appends string_b to the end of string_a, string_b must be a pointer to char or a pointer to string
size_t string_append(_string *string_a, const void *string_v);
//appends a string to a specific point of another string (0 if omitted)
size_t string_appendPos(_string *string_a, const void *string_v, ...);
//appends a string multiple times to to a specific position of another string (0 if omitted)
size_t string_appendCiclePos(_string *string_a, const void *string_v, unsigned int repeatTimes, ...);
//appends the user input to a specific point of a string
size_t string_scanAppendPos(_string *string_a, ...);
//sets string_a to string_b
size_t string_set(_string *string_a, const void *string_b);
//starts a new line, can append a string if second argument is string* or char* or scan new line if second argument is stdin
size_t string_newline(_string *string_a, ...);
//cuts a string from pos (initial position) with a certain size (if specified) or until the end (if not specified or 0)
size_t string_cut(_string *string_a, size_t pos, ...);
//overrides string_a with string_b from specified position
size_t string_override(_string *string_a, const void* string_v, ...);
//swaps pointed strings
void string_swap(_string *string_a, _string *string_b);
//deletes a string
void string_delete(_string *string_a);
//prints a string, add argument 0 (or false if using stdbool) to print without new line
void string_print(_string *string_a, ...);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // _STRINGLIB_H_


/*+----------------+*\
 ||  OVERLOADING   ||
\++----------------+*/

#if !_STRINGLIB_THREAD_SAFE_
#define string_lock(S) ((void)S)
#define string_unlock(S) ((void)S)
#define string_multilock(N, S...) ((void)N)
#endif // _STRINGLIB_THREAD_SAFE_


//FUNCTION OVERLOADING
#ifndef _STRINGLIB_OVERLOADING_WAIT_

//C11 support check
#if (_STRINGLIB_OVERLOADING_ && ((__STDC__==1 && __STDC_VERSION__>=201112L) || ((__GNUC__*10000+__GNUC_MINOR__*100+__GNUC_PATCHLEVEL__)>=40900) || ((__clang_major__*10000+__clang_minor__*100+__clang_patchlevel__)>=30000) || (__xlC__>=0x1201))) || _STRINGLIB_OVERLOADING_==2

//Overloading repeating check
#if !defined(_STRINGLIB_OVERLOADING_DONE_) || !defined(string_set)

//Macro defined when overloading support is present
#define _STRINGLIB_OVERLOADING_DONE_

//Overloading of string_scanAppendPos function for (string*) (string*, int)
#define string_scanAppendPos(A, B...)\
    _Generic(1?B:_Generic((void*)B+0, default:0),\
    int: (string_appendPos)((_string*)A, stdin, B+0),\
    default: (string_appendPos)((_string*)A, stdin, B+0))

//Overloading of string_appendPos function for (string*, string*/char*/stdin) and (string*, string*/char*/stdin, int)
#define string_appendPos(A, B, C...)\
    _Generic(1?B:_Generic((void*)B+0, default:0),\
    FILE*: ((FILE*)B+0 == stdin)?(string_appendPos)((_string*)A, (FILE*)B+0, _Generic(1?C:_Generic((void*)C+0,default:0),int:C+0,default:0)):(string_append)((_string*)A, ""),\
    char*: (string_appendPos)((_string*)A, (char*)B+0, C+0),\
    _string*: (string_isAllocated)((_string*)B+0)?(string_appendPos)((_string*)A, ((string_getString)((_string*)((void*)B+0)))+0, C+0):(string_appendPos)((_string*)A, "", C+0),\
    default: (string_append)((_string*)A, ""))

//Overloading of string_appendCiclePos function for (string*, string*/char*/stdin, int) and (string*, string*/char*/stdin, int, int)
#define string_appendCiclePos(A, B, C, D...)\
    _Generic(1?B:_Generic((void*)B+0, default:0),\
    FILE*: ((FILE*)B+0 == stdin)?(string_appendCiclePos)((_string*)A, (FILE*)B+0, (int)C+0, _Generic(1?D:_Generic((void*)D+0,default:0),int:D+0,default:0)):(string_append)((_string*)A, ""),\
    char*: (string_appendCiclePos)((_string*)A, (char*)B+0, (int)C+0, D+0),\
    _string*: (string_isAllocated)((_string*)B+0)?(string_appendCiclePos)((_string*)A, ((string_getString)((_string*)((void*)B+0)))+0, (int)C+0, D+0):(string_appendCiclePos)((_string*)A, "", (int)C+0, D+0),\
    default: (string_append)((_string*)A, ""))

//Overloading of string_append function for (string*, char*) or (string*, string*)
#define string_append(A, B)\
    _Generic(B,\
    char*: (string_append)((_string*)A, (char*)B),\
    FILE*: ((FILE*)B+0 == stdin)?(string_scanAppend)((_string*)A):(string_append)((_string*)A, ""),\
    _string*: (string_isAllocated)((_string*)B)?(string_append)((_string*)A, ((string_getString)((_string*)((void*)B+0)))+0):(string_append)((_string*)A, ""),\
    default: (string_append)((_string*)A, ""))

//Overloading of string_set function for (string*, char*) or (string*, string*)
#define string_set(A, B)\
    _Generic(B,\
    char*: (string_set)((_string*)A, (char*)B),\
    _string*: (string_isAllocated)((_string*)B)?(string_set)((_string*)A, ((string_getString)((_string*)((void*)B+0)))+0):(string_set)((_string*)A, ""),\
    default: (string_set)((_string*)A, ""))

//Overloading of string_newline function for (string*) or (string*, string*) or (string*, char*) or (string*, stdin)
#define string_newline(A, B...)\
    _Generic(1?B:_Generic((void*)B+0, default:0),\
    FILE*: ((FILE*)B+0 == stdin)?(string_newline)((_string*)A, (FILE*)B+0):(string_newline)((_string*)A, ""),\
    char*: (string_newline)((_string*)A, (char*)B+0),\
    _string*: (string_isAllocated)((_string*)B+0)?(string_newline)((_string*)A, ((string_getString)((_string*)((void*)B+0)))+0):(string_newline)((_string*)A, ""),\
    default: (string_newline)((_string*)A, ""))

//Overloading of string_cut function for (string*, int) and (string*, int, int)
#define string_cut(A, B, C...)\
    _Generic(1?C:_Generic((void*)C+0, default:0),\
    int: (string_cut)((_string*)A, (int)B+0, C+0),\
    default: (string_cut)((_string*)A, (int)B+0, 0))

//Overloading of string_contains function for (string*, char*), (string*, char) and (string*, string*)
#define string_contains(A, B, C...)\
    _Generic(B+0,\
    int: (string_contains)(A, ((char[]){(int)(B+0), '\0'}), (size_t)C+0),\
    char*: (string_contains)(A, (char*)B+0, (size_t)C+0),\
    char: (string_contains)(A, ((char[]){(int)(B+0), '\0'}), (size_t)C+0),\
    _string*: (string_contains)(A, ((string_getString)((_string*)((void*)B+0)))+0, (size_t)C+0),\
    default: (string_contains)(A, (char*)B+0, (size_t)C+0))

//Overloading of string_equals function for (string*, char*) and (string*, string*)
#define string_equals(A, B)\
    _Generic(B+0,\
    char*: (string_equals)(A, B+0),\
    _string*: (string_equals)(A, ((string_getString)((_string*)((void*)B+0)))+0),\
    default: 0)

//Overloading of string_override function for (string*, string*/char*) and (string*, string*/char*, int)
#define string_override(A, B, C...)\
    _Generic(1?B:_Generic((void*)B+0, default:0),\
    char*: (string_override)((_string*)A, (char*)B+0, C+0),\
    _string*: (string_isAllocated)((_string*)B+0)?(string_override)((_string*)A, ((string_getString)((_string*)((void*)B+0)))+0, C+0):(string_append)((_string*)A, ""),\
    default: (string_append)((_string*)A, ""))

#endif // Overloading repeating check
#else // C11 not supported

#ifdef _STRINGLIB_OVERLOADING_DONE_
#undef _STRINGLIB_OVERLOADING_DONE_
#endif // _STRINGLIB_OVERLOADING_DONE_

#if _STRINGLIB_OVERLOADING_
#undef _STRINGLIB_OVERLOADING_
#define _STRINGLIB_OVERLOADING_ 0
#endif // _STRINGLIB_OVERLOADING_

//Alternative function overloading (size-checking, no type-checking)
#define string_appendPos(A, B, C...) ((string_appendPos)(A, B, (size_t)C+0))
#define string_appendCiclePos(A, B, C, D...) ((string_appendCiclePos)(A, B, C, (size_t)C+0))
#define string_scanAppendPos(A, B...) ((string_scanAppendPos)(A, (size_t)B+0))
#define string_newline(A, B...) ((string_newline)(A, (sizeof((const char*[]){B}))?(const char*)B+0:""))
#define string_cut(A, B, C...) ((string_cut)(A, B, (size_t)C+0))
#define string_override(A, B, C...) ((string_override)(A, B, (size_t)C+0))
#define string_contains(A, B, C...) ((string_contains)(A, B, (size_t)C+0))

#endif // C11 support check

//Simple function overloading (size-checking, no type-checking)
#define string_write(A, B, C...) ((string_write)(A, B, (sizeof((int[]){C}))?C+0:1))
#define string_read(A, B, C...) ((string_read)(A, B, (sizeof((int[]){C}))?C+0:1))
#define string_readBuffered(A, B, C, D...) ((string_readBuffered)(A, B, C, (sizeof((int[]){D}))?D+0:1))
#define string_readAppend(A, B, C...) ((string_readAppend)(A, B, (sizeof((int[]){C}))?C+0:1))
#define string_readAppendBuffered(A, B, C, D...) ((string_readAppendBuffered)(A, B, C, (sizeof((int[]){D}))?D+0:1))
#define string_print(A, B...) ((string_print)(A, (sizeof((int[]){B}))?B+0:1))
#define string_containsChars(A, B, C...) ((string_containsChars)(A, B, (int)(sizeof((char[]){'\0', C})/sizeof(char)-1), C+0))

//Extra function definitions
#define string_free(STRING_PTR) string_delete(STRING_PTR)
#define string_search(A, B) string_contains(A, B)
#define string_find(A, B) string_contains(A, B)
#define string_searchChars(A, B, C...) string_containsChars(A, B, C...)
#define string_findChars(A, B, C...) string_containsChars(A, B, C...)

#endif // _STRINGLIB_OVERLOADING_WAIT_
