/*
  Copyright (c) 2014
  Author: Jeff Weisberg <jaw @ tcp4me.com>
  Created: 2014-May-15 13:31 (EDT)
  Function: 

*/
#ifndef __util_h__
#define __util_h__

#define ABS(a)                	(((a)<0)?-(a):(a))
#define MIN(a,b)        	(((a)<(b)) ? (a) : (b))
#define MAX(a,b)        	(((a)>(b)) ? (a) : (b))

#ifndef __cplusplus
typedef char bool;
#endif

extern unsigned int random(void);
extern int volume;

static inline void pause(void){
    tsleep( &systime, -1, "tick", 0 );
}


#endif /* __util_h__ */
