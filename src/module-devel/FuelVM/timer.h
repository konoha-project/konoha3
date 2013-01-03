#include <sys/time.h>
#include <stdio.h>

static struct timeval g_timer;
static void reset_timer()
{
	gettimeofday(&g_timer, NULL);
}

static void show_timer(const char *s)
{
	struct timeval endtime;
	gettimeofday(&endtime, NULL);
	double sec = (endtime.tv_sec - g_timer.tv_sec)
		+ (double)(endtime.tv_usec - g_timer.tv_usec) / 1000 / 1000;
	printf("%20s: %f sec\n", s, sec);
	reset_timer();
}

