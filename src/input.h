/*
  Copyright (c) 2014
  Author: Jeff Weisberg <jaw @ tcp4me.com>
  Created: 2014-Apr-29 12:28 (EDT)
  Function: 

*/

#ifndef __input_h__
#define __input_h__

#define WAITFOR_BUTTON		0x100
#define WAITFOR_DTAP		0x200
#define WAITFOR_TTAP		0x400
#define WAITFOR_UPDN		0x800

extern int  wait_for_action(int, int);

#define wait_for_tap()		wait_for_action( WAITFOR_DTAP, -1 )
#define wait_for_3tap()		wait_for_action( WAITFOR_TTAP, -1 )
#define wait_for_button()	wait_for_action( WAITFOR_BUTTON, -1 )
#define wait_for_any()		wait_for_action( WAITFOR_BUTTON | WAITFOR_DTAP | WAITFOR_UPDN , -1 )


#endif /* __input_h__ */
