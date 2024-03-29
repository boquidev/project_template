#if !defined(TYPE_DEFINITIONS)
#define TYPE_DEFINITIONS

#define DEBUGMODE 1
#define PRINT_FRAMERATE 0

#define internal static
#define local_persist static
#define global_variable static

#define MAX_S16 32767 
#define MIN_S16 -32768
#define MAX_U16 65535
// FLOATS
#define MAX_FLOAT          3.402823466e+38F        // max value
#define MIN_FLOAT          1.175494351e-38F        // min normalized positive value
#define F32_MIN_THRESHOLD (1e-6f)
#define COMPARE_FLOATS(f1, f2) ((((f2)-F32_MIN_THRESHOLD)<=(f1)) && ((f1)<=(((f2))+F32_MIN_THRESHOLD)))

#define NULL_INDEX16 0xffff
#define NULL_INDEX32 0xffffffff


#define PI32    3.14159265359f
#define TAU32     6.28318530718f
#define SQRT_1HALF 0.7071067811f

#define FIRST_CHAR 32
#define LAST_CHAR 255
#define CHARS_COUNT (LAST_CHAR-FIRST_CHAR)
#define CAPS_DIFFERENCE
#define CHAR_TO_INDEX(c) c-FIRST_CHAR

#define HOLDING_BUTTON_TIME 10

typedef char s8;
typedef short s16;
typedef int s32;
typedef long long s64;

typedef unsigned char u8;
typedef unsigned short u16;
typedef unsigned int u32;
typedef unsigned long long u64;

typedef char b8;
typedef short b16;
typedef int b32;
typedef s64 b64;

typedef float f32;
typedef double f64;

#if DEBUGMODE
    #define ASSERT(expression) if(!(expression)) *(int *)0 = 0;
#else
    #define ASSERT(expression) (expression)
#endif

#define ASSERTHR(hr) ASSERT(SUCCEEDED(hr))

#define UNTIL(i, range) for(u32 i=0; i<range;i++)


#define KILOBYTES(value) ((value) * 1024LL)
#define MEGABYTES(value) (KILOBYTES(value) * 1024LL)
#define GIGABYTES(value) (MEGABYTES(value) * 1024LL)
#define TERABYTES(value) (GIGABYTES(value) * 1024LL)

#define BUFFER_GET_POS(buffer) (buffer->data+buffer->pos)


#define SCAN(p, type) *(type*)p; \
    p+=sizeof(type);

#define ARRAYCOUNT(Array) (sizeof(Array) / sizeof((Array)[0]))

#define MIN(a,b) (((a)<(b))?(a):(b))
#define MAX(a,b) (((a)>(b))?(a):(b))

#define CLAMP(a,x,b)    (((x)<(a))?(a):\
                        ((b)<(x))?(b):(x))

#define c_linkage extern "C"
#define c_linkage_begin extern "C"{
#define c_linkage_end }
#endif