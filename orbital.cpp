#include "orbital.h"
#include "special_functions.h"
#include <cmath>
#include <complex>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

Orbital::Orbital(const QuantumNumbers &qn) : qn(qn) {}

// Радиальная функция R_{nl}(r) (в атомных единицах, a0=1)
double Orbital::radial(double r) const {
  int n = qn.n, l = qn.l;
  if (r < 0)
    return 0.0;
  double rho = 2.0 * r / n; // масштабированная переменная

  // Нормировочный множитель
  double norm = std::sqrt(std::pow(2.0 / n, 3) * std::tgamma(n - l) /
                          (2.0 * n * std::tgamma(n + l + 1)));

  // Полином Лагерра L_{n-l-1}^{2l+1}(rho)
  double L = laguerre_assoc(n - l - 1, 2 * l + 1, rho);

  return norm * std::exp(-rho / 2.0) * std::pow(rho, l) * L;
}

// Комплексная угловая часть: Y_l^m(θ, φ)
std::complex<double> Orbital::angularComplex(double cos_theta,
                                             double phi) const {
  int l = qn.l, m = qn.m;
  double abs_m = std::abs(m);

  // Нормировочный множитель
  double norm =
      std::sqrt((2.0 * l + 1.0) / (4.0 * M_PI) * std::tgamma(l - abs_m + 1) /
                std::tgamma(l + abs_m + 1));

  // Присоединённый полином Лежандра P_l^{|m|}(cosθ)
  double Plm = legendre_assoc(l, static_cast<int>(abs_m), cos_theta);

  double phase_sign = (m < 0 && (std::abs(m) % 2 == 1)) ? -1.0 : 1.0; // (-1)^m
  if (m < 0) {
    norm *= std::pow(-1.0, abs_m); // стандартная фаза Кондона-Шортли
  }

  // Экспонента e^{i m φ}
  std::complex<double> e_iphi(std::cos(m * phi), std::sin(m * phi));
  return norm * Plm * e_iphi * phase_sign;
}

// Комплексная волновая функция ψ = R(r) * Y(θ,φ)
std::complex<double> Orbital::complexWave(double x, double y, double z) const {
  double r = std::sqrt(x * x + y * y + z * z);
  if (r < 1e-12) {
    // Для r=0: только s-орбиталь даёт ненулевое значение
    if (qn.l == 0 && qn.m == 0)
      return std::complex<double>(radial(0.0) * angularComplex(1.0, 0.0));
    else
      return std::complex<double>(0.0, 0.0);
  }
  double cos_theta = z / r;
  double phi = std::atan2(y, x);
  return radial(r) * angularComplex(cos_theta, phi);
}

// Действительная (химическая) орбиталь
double Orbital::realWave(double x, double y, double z) const {
  int m = qn.m;
  if (m == 0) {
    return complexWave(x, y, z).real(); // Y_{l0} действительна
  }

  // Для m > 0: действительная комбинация ~ cos(mφ)
  // Для m < 0: ~ sin(|m|φ)
  int abs_m = std::abs(m);
  // Вычисляем комплексную функцию для +|m| и -|m|
  QuantumNumbers qn_plus(qn.n, qn.l, abs_m);
  QuantumNumbers qn_minus(qn.n, qn.l, -abs_m);
  Orbital orb_plus(qn_plus);
  Orbital orb_minus(qn_minus);

  std::complex<double> psi_plus = orb_plus.complexWave(x, y, z);
  std::complex<double> psi_minus = orb_minus.complexWave(x, y, z);

  const double inv_sqrt2 = 1.0 / std::sqrt(2.0);
  if (m > 0) {
    // (Y_l^m + (-1)^m Y_l^{-m}) / √2
    double sign = (abs_m % 2 == 1) ? -1.0 : 1.0;
    return (psi_plus + sign * psi_minus).real() * inv_sqrt2;
  } else { // m < 0
    // (Y_l^{|m|} - (-1)^{|m|} Y_l^{-|m|}) / (i√2)
    double sign = (abs_m % 2 == 1) ? -1.0 : 1.0;
    std::complex<double> diff = psi_plus - sign * psi_minus;
    // Деление на i = умножение на -i, затем берём действительную часть
    return (diff * std::complex<double>(0, -1)).real() * inv_sqrt2;
  }
}