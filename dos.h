/*We can put a whole mess of defines here to make up for the missing
        header files, cuz parser.h is included everywhere!!*/

#include <sys/timeb.h>
#include "values.h"

#define F_OK    0
#define PROT_READ       0x1             /* pages can be read */
#define PROT_WRITE      0x2             /* pages can be written */  
#define PROT_EXEC       0x4             /* pages can be executed */
  
#ifdef  _KERNEL
#define PROT_USER       0x8             /* pages are user accessable */
#define PROT_ZFOD       (PROT_READ | PROT_WRITE | PROT_EXEC | PROT_USER)
#define PROT_ALL        (PROT_READ | PROT_WRITE | PROT_EXEC | PROT_USER)
#endif  /* _KERNEL */

#define PROT_NONE       0x0             /* pages cannot be accessed */

/* sharing types:  must choose either SHARED or PRIVATE */
#define MAP_SHARED      1               /* share changes */
#define MAP_PRIVATE     2               /* changes are private */
#define MAP_TYPE        0xf             /* mask for share type */

/* other flags to mmap (or-ed in to MAP_SHARED or MAP_PRIVATE) */
#define MAP_FIXED       0x10            /* user assigns address */
#define MAP_NORESERVE   0x40            /* don't reserve needed swap area */

typedef int ssize_t;
#define strncasecmp _strnicmp
#define strcasecmp _stricmp
#define popen   _popen
#define pclose	_pclose
/* Constants rounded for 21 decimals. */
#define M_E         2.71828182845904523536
#define M_LOG2E     1.44269504088896340736
#define M_LOG10E    0.434294481903251827651
#define M_LN2       0.693147180559945309417
#define M_LN10      2.30258509299404568402
#define M_PI        3.14159265358979323846
#define M_PI_2      1.57079632679489661923
#define M_PI_4      0.785398163397448309616
#define M_1_PI      0.318309886183790671538
#define M_2_PI      0.636619772367581343076
#define M_1_SQRTPI  0.564189583547756286948
#define M_2_SQRTPI  1.12837916709551257390
#define M_SQRT2     1.41421356237309504880
#define M_SQRT_2    0.707106781186547524401

#define EDOM    33      /* Math argument */
#define ERANGE  34      /* Result too large */
