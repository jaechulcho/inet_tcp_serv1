/*
 * stdalsp.h
 *
 *  Created on: 2017. 2. 18.
 *      Author: josco92
 */

#ifndef STDALSP_H_
#define STDALSP_H_
#include <ctime>

inline double ellapsetime(void)
{
    static struct timespec start = {0};
    static struct timespec end = {0};
    if (start.tv_sec == 0)
    {
        clock_gettime(CLOCK_REALTIME, &start);
    }
    clock_gettime(CLOCK_REALTIME, &end);
    double duration = (end.tv_sec + end.tv_nsec/1000000000.0)
                     -(start.tv_sec + start.tv_nsec/1000000000.0);
    return duration;
}

#define print_msg(io, msgtype, arg...) \
{ \
	flockfile(io); \
	fprintf(io, "[%013.6f]["#msgtype"] [%s/%s:%03d] ", ellapsetime(), __FILE__, __FUNCTION__, __LINE__); \
	fprintf(io, arg); \
	fputc('\n', io); \
	funlockfile(io); \
}
#define pr_err(arg...) print_msg(stderr, ERR, arg)
#define pr_out(arg...) print_msg(stdout, REP, arg)

#endif /* STDALSP_H_ */
