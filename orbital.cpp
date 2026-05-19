#include "orbital.h"
#include "special_functions.h"
#include <cmath>
#include <complex>

static constexpr double kPi = 3.14159265358979323846;

Orbital::Orbital(const QuantumNumbers &qn, int Z_in) : qn(qn), Z(Z_in) {}

double Orbital::radial(double r) const {
  if (r < 0.0) {
    return 0.0;
  }
  double rho = 2.0 * static_cast<double>(Z) * r / static_cast<double>(qn.n);

  double norm = std::sqrt(
      std::pow(2.0 * static_cast<double>(Z) / static_cast<double>(qn.n), 3) *
      std::tgamma(static_cast<double>(qn.n - qn.l)) /
      (2.0 * static_cast<double>(qn.n) *
       std::tgamma(static_cast<double>(qn.n + qn.l + 1))));

  double L = laguerre_assoc(qn.n - qn.l - 1, 2.0 * qn.l + 1.0, rho);
  return norm * std::exp(-rho / 2.0) * std::pow(rho, qn.l) * L;
}

std::complex<double> Orbital::angularComplex(double cos_theta,
                                             double phi) const {
  int l = qn.l;
  int m = qn.m;
  int abs_m = std::abs(m);

  double norm = std::sqrt((2.0 * l + 1.0) / (4.0 * kPi) *
                          std::tgamma(static_cast<double>(l - abs_m + 1)) /
                          std::tgamma(static_cast<double>(l + abs_m + 1)));

  double Plm = legendre_assoc(l, abs_m, cos_theta);

  double phase_sign = (m < 0 && (abs_m % 2 == 1)) ? -1.0 : 1.0;
  if (m < 0) {
    norm *= ((abs_m % 2 == 0) ? 1.0 : -1.0);
  }

  std::complex<double> e_iphi(std::cos(m * phi), std::sin(m * phi));
  return norm * Plm * e_iphi * phase_sign;
}

std::complex<double> Orbital::complexWave(double x, double y, double z) const {
  double r = std::sqrt(x * x + y * y + z * z);
  if (r < 1e-12) {
    if (qn.l == 0 && qn.m == 0) {
      return std::complex<double>(radial(0.0) * angularComplex(1.0, 0.0));
    } else {
      return std::complex<double>(0.0, 0.0);
    }
  }
  double cos_theta = z / r;
  double phi = std::atan2(y, x);
  return radial(r) * angularComplex(cos_theta, phi);
}

double Orbital::realWave(double x, double y, double z) const {
  int m = qn.m;
  if (m == 0) {
    return complexWave(x, y, z).real();
  }

  int abs_m = std::abs(m);
  QuantumNumbers qn_plus(qn.n, qn.l, abs_m);
  QuantumNumbers qn_minus(qn.n, qn.l, -abs_m);
  Orbital orb_plus(qn_plus, Z);
  Orbital orb_minus(qn_minus, Z);

  std::complex<double> psi_plus = orb_plus.complexWave(x, y, z);
  std::complex<double> psi_minus = orb_minus.complexWave(x, y, z);

  const double inv_sqrt2 = 1.0 / std::sqrt(2.0);
  if (m > 0) {
    double sign = (abs_m % 2 == 1) ? -1.0 : 1.0;
    return (psi_plus + sign * psi_minus).real() * inv_sqrt2;
  } else {
    double sign = (abs_m % 2 == 1) ? -1.0 : 1.0;
    std::complex<double> diff = psi_plus - sign * psi_minus;
    return (diff * std::complex<double>(0.0, -1.0)).real() * inv_sqrt2;
  }
}