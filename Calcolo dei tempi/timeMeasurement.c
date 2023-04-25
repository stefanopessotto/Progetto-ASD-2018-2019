#include "timeMeasurement.h"
#include "weightedMedian.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

int random_seed;

struct measureSpec {
  double delta;
  long long value;
};

void setSeed();

double randomValue();

long long getGranularity();

double setupArray(double *array, int size);

int calcolaRip(double (**P)(double *, int), int functionNumber, double *array,
               int size, long long tMin);

long long tempoMedioNetto(double (*prepara)(double *, int),
                          double (*P)(double *, int), double *array, int size,
                          long long tMin);

void misurazione(double (*prepara)(double *, int), double (*P)(double *, int),
                 double *array, int size, int c, long long tMin, double za,
                 struct measureSpec *retval);

int main(int argc, char **argv) {

  int endSize =
      ARRAY_START_SIZE + ARRAY_NEXT_SIZE_INCREMENT * (NUMBER_OF_ITERATIONS - 1);
  int current = ARRAY_START_SIZE;
  long long granularityNS = 0, tMin = 0;
  double *array;
  struct measureSpec retDETERMINISTICO;

#ifdef DO_ALL
  struct measureSpec retQUICKSORT, retINSERTIONSORT, retRANDOM;
#endif

  array = malloc(sizeof(*array) * endSize);

  setSeed();

  granularityNS = getGranularity();
  tMin = granularityNS / TIME_TOLERANCE;
  printf("Granularity\tTMin(ns)\n%lld\t%lld\n", granularityNS, tMin);
  fflush(stdout);

  printf("size\t");
#ifdef DO_ALL
  printf("Deterministic(ns)\tDeterministic (delta)\t");
  printf("Naive InsertionSort(ns)\tNaive InsertionSort(delta)\t");
  printf("Naive QuickSort(ns)\tNaive QuickSort(delta)\t");
  printf("Random (ns)\tRandom (delta)\n");
#else
  printf("Deterministic (ns)\tDeterministic (delta)\n");
#endif

  for (int i = 0; i < NUMBER_OF_ITERATIONS; i++) {
    printf("%d\t", current);

    misurazione(setupArray, weightedMedianDeterministic, array, current, C,
                tMin, TIME_ZA, &retDETERMINISTICO);

#ifdef DO_ALL
    misurazione(setupArray, weightedMedianRandom, array, current, C, tMin,
                TIME_ZA, &retRANDOM);
    misurazione(setupArray, weightedMedianQuickSort, array, current, C, tMin,
                TIME_ZA, &retQUICKSORT);
    misurazione(setupArray, weightedMedianInsertionSort, array, current, C,
                tMin, TIME_ZA, &retINSERTIONSORT);

    printf("%lld\t%.3f\t", retDETERMINISTICO.value, retDETERMINISTICO.delta);
    printf("%lld\t%.3f\t", retINSERTIONSORT.value, retINSERTIONSORT.delta);
    printf("%lld\t%.3f\t", retQUICKSORT.value, retQUICKSORT.delta);
    printf("%lld\t%.3f\n", retRANDOM.value, retRANDOM.delta);
#else
    printf("%lld\t%.3f\n", retDETERMINISTICO.value, retDETERMINISTICO.delta);
#endif
    fflush(stdout);

    current += ARRAY_NEXT_SIZE_INCREMENT;
  }

  free(array);
  return 0;
}

/**
 * Generazione del seme cross-platform
 * In ambiente linux l'ottimale sarebbe prelevarlo dal file /dev/random.
 * In ambiente windows si possono utilizzare le API per la crittografia
 * (CryptGenRandom).
 */
void setSeed() {
  srand(time(NULL));

  random_seed = 1 + (rand() % (2147483646 - 1));
}

/**
 * Per un miglior generatore valutare l'utilizzo di http://www.pcg-random.org/
 */
double randomValue() {
  int hi = trunc((double)random_seed / (double)Q);
  int lo = random_seed - (Q * hi);

  int test = (A * lo) - (R * hi);

  if (test < 0)
    random_seed = test + M;
  else
    random_seed = test;

  return ((double)random_seed / (double)M);
}

long long getGranularity() {
  struct timespec timeStart, timeEnd;
  timeStart.tv_nsec = 0;
  timeStart.tv_sec = 0;
  timeEnd.tv_nsec = 0;
  timeEnd.tv_sec = 0;

  clock_gettime(CLOCK_MONOTONIC, &timeStart);
  do {
    clock_gettime(CLOCK_MONOTONIC, &timeEnd);
  } while ((timeStart.tv_nsec == timeEnd.tv_nsec) &&
           (timeStart.tv_sec == timeEnd.tv_sec));
  return (timeEnd.tv_sec - timeStart.tv_sec) * (TIME_SECTONSEC_MULTIPLIER) +
         (timeEnd.tv_nsec - timeStart.tv_nsec);
}

long long getTimeNS() {
  struct timespec tmp;
  tmp.tv_nsec = 0;
  tmp.tv_sec = 0;
  clock_gettime(CLOCK_MONOTONIC, &tmp);
  return (tmp.tv_sec * TIME_SECTONSEC_MULTIPLIER) + tmp.tv_nsec;
}

int calcolaRip(double (**P)(double *, int), int functionNumber, double *array,
               int size, long long tMin) {
  long long t0 = 0, t1 = 0;
  int funCounter; // allows multiple functions inside P
  int rip = 1, min = 0, max = 0, cicliErrati = 5;

  while ((t1 - t0) <= tMin) {
    rip = rip * 2;
    t0 = getTimeNS();
    for (int i = 1; i <= rip; i++) {
      for (funCounter = 0; funCounter < functionNumber; funCounter++)
        P[funCounter](array, size);
    }
    t1 = getTimeNS();
  }

  max = rip;
  min = rip / 2;

  while ((max - min) >= cicliErrati) {

    rip = (max + min) / 2;
    t0 = getTimeNS();

    for (int i = 1; i <= rip; i++) {
      for (funCounter = 0; funCounter < functionNumber; funCounter++)
        P[funCounter](array, size);
    }

    t1 = getTimeNS();

    if ((t1 - t0) <= tMin) {
      min = rip;
    } else {
      max = rip;
    }
  }

  return max;
}

double setupArray(double *array, int size) {
  int i = 0;
  while (i < size) {
    array[i] = randomValue();
    i++;
  }
  return 0;
}

long long tempoMedioNetto(double (*prepara)(double *, int),
                          double (*P)(double *, int), double *array, int size,
                          long long tMin) {
  long long t0, t1, tTara, tLordo, tMedio;
  int ripTara, ripLordo;
  double (*ptr[2])(double *, int);
  ptr[0] = prepara;
  ptr[1] = P;

  ripTara = calcolaRip(ptr, 1, array, size, tMin);
  ripLordo = calcolaRip(ptr, 2, array, size, tMin);

  t0 = getTimeNS();
  for (int i = 1; i <= ripTara; i++) {
    ptr[0](array, size);
  }
  t1 = getTimeNS();

  tTara = (t1 - t0);

  t0 = getTimeNS();
  for (int i = 1; i <= ripLordo; i++) {
    ptr[0](array, size);
    ptr[1](array, size);
  }
  t1 = getTimeNS();

  tLordo = (t1 - t0);
  tMedio = ((tLordo / ripLordo) - (tTara / ripTara));
  return tMedio;
}

void misurazione(double (*prepara)(double *, int), double (*P)(double *, int),
                 double *array, int size, int c, long long tMin, double za,
                 struct measureSpec *retval) {
  long long sum2 = 0, m, t = 0;
  int cn = 0;
  ;
  double s, delta;

  do {
    for (int i = 1; i <= c; i++) {
      m = tempoMedioNetto(prepara, P, array, size, tMin);
      t = t + m;
      sum2 = sum2 + (m * m);
    }
    cn = cn + c;
    retval->value = t / cn;
    s = sqrt((double)((unsigned long long)(sum2 / cn) -
                      (unsigned long long)(retval->value * retval->value)));
    retval->delta = ((1 / sqrt(cn)) * za * s);
    delta = retval->value / 20;
  } while (retval->delta >= delta);
}
