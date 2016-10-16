#ifndef RANDOM_DISTRIBUTION_H
#define RANDOM_DISTRIBUTION_H

#define GOODRAND rand()
#define GOODRANDMAX RAND_MAX
#define RANDTYPE long

double gammln(double x);
double poidev(double xm, long *idum);
double expdev (double xm, long *idum);

double normal_distribution(double mu, double sigma);

double poisson_pmf(const double k, const double lambda);

RANDTYPE poisson(double lambda);

#endif
