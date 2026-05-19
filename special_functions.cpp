#include "special_functions.h"
#include <cmath>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

double laguerre_assoc(int k, double alpha, double x) {
  if (k == 0) {
    return 1.0;
  }
  if (k == 1) {
    return 1.0 + alpha - x;
  }

  double L0 = 1.0;
  double L1 = 1.0 + alpha - x;
  for (int i = 2; i <= k; ++i) {
    double L2 = ((2.0 * i + alpha - 1.0 - x) * L1 - (i + alpha - 1.0) * L0) / i;
    L0 = L1;
    L1 = L2;
  }
  return L1;
}

double legendre_assoc(int l, int m, double x) {
  if (m < 0 || m > l) {
    return 0.0;
  }
  double pmm = 1.0;
  if (m > 0) {
    double fact = 1.0;
    for (int i = 1; i <= m; ++i) {
      pmm *= (1.0 - x * x) * (2.0 * i - 1.0);
      fact *= (2.0 * i);
    }
    pmm = std::sqrt(pmm) / std::sqrt(fact);
    if (m % 2 == 1) {
      pmm = -pmm;
    }
  }

  if (l == m) {
    return pmm;
  }

  double pm1 = x * (2.0 * m + 1.0) * pmm;
  if (l == m + 1) {
    return pm1;
  }

  for (int ll = m + 2; ll <= l; ++ll) {
    double pl = ((2.0 * ll - 1.0) * x * pm1 - (ll + m - 1.0) * pmm) / (ll - m);
    pmm = pm1;
    pm1 = pl;
  }
  return pm1;
}