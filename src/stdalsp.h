/*
 * stdalsp.h
 *
 *  Created on: 2017. 2. 18.
 *      Author: josco92
 */

#ifndef STDALSP_H_
#define STDALSP_H_

#define print_msg(io, msgtype, arg...) \
	flockfile(io); \
	fprintf(io, "["#msgtype"] [%s/%s:%03d] ", __FILE__, __FUNCTION__, __LINE__); \
	fprintf(io, arg); \
	fputc('\n', io); \
	funlockfile(io);
#define pr_err(arg...) print_msg(stderr, ERR, arg)
#define pr_out(arg...) print_msg(stdout, REP, arg)

#endif /* STDALSP_H_ */
