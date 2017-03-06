/*****************************************************************************
 * osdep.h: platform-specific code
 *****************************************************************************
 * Copyright (C) 2007-2012 x264 project
 *
 * Authors: Loren Merritt <lorenm@u.washington.edu>
 *          Laurent Aimar <fenrir@via.ecp.fr>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02111, USA.
 *
 * This program is also available under a commercial proprietary license.
 * For more information, contact us at licensing@x264.com.
 *****************************************************************************/

#ifndef X264_OSDEP_H
#define X264_OSDEP_H

#define _LARGEFILE_SOURCE 1
#define _FILE_OFFSET_BITS 64
#include <stdio.h>
#include <sys/stat.h>

#ifndef _MSC_VER
#include "config.h"
#endif

#if HAVE_STDINT_H
#include <stdint.h>
#endif
#if defined(_MSC_VER) || !HAVE_STDINT_H
#include <inttypes.h>
#endif

#ifdef _WIN32
#include <io.h>    // _setmode()
#include <fcntl.h> // _O_BINARY
#endif

#if defined(__ICL) || defined(_MSC_VER)
#define inline __inline
#define strcasecmp _stricmp
#define strncasecmp _strnicmp
#define snprintf _snprintf
#define strtok_r strtok_s
#define S_ISREG(x) (((x) & S_IFMT) == S_IFREG)
#endif

#ifdef __INTEL_COMPILER
#include <mathimf.h>
#else
#include <math.h>
#endif

#if (defined(__GNUC__) || defined(__INTEL_COMPILER)) && (ARCH_X86 || ARCH_X86_64)
#define HAVE_X86_INLINE_ASM 1
#endif

#ifdef _MSC_VER
#define fseek _fseeki64
#define ftell _ftelli64
#define isfinite _finite
#define _CRT_SECURE_NO_DEPRECATE
#define X264_VERSION "" // no configure script for msvc
#endif

#if !defined(isfinite) && (SYS_OPENBSD || SYS_SunOS)
#define isfinite finite
#endif
#ifdef _WIN32
#define rename(src,dst) (unlink(dst), rename(src,dst)) // POSIX says that rename() removes the destination, but win32 doesn't.
#ifndef strtok_r
#define strtok_r(str,delim,save) strtok(str,delim)
#endif
#endif

#if defined(__ICL) || defined(_MSC_VER)
#define DECLARE_ALIGNED( var, n ) __declspec(align(n)) var
#else
#define DECLARE_ALIGNED( var, n ) var __attribute__((aligned(n)))
#endif
#define ALIGNED_16( var ) DECLARE_ALIGNED( var, 16 )
#define ALIGNED_8( var )  DECLARE_ALIGNED( var, 8 )
#define ALIGNED_4( var )  DECLARE_ALIGNED( var, 4 )

// ARM compiliers don't reliably align stack variables
// - EABI requires only 8 byte stack alignment to be maintained
// - gcc can't align stack variables to more even if the stack were to be correctly aligned outside the function
// - armcc can't either, but is nice enough to actually tell you so
// - Apple gcc only maintains 4 byte alignment
// - llvm can align the stack, but only in svn and (unrelated) it exposes bugs in all released GNU binutils...

#define ALIGNED_ARRAY_EMU( mask, type, name, sub1, ... )\
    uint8_t name##_u [sizeof(type sub1 __VA_ARGS__) + mask]; \
    type (*name) __VA_ARGS__ = (void*)((intptr_t)(name##_u+mask) & ~mask)

#if ARCH_ARM && SYS_MACOSX
#define ALIGNED_ARRAY_8( ... ) ALIGNED_ARRAY_EMU( 7, __VA_ARGS__ )
#else
#define ALIGNED_ARRAY_8( type, name, sub1, ... )\
    ALIGNED_8( type name sub1 __VA_ARGS__ )
#endif

#if ARCH_ARM
#define ALIGNED_ARRAY_16( ... ) ALIGNED_ARRAY_EMU( 15, __VA_ARGS__ )
#else
#define ALIGNED_ARRAY_16( type, name, sub1, ... )\
    ALIGNED_16( type name sub1 __VA_ARGS__ )
#endif

#define EXPAND(x) x

#ifdef _MSC_VER
#define ALIGNED_32( var )  DECLARE_ALIGNED( var, 32 )
#define ALIGNED_64( var )  DECLARE_ALIGNED( var, 64 )
#define ALIGNED_ARRAY_32( type, name, sub1, ... ) ALIGNED_32( type name sub1 __VA_ARGS__ )
#define ALIGNED_ARRAY_64( type, name, sub1, ... ) ALIGNED_64( type name sub1 __VA_ARGS__ )
#else
#define ALIGNED_ARRAY_32( ... ) ALIGNED_ARRAY_EMU( 31, __VA_ARGS__ )
#define ALIGNED_ARRAY_64( ... ) ALIGNED_ARRAY_EMU( 63, __VA_ARGS__ )
#endif

#define UNINIT(x) x=x

#if defined(__GNUC__) && (__GNUC__ > 3 || __GNUC__ == 3 && __GNUC_MINOR__ > 0)
#define UNUSED __attribute__((unused))
#define ALWAYS_INLINE __attribute__((always_inline)) inline
#define NOINLINE __attribute__((noinline))
#define MAY_ALIAS __attribute__((may_alias))
#define x264_constant_p(x) __builtin_constant_p(x)
#define x264_nonconstant_p(x) (!__builtin_constant_p(x))
#else
#ifdef __ICL
#define ALWAYS_INLINE __forceinline
#define NOINLINE __declspec(noinline)
#else
#define ALWAYS_INLINE inline
#define NOINLINE
#endif
#define UNUSED
#define MAY_ALIAS
#define x264_constant_p(x) 0
#define x264_nonconstant_p(x) 0
#endif

#if !HAVE_LOG2F
#ifdef _MSC_VER
float __inline log2f(const float x)
{
	__asm {
		fld1
			fld DWORD PTR x
			fyl2x
	}
}
double __inline log2(const double x)
{
	__asm {
		fld1
			fld QWORD PTR x
			fyl2x
	}
}
#else
#define log2f(x) (logf(x)*1.4426950408889634f)
#define log2(x) (log(x)*1.4426950408889634)
#endif
#endif

/* threads */
#if HAVE_BEOSTHREAD
#include <kernel/OS.h>
#define x264_pthread_t               thread_id
static inline int x264_pthread_create( x264_pthread_t *t, void *a, void *(*f)(void *), void *d )
{
     *t = spawn_thread( f, "", 10, d );
     if( *t < B_NO_ERROR )
         return -1;
     resume_thread( *t );
     return 0;
}
#define x264_pthread_join(t,s)       { long tmp; \
                                       wait_for_thread(t,(s)?(long*)(*(s)):&tmp); }

#elif HAVE_POSIXTHREAD
#include <pthread.h>
#define x264_pthread_t               pthread_t
#define x264_pthread_create          pthread_create
#define x264_pthread_join            pthread_join
#define x264_pthread_mutex_t         pthread_mutex_t
#define x264_pthread_mutex_init      pthread_mutex_init
#define x264_pthread_mutex_destroy   pthread_mutex_destroy
#define x264_pthread_mutex_lock      pthread_mutex_lock
#define x264_pthread_mutex_unlock    pthread_mutex_unlock
#define x264_pthread_cond_t          pthread_cond_t
#define x264_pthread_cond_init       pthread_cond_init
#define x264_pthread_cond_destroy    pthread_cond_destroy
#define x264_pthread_cond_broadcast  pthread_cond_broadcast
#define x264_pthread_cond_wait       pthread_cond_wait
#define x264_pthread_attr_t          pthread_attr_t
#define x264_pthread_attr_init       pthread_attr_init
#define x264_pthread_attr_destroy    pthread_attr_destroy
#define x264_pthread_num_processors_np pthread_num_processors_np
#define X264_PTHREAD_MUTEX_INITIALIZER PTHREAD_MUTEX_INITIALIZER

#elif HAVE_WIN32THREAD
#include "win32thread.h"

#else
#define x264_pthread_t               int
#define x264_pthread_create(t,u,f,d) 0
#define x264_pthread_join(t,s)
#endif //HAVE_*THREAD

#if !HAVE_POSIXTHREAD && !HAVE_WIN32THREAD
#define x264_pthread_mutex_t         int
#define x264_pthread_mutex_init(m,f) 0
#define x264_pthread_mutex_destroy(m)
#define x264_pthread_mutex_lock(m)
#define x264_pthread_mutex_unlock(m)
#define x264_pthread_cond_t          int
#define x264_pthread_cond_init(c,f)  0
#define x264_pthread_cond_destroy(c)
#define x264_pthread_cond_broadcast(c)
#define x264_pthread_cond_wait(c,m)
#define x264_pthread_attr_t          int
#define x264_pthread_attr_init(a)    0
#define x264_pthread_attr_destroy(a)
#define X264_PTHREAD_MUTEX_INITIALIZER 0
#endif

#if HAVE_WIN32THREAD || PTW32_STATIC_LIB
int x264_threading_init( void );
#else
#define x264_threading_init() 0
#endif

#define WORD_SIZE sizeof(void*)

#define asm __asm__

#if !defined(_WIN64) && !defined(__LP64__) && defined(_MSC_VER)
#define BROKEN_STACK_ALIGNMENT 1 /* define it if stack is not mod16 */
#endif

#if WORDS_BIGENDIAN
#define endian_fix(x) (x)
#define endian_fix64(x) (x)
#define endian_fix32(x) (x)
#define endian_fix16(x) (x)
#else
#if HAVE_X86_INLINE_ASM && HAVE_MMX
static ALWAYS_INLINE uint32_t endian_fix32( uint32_t x )
{
    asm("bswap %0":"+r"(x));
    return x;
}
#elif defined(_MSC_VER)
#define endian_fix32(x) _byteswap_ulong(x)
#define endian_fix16(x) _rotr16(x,8)
#elif defined(__GNUC__) && HAVE_ARMV6
static ALWAYS_INLINE uint32_t endian_fix32( uint32_t x )
{
    asm("rev %0, %0":"+r"(x));
    return x;
}
#else
static ALWAYS_INLINE uint32_t endian_fix32( uint32_t x )
{
    return (x<<24) + ((x<<8)&0xff0000) + ((x>>8)&0xff00) + (x>>24);
}
#endif
#if HAVE_X86_INLINE_ASM && ARCH_X86_64
static ALWAYS_INLINE uint64_t endian_fix64( uint64_t x )
{
    asm("bswap %0":"+r"(x));
    return x;
}
#else
static ALWAYS_INLINE uint64_t endian_fix64( uint64_t x )
{
    return endian_fix32(x>>32) + ((uint64_t)endian_fix32(x)<<32);
}
#endif
static ALWAYS_INLINE intptr_t endian_fix( intptr_t x )
{
    return WORD_SIZE == 8 ? endian_fix64(x) : endian_fix32(x);
}
#ifndef endian_fix16
static ALWAYS_INLINE uint16_t endian_fix16( uint16_t x )
{
    return (x<<8)|(x>>8);
}
#endif
#endif

#if defined(__GNUC__) && (__GNUC__ > 3 || __GNUC__ == 3 && __GNUC_MINOR__ > 3)
#define x264_clz(x) __builtin_clz(x)
#define x264_ctz(x) __builtin_ctz(x)
#elif defined(_MSC_VER) && defined(_WIN32)
int inline x264_clz( const uint32_t x )
{
	uint32_t r;
	_BitScanReverse(&r,x);
	return r^31;
}
int inline x264_ctz( const uint32_t x )
{
	uint32_t r;
	_BitScanForward(&r,x);
	return r;
}
#else
static int ALWAYS_INLINE x264_clz( uint32_t x )
{
    static uint8_t lut[16] = {4,3,2,2,1,1,1,1,0,0,0,0,0,0,0,0};
    int y, z = (((x >> 16) - 1) >> 27) & 16;
    x >>= z^16;
    z += y = ((x - 0x100) >> 28) & 8;
    x >>= y^8;
    z += y = ((x - 0x10) >> 29) & 4;
    x >>= y^4;
    return z + lut[x];
}

static int ALWAYS_INLINE x264_ctz( uint32_t x )
{
    static uint8_t lut[16] = {4,0,1,0,2,0,1,0,3,0,1,0,2,0,1,0};
    int y, z = (((x & 0xffff) - 1) >> 27) & 16;
    x >>= z;
    z += y = (((x & 0xff) - 1) >> 28) & 8;
    x >>= y;
    z += y = (((x & 0xf) - 1) >> 29) & 4;
    x >>= y;
    return z + lut[x&0xf];
}
#endif

#if HAVE_X86_INLINE_ASM && HAVE_MMX
/* Don't use __builtin_prefetch; even as recent as 4.3.4, GCC seems incapable of
 * using complex address modes properly unless we use inline asm. */
static ALWAYS_INLINE void x264_prefetch( void *p )
{
    asm volatile( "prefetcht0 %0"::"m"(*(uint8_t*)p) );
}
/* We require that prefetch not fault on invalid reads, so we only enable it on
 * known architectures. */
#elif defined(__GNUC__) && (__GNUC__ > 3 || __GNUC__ == 3 && __GNUC_MINOR__ > 1) &&\
      (ARCH_X86 || ARCH_X86_64 || ARCH_ARM || ARCH_PPC)
#define x264_prefetch(x) __builtin_prefetch(x)
#elif defined(_MSC_VER)
#define x264_prefetch(x) _mm_prefetch((const char*)(x), _MM_HINT_T0)
#else
#define x264_prefetch(x)
#endif

#if HAVE_POSIXTHREAD
#if SYS_WINDOWS
#define x264_lower_thread_priority(p)\
{\
    x264_pthread_t handle = pthread_self();\
    struct sched_param sp;\
    int policy = SCHED_OTHER;\
    pthread_getschedparam( handle, &policy, &sp );\
    sp.sched_priority -= p;\
    pthread_setschedparam( handle, policy, &sp );\
}
#else
#include <unistd.h>
#define x264_lower_thread_priority(p) { UNUSED int nice_ret = nice(p); }
#endif /* SYS_WINDOWS */
#elif HAVE_WIN32THREAD
#define x264_lower_thread_priority(p) SetThreadPriority( GetCurrentThread(), X264_MAX( -2, -p ) )
#else
#define x264_lower_thread_priority(p)
#endif

static inline uint8_t x264_is_regular_file( FILE *filehandle )
{
    struct stat file_stat;
    if( fstat( fileno( filehandle ), &file_stat ) )
        return -1;
    return S_ISREG( file_stat.st_mode );
}

static inline uint8_t x264_is_regular_file_path( const char *filename )
{
    struct stat file_stat;
    if( stat( filename, &file_stat ) )
        return -1;
    return S_ISREG( file_stat.st_mode );
}

#ifdef _MSC_VER
#include <xmmintrin.h>

#define X264_BIT_DEPTH 8
#define HIGH_BIT_DEPTH 0

int32_t inline lrintf(const float x)
{
	return _mm_cvt_ss2si(_mm_load_ss(&x));
}

int32_t inline lrint(const double x)
{
	int32_t r;
	__asm fld QWORD PTR x
	__asm fistp DWORD PTR r
	return r;
}

int64_t inline llrint(const double x)
{
	int64_t r;
	__asm fld QWORD PTR x
	__asm fistp QWORD PTR r
	return r;
}
#endif

#ifdef _MSC_VER
double inline round(double d)
{
	__asm fld d
	__asm frndint
}
#endif

#endif /* X264_OSDEP_H */
