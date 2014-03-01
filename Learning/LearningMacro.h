#ifndef _LEARNINGMACRO_H_
#define _LEARNINGMACRO_H_

namespace eagleeye
{
#define MAX_ITER 500

// minimum # of iterations before termination  
#define MIN_ITER 100

// convergence threshold  
#define DELTA_STOP 0.9995  

// number of times in a row the convergence threshold  
// must be reached before stopping  
#define STOP_COUNT 5  

// small cache parameters  
#define INCACHE 25  
#define MINWAIT (INCACHE+25)  
#define REGFREQ 20  

#define CMD_LEN 2048
}

#endif