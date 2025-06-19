#include <stdio.h>
#include <stddef.h>
#include <time.h> // Para clock_gettime e struct timesps

double predict(double x)
{
  return x * 938.237861251353 + 152.91886182616113;
}

int predict_int(double x)
{
  return (((int)(x * 100)) * 93823 + 1529188) / 100;
}

int main()
{
  double input = 0.03; // valor da feature
  volatile double p1 = 0;
  volatile int p2 = 0;

  int i;
  struct timespec start_ts, end_ts;
  long long elapsed_ns;

  if (clock_gettime(CLOCK_MONOTONIC, &start_ts) == -1)
  {
    perror("clock_gettime (start)");
    return 1;
  }

  for (i = 0; i < 100000; i += 1)
  {
    p1 += predict(input);
  }

  if (clock_gettime(CLOCK_MONOTONIC, &end_ts) == -1)
  {
    perror("clock_gettime (end)");
    return 1;
  }

  elapsed_ns = (end_ts.tv_sec - start_ts.tv_sec) * 1000000000LL +
               (end_ts.tv_nsec - start_ts.tv_nsec);

  printf("Tempo real decorrido: %lld nanossegundos\n", elapsed_ns);
  printf("Tempo real decorrido: %.6f milissegundos\n", (double)elapsed_ns / 1000000.0);

  // outra vez

  if (clock_gettime(CLOCK_MONOTONIC, &start_ts) == -1)
  {
    perror("clock_gettime (start)");
    return 1;
  }

  for (i = 0; i < 100000; i += 1)
  {
    p2 += predict_int(input);
  }

  if (clock_gettime(CLOCK_MONOTONIC, &end_ts) == -1)
  {
    perror("clock_gettime (end)");
    return 1;
  }

  elapsed_ns = (end_ts.tv_sec - start_ts.tv_sec) * 1000000000LL +
               (end_ts.tv_nsec - start_ts.tv_nsec);

  printf("Tempo real decorrido: %lld nanossegundos\n", elapsed_ns);
  printf("Tempo real decorrido: %.6f milissegundos\n", (double)elapsed_ns / 1000000.0);

  printf("Predição: %f %d\n", p1, p2 / 100);
  return 0;
}
