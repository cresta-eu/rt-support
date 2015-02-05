#ifndef _TIME_MEASUREMENT_INCLUDED_
#define _TIME_MEASUREMENT_INCLUDED_

#ifdef __cplusplus
extern "C" {
#endif


int init_time_measurements( void );
void deinit_time_measurements( void );

int clock_get_usec(double *usec);


#ifdef __cplusplus
}
#endif

#endif /* _TIME_MEASUREMENT_INCLUDED_ */
