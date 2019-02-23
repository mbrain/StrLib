#define _STRINGLIB_OVERLOADING_WAIT_
#include "stringlib.h"
#undef _STRINGLIB_OVERLOADING_WAIT_

#include <stdlib.h>
#include <stdarg.h>

//you should never use these variables
#if _STRINGLIB_THREAD_SAFE_
#include <stdatomic.h>
_Thread_local static char _STRINGLIB_LOCAL_VAR_ = 0;
static atomic_flag _STRINGLIB_GLOBAL_VAR_ = ATOMIC_FLAG_INIT;
#else // _STRINGLIB_THREAD_SAFE_
static char _STRINGLIB_GLOBAL_VAR_ = 0;
#endif
static void *_STRINGLIB_HASTHREADS_ = NULL;

//assigned to stringAllocation and stringSignature to state string is allocated, used to check if string is allocated
#define _STRINGLIB_ALLOCATION_TRUE_ ((void*)(&_STRINGLIB_GLOBAL_VAR_))
//assigned when deallocating string, should NOT check if false, only if not true
#define _STRINGLIB_ALLOCATION_FALSE_ NULL;
//library ID for current thread
#define _STRINGLIB_THREAD_ID_ ((uintptr_t)&_STRINGLIB_LOCAL_VAR_)


#define _STRINGLIB_ACCESS_STRING_(STRING) (*(*((char***)((_string*[]){(&(STRING))}))))
#ifdef UINT8_MAX // 8 bit integer exists
#define _STRINGLIB_ACCESS_SIZE_(STRING) (*((size_t*)&(((uint8_t*)(&(STRING)))[sizeof(uintptr_t)])))
#define _STRINGLIB_ACCESS_SIZECHARS_(STRING) (*((size_t*)&(((uint8_t*)(&(STRING)))[sizeof(uintptr_t)+sizeof(size_t)])))
#define _STRINGLIB_ACCESS_ALLOCATION_(STRING) (*((void**)&(((uint8_t*)(&(STRING)))[sizeof(uintptr_t)+2*sizeof(size_t)])))
#define _STRINGLIB_ACCESS_SIGNATURE_(STRING) (*((void**)&(((uint8_t*)(&(STRING)))[2*sizeof(uintptr_t)+2*sizeof(size_t)])))
#define _STRINGLIB_ACCESS_THREAD_LEVEL_(STRING) (*((size_t*)&(((uint8_t*)(&(STRING)))[3*sizeof(uintptr_t)+2*sizeof(size_t)])))
#define _STRINGLIB_ACCESS_THREAD_ID_(STRING) (*((_Atomic volatile void**)&(((uint8_t*)(&(STRING)))[3*sizeof(uintptr_t)+3*sizeof(size_t)])))
#else // 8 bit integer does not exist
#define _STRINGLIB_ACCESS_SIZE_(STRING) (*((size_t*)&(((uintptr_t*)(&(STRING)))[1])))
#define _STRINGLIB_ACCESS_SIZECHARS_(STRING) (*((size_t*)&(((size_t*)&(((uintptr_t*)(&(STRING)))[1]))[1])))
#define _STRINGLIB_ACCESS_ALLOCATION_(STRING) (*((void**)&(((size_t*)&(((uintptr_t*)(&(STRING)))[1]))[2])))
#define _STRINGLIB_ACCESS_SIGNATURE_(STRING) (*((void**)&(((size_t*)&(((uintptr_t*)(&(STRING)))[2]))[2])))
#define _STRINGLIB_ACCESS_THREAD_LEVEL_(STRING) (*((size_t*)&(((size_t*)&(((uintptr_t*)(&(STRING)))[3]))[2])))
#define _STRINGLIB_ACCESS_THREAD_ID_(STRING) (*((_Atomic volatile void**)&(((size_t*)&(((uintptr_t*)(&(STRING)))[3]))[3])))
#endif // UINT8_MAX

//Locks the string to other threads, locks can be nested
void (string_lock)(_string *string_a)
{
    #if _STRINGLIB_THREAD_SAFE_
    struct thread_signature {volatile uintptr_t block_a; volatile uintptr_t block_b;} thread = {_STRINGLIB_THREAD_ID_, _STRINGLIB_THREAD_ID_};
    struct thread_signature zero2 = {0, 0};
    struct thread_signature tmp_signature;
    _Atomic volatile struct thread_signature zero1;
    _Atomic volatile struct thread_signature* signature_ptr = (_Atomic volatile struct thread_signature*)&(_STRINGLIB_ACCESS_THREAD_ID_(*string_a));
    size_t* threadlevel_ptr = &_STRINGLIB_ACCESS_THREAD_LEVEL_(*string_a);

    while (atomic_flag_test_and_set(&_STRINGLIB_GLOBAL_VAR_));
    if (_STRINGLIB_HASTHREADS_!=_STRINGLIB_ALLOCATION_TRUE_)
    {
        if (_STRINGLIB_HASTHREADS_==NULL) _STRINGLIB_HASTHREADS_=(void*)_STRINGLIB_THREAD_ID_;
        else if (_STRINGLIB_HASTHREADS_!=(void*)_STRINGLIB_THREAD_ID_) _STRINGLIB_HASTHREADS_=_STRINGLIB_ALLOCATION_TRUE_;
    }
    if ((tmp_signature=atomic_load(signature_ptr)), tmp_signature.block_a != tmp_signature.block_b)
        {*threadlevel_ptr=0; atomic_store(signature_ptr, zero2);}
    atomic_flag_clear(&_STRINGLIB_GLOBAL_VAR_);

    if (_STRINGLIB_THREAD_ID_ != atomic_load(signature_ptr).block_a)
    {
        atomic_store(&zero1, zero2);
        while (!atomic_compare_exchange_weak(signature_ptr, &zero1, thread)) {atomic_store(&zero1, zero2); if (_STRINGLIB_HASTHREADS_!=_STRINGLIB_ALLOCATION_TRUE_) {atomic_store(signature_ptr, zero2); break;}}
        *threadlevel_ptr=0;
    }
    *threadlevel_ptr+=1;
    #else // _STRINGLIB_THREAD_SAFE_
    (void)string_a;
    #endif // _STRINGLIB_THREAD_SAFE_
}

//Unlocks string for the other threads or unlocks a level
void (string_unlock)(_string *string_a)
{
    #if _STRINGLIB_THREAD_SAFE_
    struct thread_signature {volatile uintptr_t block_a; volatile uintptr_t block_b;} zero = {0, 0};
    ///struct thread_signature thread = {(uintptr_t)_STRINGLIB_ALLOCATION_TRUE_, (uintptr_t)_STRINGLIB_ALLOCATION_TRUE_};
    _Atomic volatile struct thread_signature* signature_ptr = (_Atomic volatile struct thread_signature*)&(_STRINGLIB_ACCESS_THREAD_ID_(*string_a));
    size_t* threadlevel_ptr = &_STRINGLIB_ACCESS_THREAD_LEVEL_(*string_a);

    /**if ((uintptr_t)_STRINGLIB_ALLOCATION_TRUE_ == atomic_load(signature_ptr).block_a)
    {
        while (atomic_flag_test_and_set(&_STRINGLIB_GLOBAL_VAR_));
        if (*threadlevel_ptr > 0) *threadlevel_ptr -= 1;
        if (*threadlevel_ptr == 0) atomic_compare_exchange_strong(signature_ptr, &thread, zero);
        atomic_flag_clear(&_STRINGLIB_GLOBAL_VAR_);
        while (*threadlevel_ptr!=_STRINGLIB_LOCAL_VAR_);
        _STRINGLIB_LOCAL_VAR_ = 0;
    }
    else
    {*/
    if (*threadlevel_ptr > 0) *threadlevel_ptr -= 1;
    if (*threadlevel_ptr == 0) atomic_store(signature_ptr, zero);
    ///}
    #else // _STRINGLIB_THREAD_SAFE_
    (void)string_a;
    #endif // _STRINGLIB_THREAD_SAFE_
}

//Locks multiple strings safely, nested locks after a multi-lock can be singular locks
void (string_multilock)(int string_count, ...)
{
    #if _STRINGLIB_THREAD_SAFE_
    int i;
    int j;
    _string **addresses = malloc(sizeof(_string*)*string_count);
    _string **address_min = addresses;
    va_list valist;

    va_start(valist, string_count);
    addresses[0] = va_arg(valist, _string*);
    for (i = 1; i < string_count; i++) if ((addresses[i]=va_arg(valist, _string*)) < *address_min) address_min = addresses+i;
    va_end(valist);

    string_lock(*address_min);
    *address_min = addresses[0];
    address_min=addresses+1;

    for (j = 2; j < string_count+1; j++)
    {
        for (i = j; i < string_count; i++) if (addresses[i] < *address_min) address_min = addresses+i;
        string_lock(*address_min);
        *address_min = addresses[j-1];
        address_min=addresses+j;
    }
    free(addresses);
    #else // _STRINGLIB_THREAD_SAFE_
    (void)string_count;
    #endif // _STRINGLIB_THREAD_SAFE_
}

//Locks write access of the string to other threads, W-locks can be nested and acquired by multiple threads
/**void (string_lockWrite)(_string *string_a)
{
    struct thread_signature {volatile uintptr_t block_a; volatile uintptr_t block_b;} thread = {(uintptr_t)_STRINGLIB_ALLOCATION_TRUE_, (uintptr_t)_STRINGLIB_ALLOCATION_TRUE_};
    struct thread_signature zero2 = {0, 0};
    struct thread_signature tmp_signature;
    _Atomic volatile struct thread_signature zero1;
    _Atomic volatile struct thread_signature* signature_ptr = (_Atomic volatile struct thread_signature*)&(_STRINGLIB_ACCESS_THREAD_ID_(*string_a));
    size_t* threadlevel_ptr = &_STRINGLIB_ACCESS_THREAD_LEVEL_(*string_a);

    while (atomic_flag_test_and_set(&_STRINGLIB_GLOBAL_VAR_));
    if ((tmp_signature=atomic_load(signature_ptr)), tmp_signature.block_a != tmp_signature.block_b)
        {*threadlevel_ptr=0; atomic_store(signature_ptr, zero2);}

    ///TODO: ONLY ONE ATOMIC LOAD
    if (tmp_signature=atomic_load(signature_ptr), (uintptr_t)_STRINGLIB_ALLOCATION_TRUE_ != tmp_signature.block_a && _STRINGLIB_THREAD_ID_ != tmp_signature.block_a)
    {
        atomic_flag_clear(&_STRINGLIB_GLOBAL_VAR_);
        atomic_store(&zero1, zero2);
        while (!atomic_compare_exchange_weak(signature_ptr, &zero1, thread)) atomic_store(&zero1, zero2);
        while (atomic_flag_test_and_set(&_STRINGLIB_GLOBAL_VAR_));
        *threadlevel_ptr=0;
    }
    atomic_flag_clear(&_STRINGLIB_GLOBAL_VAR_);
    while (atomic_flag_test_and_set(&_STRINGLIB_GLOBAL_VAR_));
    if _(!STRINGLIB_LOCAL_VAR_) _STRINGLIB_LOCAL_VAR_=((*threadlevel_ptr)++);
    atomic_flag_clear(&_STRINGLIB_GLOBAL_VAR_);
    return;
}*/

//checks if string_a contains string_b, returns position+1
size_t (string_contains)(_string *string_a, const char *string_b, ...)
{
    size_t tempInt = 0;
    size_t tempScaler;
    size_t tempScaler2;
    va_list valist;

    va_start(valist, string_b);
    tempScaler = ((size_t)va_arg(valist, size_t));
    va_end(valist);

    string_lock(string_a);

    if (!string_isAllocated(string_a)) {string_unlock(string_a); return 0;}

    while (tempScaler < _STRINGLIB_ACCESS_SIZE_(*string_a))
    {
        tempScaler2 = tempScaler;
        while (string_b[tempInt] != '\0')
            {if (_STRINGLIB_ACCESS_STRING_(*string_a)[tempScaler2] == string_b[tempInt]) {++tempInt; ++tempScaler2;} else {tempInt = -1; break;}}
        if (tempInt==-1) tempInt = 0;
        else {string_unlock(string_a); return tempScaler+1;}
        ++tempScaler;
    }
    string_unlock(string_a);
    return 0;
}

//checks if string_a contains all characters (checkall=1/true) or one of the characters (checkall=0/false)
int (string_containsChars)(_string *string_a, int checkall, ...)
{
    char tempChar;
    int tempInt = 0;
    int charnum = 0;
    size_t tempScaler = 0;
    va_list valist;

    string_lock(string_a);
    if (!string_isAllocated(string_a)) {string_unlock(string_a); return 0;}

    va_start(valist, checkall);
    charnum = ((int)va_arg(valist, int));
    while (tempInt < charnum)
    {
        tempChar = (char)va_arg(valist, int);
        if (!checkall) while (tempScaler < _STRINGLIB_ACCESS_SIZE_(*string_a))
        {if (_STRINGLIB_ACCESS_STRING_(*string_a)[tempScaler] == tempChar) {va_end(valist); string_unlock(string_a); return 1;}; ++tempScaler;}
        else
        {
            while (tempScaler < _STRINGLIB_ACCESS_SIZE_(*string_a))
                {if (_STRINGLIB_ACCESS_STRING_(*string_a)[tempScaler] == tempChar) {break;}; ++tempScaler;}
            if (tempScaler == _STRINGLIB_ACCESS_SIZE_(*string_a)) {va_end(valist); string_unlock(string_a); return 0;}
        }
        tempScaler = 0;
        ++tempInt;
    }
    string_unlock(string_a);
    va_end(valist);
    return  charnum?checkall:1;
}

//checks if string_a is identical to string_v
int (string_equals)(_string *string_a, const void *string_v)
{
    size_t tempScaler = 0;
    char* string_b = (char*) string_v;

    string_lock(string_a);
    if (!string_isAllocated(string_a)) {string_unlock(string_a); return 0;}

    while (tempScaler<_STRINGLIB_ACCESS_SIZE_(*string_a) && string_b[tempScaler] != '\0') {if (_STRINGLIB_ACCESS_STRING_(*string_a)[tempScaler] == string_b[tempScaler]) ++tempScaler; else {string_unlock(string_a); return 0;}}
    if (_STRINGLIB_ACCESS_STRING_(*string_a)[tempScaler] == string_b[tempScaler]) {string_unlock(string_a); return 1;}
    string_unlock(string_a);
    return 0;
}

//checks if string is allocated
int (string_isAllocated)(_string *string_a)
{
    int ret;
    string_lock(string_a);
    ret = (_STRINGLIB_ACCESS_ALLOCATION_(*string_a) == _STRINGLIB_ALLOCATION_TRUE_ && _STRINGLIB_ACCESS_SIGNATURE_(*string_a) == _STRINGLIB_ALLOCATION_TRUE_ && (_STRINGLIB_ACCESS_SIZE_(*string_a) == _STRINGLIB_ACCESS_SIZECHARS_(*string_a)+1));
    string_unlock(string_a);
    return ret;
}

//initializes a string to size 1, no need to use this
size_t (string_init)(_string *string_a)
{
    int stringMallocFail = 0;
    char *tempStrMalloc = NULL;

    string_lock(string_a);

    //frees string_a if already allocated
    if (string_isAllocated(string_a))
    {
        free(_STRINGLIB_ACCESS_STRING_(*string_a));
        //unallocates string
        _STRINGLIB_ACCESS_ALLOCATION_(*string_a) = _STRINGLIB_ALLOCATION_FALSE_;
        _STRINGLIB_ACCESS_SIGNATURE_(*string_a) = _STRINGLIB_ALLOCATION_FALSE_;
    }
    _STRINGLIB_ACCESS_STRING_(*string_a) = NULL;

    //initializes string, if initialization fails ends function returning 0
    tempStrMalloc = (char*) malloc(1 * sizeof(char));
    while (tempStrMalloc == NULL)
    {
        tempStrMalloc = (char*) malloc(1 * sizeof(char));
        //should never fail in normal circumstances
        if (++stringMallocFail == _STRINGLIB_MAX_ALLOC_FAILS_) {string_unlock(string_a); free(tempStrMalloc); fputs("string memory initialization failed\n", stderr); return 0;};
    }
    _STRINGLIB_ACCESS_STRING_(*string_a) = tempStrMalloc;
    _STRINGLIB_ACCESS_STRING_(*string_a)[0] = '\0';

    _STRINGLIB_ACCESS_SIZE_(*string_a) = 1;
    _STRINGLIB_ACCESS_SIZECHARS_(*string_a) = 0;

    //allocates string
    _STRINGLIB_ACCESS_ALLOCATION_(*string_a) = _STRINGLIB_ALLOCATION_TRUE_;
    _STRINGLIB_ACCESS_SIGNATURE_(*string_a) = _STRINGLIB_ALLOCATION_TRUE_;

    string_unlock(string_a);

    return 1;
}

//appends the user input to a string
size_t (string_scanAppend)(_string *string_a)
{
    return string_readAppendBuffered(string_a, stdin, _STRINGLIB_IO_READ_BUFSIZE_, 0);
}

//scans a string from user input and returns string size
size_t (string_scan)(_string *string_a)
{
    size_t ret;

    string_lock(string_a);
    //initializes string (even if already initialized), if initialization fails ends function returning 0
    if (string_init(string_a) == 0) {string_unlock(string_a); return 0;}
    ret = string_readAppendBuffered(string_a, stdin, _STRINGLIB_IO_READ_BUFSIZE_, 0);
    string_unlock(string_a);
    return ret;
}

//appends string_b to the end of string_a, string_b must be a pointer to char or a pointer to string
size_t (string_append)(_string *string_a, const void *string_v)
{
    size_t ret;

    string_lock(string_a);
    ret = string_appendCiclePos(string_a, string_v, 1, _STRINGLIB_ACCESS_SIZE_(*string_a)-1);
    string_unlock(string_a);
    return ret;
}

//appends a string to a specific position of another string (0 if omitted)
size_t (string_appendPos)(_string *string_a, const void *string_v, ...)
{
    va_list valist;
    size_t stringsize;

    va_start(valist, string_v);
    stringsize = string_appendCiclePos(string_a, string_v, 1, va_arg(valist, size_t));
    va_end(valist);

    return stringsize;
}

//appends a string multiple times to to a specific position of another string (0 if omitted)
size_t (string_appendCiclePos)(_string *string_a, const void *string_v, unsigned int repeatTimes, ...)
{
    va_list valist;
    size_t pos;
    size_t stringSize_a = 0;
    size_t stringSize_b = 0;
    size_t stringSizeRep_b = 0;
    int stringReallocFail = 0;
    int isScanned = 0;
    char *tempStrRealloc = NULL;

    char* string_b = (char*) string_v;

    va_start(valist, repeatTimes);
    pos = (va_arg(valist, size_t));
    va_end(valist);

    string_lock(string_a);

    //allocates string_a if it was not
    if (!string_isAllocated(string_a))
    {
        if (string_init(string_a) == 0) {string_unlock(string_a); return 0;}
        pos = 0;
    }

    //checks position validity
    if (pos+2>_STRINGLIB_ACCESS_SIZE_(*string_a))
    {
        if (string_v==stdin && repeatTimes==1) {pos=string_readAppendBuffered(string_a, stdin, _STRINGLIB_IO_READ_BUFSIZE_, 0); string_unlock(string_a); return pos;};
        pos = _STRINGLIB_ACCESS_SIZE_(*string_a)-1;
    }
    else if (pos<0) pos = 0;

    //obtains string_b size
    if ((FILE*)string_v != stdin) while (string_b[stringSize_b] != '\0') stringSize_b++;
    else
    {
        //scans string_b (if second parameter is stdin)
        _string tempString = STRING_NEW; if (string_init(&tempString) == 0)
        {
            pos = _STRINGLIB_ACCESS_SIZE_(*string_a);
            string_unlock(string_a);
            fputs("string memory allocation failed\n", stderr);
            return pos;
        }
        if (string_readAppendBuffered(&tempString, stdin, _STRINGLIB_IO_READ_BUFSIZE_, 0) == 0)
        {
            pos = _STRINGLIB_ACCESS_SIZE_(*string_a);
            string_unlock(string_a);
            free(_STRINGLIB_ACCESS_STRING_(tempString));
            fputs("string memory allocation failed\n", stderr);
            return pos;
        }
        isScanned = 1;
        string_b = _STRINGLIB_ACCESS_STRING_(tempString);
        stringSize_b = _STRINGLIB_ACCESS_SIZECHARS_(tempString);
    }

    if (stringSize_b>0)
    {
        //allocates needed memory
        tempStrRealloc = realloc(_STRINGLIB_ACCESS_STRING_(*string_a), (_STRINGLIB_ACCESS_SIZE_(*string_a)+(repeatTimes*stringSize_b)) * sizeof(char));
        while (tempStrRealloc == NULL)
        {
            tempStrRealloc = realloc(_STRINGLIB_ACCESS_STRING_(*string_a), (_STRINGLIB_ACCESS_SIZE_(*string_a)+(repeatTimes*stringSize_b)) * sizeof(char));
            //should never fail in normal circumstances
            if (++stringReallocFail == _STRINGLIB_MAX_ALLOC_FAILS_)
            {
                pos=_STRINGLIB_ACCESS_SIZE_(*string_a);
                string_unlock(string_a);
                free(tempStrRealloc);
                fputs("string memory allocation failed\n", stderr);
                if (isScanned) free(string_b);
                return pos;
            }
        }
        stringReallocFail = 0;

        //checks if string_b is the same string as string_a, if so assigns reallocated pointer to string_b too
        if ((_STRINGLIB_ACCESS_STRING_(*string_a)) == &(string_b[0])) string_b = tempStrRealloc;
        //assigns string_a to reallocated pointer
        _STRINGLIB_ACCESS_STRING_(*string_a) = tempStrRealloc;

        stringSize_a = _STRINGLIB_ACCESS_SIZE_(*string_a);
        //stringReallocFail is used as a placeholder for stringSize_b original value
        stringReallocFail = stringSize_b;
        //drags last part of string_a to the end
        tempStrRealloc = _STRINGLIB_ACCESS_STRING_(*string_a);
        stringSizeRep_b = repeatTimes*stringSize_b;
        while (stringSize_a-pos >0)
        {
            tempStrRealloc[_STRINGLIB_ACCESS_SIZE_(*string_a)+(--stringSizeRep_b)] = tempStrRealloc[--stringSize_a];
        }
        //restores stringSize_b
        stringSize_b = stringReallocFail;
        stringSizeRep_b = repeatTimes*stringSize_b;
        //copies string_b content into string_a
        while (stringSizeRep_b-- >0)
        {
            _STRINGLIB_ACCESS_STRING_(*string_a)[pos+stringSizeRep_b] = string_b[stringSizeRep_b%stringSize_b];
        }

        //frees string_b if it was scanned
        if (isScanned) free(string_b);
    }
    else {pos=_STRINGLIB_ACCESS_SIZE_(*string_a); string_unlock(string_a); return pos;}

    pos = ((_STRINGLIB_ACCESS_SIZE_(*string_a) += repeatTimes*stringSize_b));
    _STRINGLIB_ACCESS_SIZECHARS_(*string_a) += repeatTimes*stringSize_b;
    string_unlock(string_a);
    return pos;
}

//appends the user input to a specific point of a string
size_t (string_scanAppendPos)(_string *string_a, ...)
{
    va_list valist;
    size_t pos = 0;

    va_start(valist, string_a);
    pos = (va_arg(valist, size_t));
    va_end(valist);
    return string_appendCiclePos(string_a, stdin, 1, pos);
}

//sets string_a to string_b
size_t (string_set)(_string *string_a, const void *string_b)
{
    size_t ret;

    string_lock(string_a);
    if (string_b == _STRINGLIB_ACCESS_STRING_(*string_a)) {ret=_STRINGLIB_ACCESS_SIZE_(*string_a); string_unlock(string_a); return ret;}
    //initializes string (even if already initialized), if initialization fails ends function returning 0
    if (string_init(string_a) == 0) {string_unlock(string_a); return 0;}
    //appends to newly initialized string
    ret = string_appendCiclePos(string_a, string_b, 1, _STRINGLIB_ACCESS_SIZE_(*string_a)-1);
    string_unlock(string_a);
    return ret;
}

//starts a new line, can append a string if second argument is string* or char* or scan new line if second argument is stdin
size_t (string_newline)(_string *string_a, ...)
{
    va_list valist;
    size_t ret;
    char *paramvoid = NULL;
    int stringReallocFail = 0;
    size_t newlinepos;
    char *tempStrRealloc = NULL;

    string_lock(string_a);
    //allocates string_a if it was not
    if (!string_isAllocated(string_a)) {if (string_init(string_a) == 0) {string_unlock(string_a); return 0;}}

    //tries to allocate more memory
    tempStrRealloc = realloc(_STRINGLIB_ACCESS_STRING_(*string_a), (_STRINGLIB_ACCESS_SIZE_(*string_a)+1) * sizeof(char));
    while (tempStrRealloc == NULL)
    {
        //retries allocating more memory
        tempStrRealloc = realloc(_STRINGLIB_ACCESS_STRING_(*string_a), (_STRINGLIB_ACCESS_SIZE_(*string_a)+1) * sizeof(char));
        //should never fail in normal circumstances
        if (++stringReallocFail == _STRINGLIB_MAX_ALLOC_FAILS_)
        {
            ret=_STRINGLIB_ACCESS_SIZE_(*string_a);
            string_unlock(string_a);
            free(tempStrRealloc);
            fputs("string memory allocation failed\n", stderr);
            return ret;
        }
    }
    stringReallocFail = 0;

    va_start(valist, string_a);
    paramvoid = (va_arg(valist, char*));
    va_end(valist);

    _STRINGLIB_ACCESS_SIZE_(*string_a) += 1;
    _STRINGLIB_ACCESS_SIZECHARS_(*string_a) += 1;
    if (paramvoid == _STRINGLIB_ACCESS_STRING_(*string_a)) paramvoid = tempStrRealloc;
    _STRINGLIB_ACCESS_STRING_(*string_a) = tempStrRealloc;

    //Inserts new line character
    newlinepos= _STRINGLIB_ACCESS_SIZE_(*string_a)-2;
    _STRINGLIB_ACCESS_STRING_(*string_a)[newlinepos] = '\n';
    _STRINGLIB_ACCESS_STRING_(*string_a)[newlinepos+1] = '\0';

    //eventually appends or scans second string
    ret = ((FILE*)paramvoid == stdin)?
        string_readAppendBuffered(string_a, stdin, _STRINGLIB_IO_READ_BUFSIZE_, 0):
        (paramvoid == _STRINGLIB_ACCESS_STRING_(*string_a))?
            _STRINGLIB_ACCESS_STRING_(*string_a)[newlinepos] = '\0', string_appendCiclePos(string_a, paramvoid, 1, _STRINGLIB_ACCESS_SIZE_(*string_a)-1), _STRINGLIB_ACCESS_STRING_(*string_a)[newlinepos] = '\n', _STRINGLIB_ACCESS_SIZE_(*string_a):
            string_appendCiclePos(string_a, paramvoid, 1, _STRINGLIB_ACCESS_SIZE_(*string_a)-1);
    string_unlock(string_a);
    return ret;
}

//cuts a string from pos (initial position) with a certain size (if specified) or until the end (if not specified or 0)
size_t (string_cut)(_string *string_a, size_t pos, ...)
{
    va_list valist;
    size_t size;
    size_t ret;
    char *tempStrRealloc = NULL;
    size_t initpos;
    int stringReallocFail = 0;
    size_t copypos = 0;

    string_lock(string_a);
    //if string is not allocated, it allocates it and ends function
    if (!string_isAllocated(string_a)) {ret=string_init(string_a); string_unlock(string_a); return ret;}

    va_start(valist, pos);
    size = va_arg(valist, size_t);
    va_end(valist);

    if (pos > _STRINGLIB_ACCESS_SIZE_(*string_a)-1) pos = _STRINGLIB_ACCESS_SIZE_(*string_a)-1;
    initpos = pos;
    if (size<1 || size>_STRINGLIB_ACCESS_SIZE_(*string_a)-pos-2)
    {
        if (pos < 1) {ret=_STRINGLIB_ACCESS_SIZE_(*string_a); string_unlock(string_a); return ret;}
        size = _STRINGLIB_ACCESS_SIZE_(*string_a)-1;
    }

    tempStrRealloc = _STRINGLIB_ACCESS_STRING_(*string_a);
    while (pos != initpos+size)
    {
        tempStrRealloc[copypos++] = tempStrRealloc[pos++];
    }
    _STRINGLIB_ACCESS_STRING_(*string_a)[copypos] = '\0';
    tempStrRealloc = NULL;

    //tries to allocate more memory
    tempStrRealloc = realloc(_STRINGLIB_ACCESS_STRING_(*string_a), (size+1) * sizeof(char));
    while (tempStrRealloc == NULL)
    {
        //retries allocating more memory
        tempStrRealloc = realloc(_STRINGLIB_ACCESS_STRING_(*string_a), (size+1) * sizeof(char));
        //should never fail in normal circumstances
        if (++stringReallocFail == _STRINGLIB_MAX_ALLOC_FAILS_)
        {
            ret=_STRINGLIB_ACCESS_SIZE_(*string_a);
            string_unlock(string_a);
            free(tempStrRealloc);
            fputs("string memory allocation failed\n", stderr);
            return ret;
        }
    }
    stringReallocFail = 0;

    _STRINGLIB_ACCESS_STRING_(*string_a) = tempStrRealloc;
    _STRINGLIB_ACCESS_SIZECHARS_(*string_a) = size;
    ret=(_STRINGLIB_ACCESS_SIZE_(*string_a) = size+1);
    string_unlock(string_a);
    return ret;
}

//swaps pointed strings
void (string_swap)(_string *string_a, _string *string_b)
{
    _string stringTemp;

    string_multilock(2, string_a, string_b);
    stringTemp = *string_a;
    *string_a = *string_b;
    string_unlock(string_a);
    *string_b = stringTemp;
    string_unlock(string_b);
    return;
}

//deletes a string
void (string_delete)(_string *string_a)
{
    string_lock(string_a);
    //frees string_a if already allocated
    if (string_isAllocated(string_a))
    {
        free(_STRINGLIB_ACCESS_STRING_(*string_a));
        //unallocates string
        _STRINGLIB_ACCESS_ALLOCATION_(*string_a) = _STRINGLIB_ALLOCATION_FALSE_;
        _STRINGLIB_ACCESS_SIGNATURE_(*string_a) = _STRINGLIB_ALLOCATION_FALSE_;
    }
    _STRINGLIB_ACCESS_STRING_(*string_a) = NULL;
    string_unlock(string_a);
}

//prints a string, add argument 0 to print without new line
void (string_print)(_string *string_a, ...)
{
    size_t size = 0;
    size_t bufferReallocFail = 0;
    char* buffer;

    va_list valist;
    string_lock(string_a);
    if (string_isAllocated(string_a))
    {
        //print string
        if ((_STRINGLIB_HASTHREADS_!=_STRINGLIB_ALLOCATION_TRUE_ && _STRINGLIB_IO_WRITE_COPY_<2) || !_STRINGLIB_IO_WRITE_COPY_)
        {
            ///fputs(_STRINGLIB_ACCESS_STRING_(*string_a), stdout);
            fwrite(_STRINGLIB_ACCESS_STRING_(*string_a), sizeof(char), _STRINGLIB_ACCESS_SIZE_(*string_a)-1, stdout);
            string_unlock(string_a);

            //print new line if second argument is not 0
            va_start(valist, string_a);
            if (va_arg(valist, int)) fputc('\n', stdout);
            va_end(valist);
        }
        else
        {
            buffer = (char*)malloc((_STRINGLIB_ACCESS_SIZE_(*string_a))*sizeof(char));
            while (buffer == NULL)
            {
                buffer = (char*)malloc((_STRINGLIB_ACCESS_SIZE_(*string_a))*sizeof(char));
                //should never fail in normal circumstances
                if (++bufferReallocFail == _STRINGLIB_MAX_ALLOC_FAILS_) {string_unlock(string_a); free(buffer); fputs("string buffer memory initialization failed\n", stderr); return;};
            }
            for (size=0; *(_STRINGLIB_ACCESS_STRING_(*string_a)+size) != '\0'; ++size) *(buffer+size)=*(_STRINGLIB_ACCESS_STRING_(*string_a)+size);
            //print new line if second argument is not 0
            string_unlock(string_a);

            va_start(valist, string_a);
            if (va_arg(valist, int)) *(buffer+(size++))='\n';
            va_end(valist);

            fwrite(buffer, sizeof(char), size, stdout);
            free(buffer);
        }
    }
    else string_unlock(string_a);
    return;
}

//writes a string to a text file, add extra parameter 0 or false to avoid writing a new line at the end
size_t (string_write)(_string *string_a, FILE *file_a, ...)
{
    size_t offset = 0;
    size_t  pos = 0;
    size_t bufferReallocFail = 0;
    char* buffer;
    va_list valist;

    string_lock(string_a);
    if (string_isAllocated(string_a))
    {
        if ((_STRINGLIB_HASTHREADS_!=_STRINGLIB_ALLOCATION_TRUE_ && _STRINGLIB_IO_WRITE_COPY_<2) || !_STRINGLIB_IO_WRITE_COPY_)
        {
            while (*(_STRINGLIB_ACCESS_STRING_(*string_a)+pos)!= '\0')
            {
                //if string has a new line, prints '\0' char before new line
                if (*(_STRINGLIB_ACCESS_STRING_(*string_a)+pos)== '\n')
                {
                    fwrite(_STRINGLIB_ACCESS_STRING_(*string_a)+offset, sizeof(char), pos-offset, file_a);
                    fwrite("\r\n", sizeof(char), 2, file_a);
                    offset = pos+1;
                }
                ++pos;
            }
            //prints string last line (could be whole string)
            fwrite(_STRINGLIB_ACCESS_STRING_(*string_a)+offset, sizeof(char), pos-offset, file_a);
            string_unlock(string_a);

            //print new line if second argument is not 0
            va_start(valist, file_a);
            if (va_arg(valist, int)) fputc('\n', file_a);
            va_end(valist);
        }
        else
        {
            while (*(_STRINGLIB_ACCESS_STRING_(*string_a)+pos)!= '\0')
            {
                if (*(_STRINGLIB_ACCESS_STRING_(*string_a)+pos)== '\n') ++offset;
                ++pos;
            }
            buffer = (char*)malloc((pos+offset+1)*sizeof(char));
            while (buffer == NULL)
            {
                buffer = (char*)malloc((pos+offset+1)*sizeof(char));
                //should never fail in normal circumstances
                if (++bufferReallocFail == _STRINGLIB_MAX_ALLOC_FAILS_) {string_unlock(string_a); free(buffer); fputs("string buffer memory initialization failed\n", stderr); return 0;};
            }
            pos=(offset=0);
            while (*(_STRINGLIB_ACCESS_STRING_(*string_a)+pos)!= '\0')
            {
                //if string has a new line, prints '\0' char before new lined
                if (*(_STRINGLIB_ACCESS_STRING_(*string_a)+pos)== '\n')
                {
                    *(buffer+pos+offset)='\r';
                    offset++;
                }
                *(buffer+pos+offset)=*(_STRINGLIB_ACCESS_STRING_(*string_a)+pos);
                ++pos;
            }
            string_unlock(string_a);

            va_start(valist, file_a);
            if (va_arg(valist, int)) *(buffer+(pos++)+offset)='\n';
            va_end(valist);

            fwrite(buffer, sizeof(char), pos+offset, file_a);
            free(buffer);
        }
    }
    else string_unlock(string_a);
    return ++pos;
}

//appends a string from a text file with a custom buffer or known size; add extra parameter 0 or false to ignore multi-line string
size_t (string_readAppendBuffered)(_string *string_a, FILE *file_a, size_t buffersize, ...)
{
    char lastChar = '1';
    char *tempStrRealloc = NULL;
    size_t stringSize;
    int stringReallocFail = 0;
    int ignoreLines;
    size_t buffer = 0;
    va_list valist;

    //print new line if second argument is not 0
    va_start(valist, buffersize);
    ignoreLines = (va_arg(valist, int))?0:1;
    va_end(valist);

    string_lock(string_a);

    //initializes string (only if not initialized), if initialization fails ends function returning 0
    if (!string_isAllocated(string_a)) {if (string_init(string_a) == 0) {string_unlock(string_a); return 0;}}

    stringSize = _STRINGLIB_ACCESS_SIZE_(*string_a);

    //reads string
    while ((_STRINGLIB_ACCESS_STRING_(*string_a)[stringSize-1] = fgetc(file_a)) != '\n' || lastChar == '\r')
    {
        if (lastChar == '\r')
        {
            if (_STRINGLIB_ACCESS_STRING_(*string_a)[stringSize-1] != '\n')
            {
                ungetc(_STRINGLIB_ACCESS_STRING_(*string_a)[stringSize-1], file_a);
                break;
            }
            else if (ignoreLines) break;
        }
        if (_STRINGLIB_ACCESS_STRING_(*string_a)[stringSize-1] == '\r')
        {
            lastChar = _STRINGLIB_ACCESS_STRING_(*string_a)[stringSize-1];
            continue;
        }
        lastChar = _STRINGLIB_ACCESS_STRING_(*string_a)[stringSize-1];

        //increases string size, if allocation fails ends function returning the allocated size when terminated
        ++stringSize;
        if (buffer--==0)
        {
            buffer=buffersize;
            tempStrRealloc = realloc(_STRINGLIB_ACCESS_STRING_(*string_a), (stringSize * sizeof(char))+(buffersize * sizeof(char)) );
            while (tempStrRealloc == NULL)
            {
                tempStrRealloc = realloc(_STRINGLIB_ACCESS_STRING_(*string_a), (stringSize * sizeof(char))+(buffersize * sizeof(char)) );
                //should never fail in normal circumstances
                if (++stringReallocFail == _STRINGLIB_MAX_ALLOC_FAILS_)
                {
                    string_unlock(string_a);
                    free(tempStrRealloc);
                    _STRINGLIB_ACCESS_SIZE_(*string_a) = stringSize-1;
                    _STRINGLIB_ACCESS_SIZECHARS_(*string_a) = stringSize-2;
                    fputs("string memory allocation failed\n", stderr);
                    return stringSize-1;
                }
            }
            stringReallocFail = 0;
            _STRINGLIB_ACCESS_STRING_(*string_a) = tempStrRealloc;
        }

        if (feof(file_a)) break;
    }
    if (buffer>0) _STRINGLIB_ACCESS_STRING_(*string_a) = realloc(_STRINGLIB_ACCESS_STRING_(*string_a), stringSize * sizeof(char));

    _STRINGLIB_ACCESS_STRING_(*string_a)[stringSize-1] = '\0';

    _STRINGLIB_ACCESS_SIZE_(*string_a) = stringSize;
    _STRINGLIB_ACCESS_SIZECHARS_(*string_a) = stringSize-1;
    string_unlock(string_a);
    return stringSize;
}

//reads a string from a text file and appends it to string_a, add extra parameter 0 or false to ignore multi-line string
size_t (string_readAppend)(_string *string_a, FILE *file_a, ...)
{
    va_list valist;
    size_t ret;

    va_start(valist, file_a);
    ret = string_readAppendBuffered(string_a, file_a, _STRINGLIB_IO_READ_BUFSIZE_, va_arg(valist, int));
    va_end(valist);

    return ret;
}

//reads a string from a text file, add extra parameter 0 or false to ignore multi-line string
size_t (string_read)(_string *string_a, FILE *file_a, ...)
{
    va_list valist;
    size_t ret;

    string_lock(string_a);
    //initializes string (even if already initialized), if initialization fails ends function returning 0
    if (string_init(string_a) == 0) {string_unlock(string_a); return 0;}

    va_start(valist, file_a);
    ret = string_readAppendBuffered(string_a, file_a, _STRINGLIB_IO_READ_BUFSIZE_, va_arg(valist, int));
    string_unlock(string_a);
    va_end(valist);

    return ret;
}

//reads a string from a text file with a custom buffer or known size; add extra parameter 0 or false to ignore multi-line string
size_t (string_readBuffered)(_string *string_a, FILE *file_a, size_t buffersize, ...)
{
    va_list valist;
    size_t ret;

    string_lock(string_a);
    //initializes string (even if already initialized), if initialization fails ends function returning 0
    if (string_init(string_a) == 0) {string_unlock(string_a); return 0;}

    va_start(valist, buffersize);
    ret = string_readAppendBuffered(string_a, file_a, buffersize, va_arg(valist, int));
    string_unlock(string_a);
    va_end(valist);

    return ret;
}

//writes a string to a binary file
size_t (string_writeBin)(_string *string_a, FILE *file_a)
{
    size_t size;
    char *buffer;

    string_lock(string_a);
    if (!string_isAllocated(string_a)) {string_unlock(string_a); size = 0; fwrite(&size, sizeof(size_t), 1, file_a); return 0;}

    if ((_STRINGLIB_HASTHREADS_!=_STRINGLIB_ALLOCATION_TRUE_ && _STRINGLIB_IO_WRITE_COPY_<2) || !_STRINGLIB_IO_WRITE_COPY_)
    {
        //writes string size
        fwrite(&(_STRINGLIB_ACCESS_SIZE_(*string_a)), sizeof(size_t), 1, file_a);
        //writes string
        fwrite(_STRINGLIB_ACCESS_STRING_(*string_a), sizeof(char), _STRINGLIB_ACCESS_SIZE_(*string_a), file_a);
        string_unlock(string_a);
    }
    else
    {
        size = 0;
        buffer = (char*)malloc(((_STRINGLIB_ACCESS_SIZE_(*string_a))*sizeof(char))+sizeof(size_t));
        while (buffer == NULL)
        {
            buffer = (char*)malloc(((_STRINGLIB_ACCESS_SIZE_(*string_a))*sizeof(char))+sizeof(size_t));
            //should never fail in normal circumstances
            if (++size == _STRINGLIB_MAX_ALLOC_FAILS_) {string_unlock(string_a); free(buffer); fputs("string buffer memory initialization failed\n", stderr); return 0;};
        }
        buffer=(char*)(((size_t*)buffer)+1);
        for (size=0; size<_STRINGLIB_ACCESS_SIZE_(*string_a); ++size) *(buffer+size)=*(_STRINGLIB_ACCESS_STRING_(*string_a)+size);
        string_unlock(string_a);

        buffer=(char*)(((size_t*)buffer)-1);
        *((size_t*)buffer)=size;
        fwrite(buffer, 1, sizeof(size_t)+(size*sizeof(char)), file_a);
        free(buffer);
    }
    return size;
}

//reads a string from a binary file
size_t (string_readBin)(_string *string_a, FILE *file_a)
{
    int stringMallocFail = 0;
    size_t ret;
    char *tempStrMalloc = NULL;

    string_lock(string_a);
    if (string_isAllocated(string_a)) string_delete(string_a);
    //reads string size
    fread(&(_STRINGLIB_ACCESS_SIZE_(*string_a)), sizeof(size_t), 1, file_a);

    //allocates string size, if initialization fails ends function returning 0
    if (_STRINGLIB_ACCESS_SIZE_(*string_a))
    {
        tempStrMalloc = (char*) malloc(_STRINGLIB_ACCESS_SIZE_(*string_a) * sizeof(char));
        while (tempStrMalloc == NULL)
        {
            tempStrMalloc = (char*) malloc(_STRINGLIB_ACCESS_SIZE_(*string_a) * sizeof(char));
            //should never fail in normal circumstances
            if (++stringMallocFail == _STRINGLIB_MAX_ALLOC_FAILS_) {string_unlock(string_a); free(tempStrMalloc); fputs("string memory initialization failed\n", stderr); return 0;};
        }
        _STRINGLIB_ACCESS_STRING_(*string_a) = tempStrMalloc;
        //reads string
        fread(_STRINGLIB_ACCESS_STRING_(*string_a), sizeof(char), _STRINGLIB_ACCESS_SIZE_(*string_a), file_a);
    }

    //sets string char size and signature pointers
    _STRINGLIB_ACCESS_SIZECHARS_(*string_a) = (_STRINGLIB_ACCESS_SIZE_(*string_a))?(_STRINGLIB_ACCESS_SIZE_(*string_a)-1):0;
    _STRINGLIB_ACCESS_ALLOCATION_(*string_a) = (_STRINGLIB_ACCESS_SIZE_(*string_a))?_STRINGLIB_ALLOCATION_TRUE_:_STRINGLIB_ALLOCATION_FALSE_;
    _STRINGLIB_ACCESS_SIGNATURE_(*string_a) = (_STRINGLIB_ACCESS_SIZE_(*string_a))?_STRINGLIB_ALLOCATION_TRUE_:_STRINGLIB_ALLOCATION_FALSE_;
    ret = _STRINGLIB_ACCESS_SIZE_(*string_a);
    string_unlock(string_a);
    return ret;
}

//returns string
const char *const (string_getString)(_string *string_a)
{
    char* ret;
    string_lock(string_a);
    ret = (string_isAllocated)(string_a)?_STRINGLIB_ACCESS_STRING_(*string_a):"";
    string_unlock(string_a);
    return ret;
}

//returns string size in bytes
size_t (string_getSize)(_string *string_a)
{
    size_t ret;
    string_lock(string_a);
    ret = ((string_isAllocated)(string_a))?_STRINGLIB_ACCESS_SIZE_(*string_a):0;
    string_unlock(string_a);
    return ret;
}

//overrides string_a with string_b from specified position
size_t (string_override)(_string *string_a, const void* string_v, ...)
{
    char* string_b = (char*) string_v;
    size_t pos = 0;
    size_t scaler = 0;
    va_list valist;

    va_start(valist, string_v);
    pos = va_arg(valist, size_t);
    va_end(valist);

    string_lock(string_a);
    //allocates string_a if it was not
    if (!string_isAllocated(string_a)) {if (string_init(string_a) == 0) {string_unlock(string_a); return 0;}}

    if (pos<0) pos=0;

    while (*(string_b+scaler) != '\0')
    {
        if (pos < _STRINGLIB_ACCESS_SIZE_(*string_a)-1) *(_STRINGLIB_ACCESS_STRING_(*string_a)+pos)=*(string_b+scaler);
        else {pos = (string_appendCiclePos)(string_a, string_b+scaler, 1, _STRINGLIB_ACCESS_SIZE_(*string_a)-1); string_unlock(string_a); return pos;}
        ++scaler;
        ++pos;
    }
    pos = _STRINGLIB_ACCESS_SIZE_(*string_a);
    string_unlock(string_a);
    return pos;
}


#undef _STRINGLIB_ALLOCATION_TRUE_
#undef _STRINGLIB_ALLOCATION_FALSE_
#undef _STRINGLIB_THREAD_ID_

#undef _STRINGLIB_ACCESS_STRING_
#undef _STRINGLIB_ACCESS_SIZE_
#undef _STRINGLIB_ACCESS_SIZECHARS_
#undef _STRINGLIB_ACCESS_ALLOCATION_
#undef _STRINGLIB_ACCESS_SIGNATURE_
#undef _STRINGLIB_ACCESS_THREAD_LEVEL_
#undef _STRINGLIB_ACCESS_THREAD_ID_
