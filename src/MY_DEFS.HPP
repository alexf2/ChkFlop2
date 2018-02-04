#if !defined(_MY_DEFS_)
#define _MY_DEFS_

#if defined(_MYDLL)
  #define _MYEXPORTCLASS _export
  #define _MYEXPORTFUNC _export
#elif defined(_MYAPP)
  #define _MYEXPORTCLASS _import
  #define _MYEXPORTFUNC extern
#else #define _MYEXPORT
#endif

#endif

