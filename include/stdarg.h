#ifndef __STDARG_H__
#define __STDARG_H__

typedef struct {
  unsigned int gp_offset;
  unsigned int fp_offset;
  void *overflow_arg_area;
  void *reg_save_area;
} __va_list;
typedef __va_list va_list[1];
typedef va_list __gnuc_va_list; // for compatibility

#define va_start(ap, last)                                                     \
  do {                                                                         \
    *(ap) = *(__va_list *)__hidden_va_area__;                                  \
  } while (0)
#define va_end(ap)
#define va_copy(dest, src) ((dest)[0] = (src)[0])

#endif
