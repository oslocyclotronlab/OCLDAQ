#include <stdio.h>
#include <time.h>
#include <stdlib.h>

int main(int argc, char* argv[])
{
    timeval start, stop, timeout;
    //timespec sleepy, remain;

    //sleepy.tv_sec  = 0;
    //sleepy.tv_nsec = 1000;

    if( argc>1 )
	;//sleepy.tv_nsec = atoi(argv[1]);
    int n = 1000;
    if( argc>2 )
	n = atoi(argv[2]);

    gettimeofday(&start, 0);
    int e=0;
    for(int i=0; i<n; ++i) {
	//e = nanosleep(&sleepy, &remain);

	timeout.tv_sec  = 0;
	timeout.tv_usec = 3;
	e = select(0, 0, 0, 0, &timeout);
    }
    gettimeofday(&stop, 0);

    const double diff = (stop.tv_sec-start.tv_sec)+(stop.tv_usec-start.tv_usec)*1e-6;
    //const double s = (sleepy.tv_sec + sleepy.tv_nsec*1e-9);
    //const double r = (remain.tv_sec + remain.tv_nsec*1e-9);
    //printf("e=%d s=%g r=%g diff=%g sleep=%g\n", e, s, r, diff, diff/n);
    printf("diff=%g sleep=%g\n", diff, diff/n);

    return 0;
}
