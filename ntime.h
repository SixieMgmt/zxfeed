//
// Created by btq on 4/23/16.
//

#ifndef CPP_NTIME_H
#define CPP_NTIME_H

#include <stdint.h>
#include <sys/time.h>
#include <cstddef>
#define us1sec        1000000

    uint64_t  now_usec() {
		struct timeval tv;
		gettimeofday(&tv, NULL);
		return tv.tv_sec  * us1sec + tv.tv_usec;
	}

	static const uint64_t msec = 1;
	static const uint64_t  sec = 1000 * msec;
	static const uint64_t  min =   60 *  sec;
	static const uint64_t hour =   60 *  min;

	static const uint64_t
        t0100 =  1 * hour,
		t0230 =  2 * hour + 30 * min,
        t0300 =  3 * hour,
		t0400 =  4 * hour,
		t0600 =  6 * hour,
		t0700 =  7 * hour,
		t0800 =  8 * hour,
		t0900 =  9 * hour,
		t0910 =  9 * hour + 10 * min,
		t0920 =  9 * hour + 20 * min,
		t0925 =  9 * hour + 25 * min,
		t0929 =  9 * hour + 29 * min,

		t0930 =  9 * hour + 30 * min,
		t1000 = 10 * hour,
		t1100 = 11 * hour,
		t1130 = 11 * hour + 30 * min,
		t1200 = 12 * hour,
		t1300 = 13 * hour,
		t1330 = 13 * hour + 30 * min,
		t1400 = 14 * hour,
		t1430 = 14 * hour + 30 * min,
		t1500 = 15 * hour,
		t1530 = 15 * hour + 30 * min,
		t1540 = 15 * hour + 40 * min,
		t1550 = 15 * hour + 50 * min,
		t1555 = 15 * hour + 55 * min,
		t1559 = 15 * hour + 59 * min,
		t1600 = 16 * hour,
			t1700 = 17 * hour,
			t1800 = 18 * hour,
		t2100 = 21 * hour,
		t2300 = 23 * hour,
        t2330 = 23 * hour + 30 * min,
            t2400 = 24 * hour,
		t0855 = 8 * hour + 55 * min,
		t0856 = 8 * hour + 56 * min,
		t0857 = 8 * hour + 57 * min,
		t2055 = 20 * hour + 55 * min,
		t2056 = 20 * hour + 56 * min,
		t2057 = 20 * hour + 57 * min,
		freeBidDay = t0855, freeBidNight = t2055,
		market_opend  = t0900,
		market_closed = t1500,
		market_openn  = t2100,
        market_close1 = t2330,
        market_close2 = t0100,
        market_close3 = t0300,
		midnight = 86399999ul;


#define ms500       500ul
#define min5		300000ul

#endif //CPP_NTIME_H
