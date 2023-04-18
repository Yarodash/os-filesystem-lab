#pragma once

#include <assert.h>
#include <string.h>

typedef signed char int8;
typedef unsigned char uint8;
typedef signed long int32;
typedef unsigned long uint32;

#define ROUND(x, y) (((x)+((y)-1)) & ~((y)-1))
#define SECTOR(x, y) ((x)/(y))
#define MIN(x, y) ((x)<(y)?(x):(y))
#define MAX(x, y) ((x)>(y)?(x):(y))
#define CLAMP(x, y, z) MAX((x), MIN((y), (z)))
