/*----------------------------------------------------------------------
 * (c) 2009 Microstrain, Inc.
 *----------------------------------------------------------------------
 * THE PRESENT SOFTWARE WHICH IS FOR GUIDANCE ONLY AIMS AT PROVIDING 
 * CUSTOMERS WITH CODING INFORMATION REGARDING THEIR PRODUCTS IN ORDER 
 * FOR THEM TO SAVE TIME. AS A RESULT, MICROSTRAIN SHALL NOT BE HELD LIABLE 
 * FOR ANY DIRECT, INDIRECT OR CONSEQUENTIAL DAMAGES WITH RESPECT TO ANY 
 * CLAIMS ARISING FROM THE CONTENT OF SUCH SOFTWARE AND/OR THE USE MADE BY 
 * CUSTOMERS OF THE CODING INFORMATION CONTAINED HEREIN IN CONNECTION WITH 
 * THEIR PRODUCTS.
 *---------------------------------------------------------------------*/

/*----------------------------------------------------------------------
 * ms_basic_type.h
 * This file provides a set of typedefs that are commonly used as shorthand for 
 * basic "C" datatypes.  Along with being quicker to type, the typedefs are also
 * unambiguous with regard to the bit width and hence integer maximum and minimum
 * values.
 *----------------------------------------------------------------------*/

#ifndef __ms_basic_type_h
#define __ms_basic_type_h

/* -------- TYPEDEFS */

/* The basics */
typedef signed long  s32;
typedef signed short s16;
typedef signed char  s8;

typedef unsigned long  u32;
typedef unsigned short u16;
typedef unsigned char  u8;

/* Volatile */
/* Most often used for hardware registers or debugging where values are changed 
or viewed external to the scope of the compiled code and thus would otherwise
generate compiler errors or warnings */
typedef volatile signed long  vs32;
typedef volatile signed short vs16;
typedef volatile signed char  vs8;

typedef volatile unsigned long  vu32;
typedef volatile unsigned short vu16;
typedef volatile unsigned char  vu8;

/* Constant */
/* Used to define read-only values */
typedef signed long  const sc32;
typedef signed short const sc16;
typedef signed char  const sc8; 

typedef unsigned long  const uc32;
typedef unsigned short const uc16;
typedef unsigned char  const uc8; 

/* Volatile Constant */
/* An oxymoron but useful when defining read-only values that are changed
external to the scope of the compiled code as with read-only hardware 
registers */
typedef volatile signed long  const vsc32;
typedef volatile signed short const vsc16;
typedef volatile signed char  const vsc8; 

typedef volatile unsigned long  const vuc32;
typedef volatile unsigned short const vuc16;
typedef volatile unsigned char  const vuc8; 

/* -------- DEFINES */
#define U8MAX     ((u8)255)
#define S8MAX     ((s8)127)
#define S8MIN     ((s8)-128)
#define U16MAX    ((u16)65535u)
#define S16MAX    ((s16)32767)
#define S16MIN    ((s16)-32768)
#define U32MAX    ((u32)4294967295uL)
#define S32MAX    ((s32)2147483647)
#define S32MIN    ((s32)2147483648uL)

#endif /* __ms_basic_type_h */

