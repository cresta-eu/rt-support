
#ifndef _BITOPS_INCLUDED_
#define _BITOPS_INCLUDED_

/* ------------------------------------------------------------------------- */

#define BIT(i) (1<<(i))

#define SET_BIT(val, i) ((val)  | BIT((i))
#define CLR_BIT(val, i) ((val)  & ~(BIT((i)))
#define TST_BIT(val, i) (((val) & BIT((i))) != 0 )

int bitcount(unsigned int x);

/* ------------------------------------------------------------------------- */

#endif
