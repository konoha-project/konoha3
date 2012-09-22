#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <getopt.h>
#include <math.h>
#include "logpool.h"
#define MAX_PROCS   1024
static struct logpool_param_stream TRACE_API_PARAM = {
  8,
  1024,
  "0.0.0.0", 14801
};
#define LOGAPI_PARAM ((logpool_param_t *) &TRACE_API_PARAM)
#define LOGAPI_INIT_FLAG (LOGPOOL_TRACE)
#define LOGPOOL_TEST_COUNT(argc, argv) get_count(argc, argv)
#define LOGAPI STREAM_API
extern logapi_t LOGAPI;
#define LOG_END 0
#define LOG_s   1
#define LOG_u   2
#define LOG_i   2
#define LOG_f   4

#define LogUint(K,V)    LOG_u, (K), ((uintptr_t)V)
#define LogText(K,V)    LOG_s, (K), (V)

static int verbose = 0;

#define ARRAY_SIZE(a) (sizeof(a) / sizeof(a[0]))
static float factor[] = {
  -0.497485, +0.903388,
  +0.164960, -0.851939,
  -0.790460, +0.996312,
  +0.100574, -0.153701,
  +0.908069, +0.817059,
};

unsigned long long cpu(int i) {
  static long long v = 20/*20%*/;
  float f = (float)(rand()%16);
  v += (long long)(f * sin(i) * factor[(int)f % ARRAY_SIZE(factor)]);
  if (v < 0) v = rand() % 32;
  if (v > 100) v = 100;
  return v;
}

static unsigned long long mem(int i) {
#define DEFAULT_MEM (1024*1024)
#define MAX_MEM     (1024*1024*1024*64UL)
  static unsigned long long v = 1024*4096/*4KB*/;
  float f = (float)(rand()%32);
  v += (long long)(f * sin(i) * factor[(int)f % ARRAY_SIZE(factor)]);
  if (v < 0) v = rand() % 1024 + DEFAULT_MEM;
  if (v > MAX_MEM) v = MAX_MEM;
#undef MAX_MEM
#undef DEFAULT_MEM
  return v;
}

static unsigned long long net(int i) {
  static unsigned long long v = 0;
  float f = (float)(rand()%64);
  v += (long long)(f * sin(i) * factor[(int)f % ARRAY_SIZE(factor)]);
  if (v < 0) v = rand() % 1024;
  if (v > 1024*4096) v = 1024*4096;
  return 0;
}

static void emu(int interval, int step, int pid)
{
  logpool_global_init(LOGAPI_INIT_FLAG);
  logpool_t *logpool = logpool_open(NULL, &LOGAPI, LOGAPI_PARAM);
  int i;
  void *args;
  if (verbose) {
    fprintf(stderr, "start emu %d %d %d\n", interval, step, pid);
  }
  for (i = 0; i < step; ++i) {
    logpool_record(logpool, &args, LOG_NOTICE, "cpuevent",
        LogUint("cpu", cpu(i)),
        LogUint("mem", mem(i)),
        LogUint("net", net(i)),
        LOG_END
        );
    usleep(interval);
  }
  logpool_close(logpool);
  logpool_global_exit();
  if (verbose) {
    fprintf(stderr, "finish emu %d %d %d\n", interval, step, pid);
  }

}

static void start_procs(int procs, int steps, int interval)
{
  int i, pid[MAX_PROCS];
  int status;

  if (procs > MAX_PROCS)
    procs = MAX_PROCS;

  for (i = 0; i < procs; i++) {
    pid[i] = fork();
    if (pid[i] == 0) {
      emu(interval, steps, i);
      break;
    } else {
      if (pid[i] < 0)
        perror("fork");
      usleep(10);
    }
  }

  for (i = 0; i < procs; i++)
    waitpid(pid[i], &status, 0);
}

void usage(void)
{
  fprintf(stderr, "Usage: cpuemu [-h] [-r interval(msec)] [-p nr_procs]"
      " [-s seed] [-v] [-z step]\n");
  fprintf(stderr, "profile: [0] -r100 -p100 -s1 -z100000\n");
  exit(1);
}

int main(int argc, char * argv[])
{
  char *r = "10", *s = "1", *p = "1", *z = "100";
  int rate, procs, seed, steps;
  int c;
  if (argc == 1)
    usage();

  while (1) {
    c = getopt(argc, argv, "hr:p:s:p:Vz:v");
    if (c == -1)
      break;
    switch (c) {
      case 'h':
        usage();
      case 'r':
        r = optarg;
        break;
      case 'p':
        p = optarg;
        break;
      case 's':
        s = optarg;
        break;
      case 'v':
        verbose = 1;
        break;
      case 'V':
        printf("Version 1.0.0\n");
        return 1;
      case 'z':
        z = optarg;
        break;
      case '?':
        return 1;
    }
  }
  seed  = atoi(s);
  rate  = atoi(r);
  procs = atoi(p);
  steps = atoi(z);
  srand(seed);
  fprintf(stderr, "seed=%d, rate=%d, procs=%d, steps=%d\n",
      seed, rate, procs, steps);
  start_procs(procs, steps, rate);
  return 0;
}
