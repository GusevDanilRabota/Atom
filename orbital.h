/**
 * @file orbital.h
 * @brief Волновая функция атома водорода (и водородоподобных ионов).
 *
 * Поддерживает произвольные n, l, m и эффективный заряд ядра Z.
 * Использует атомные единицы (боровский радиус a0 = 1).
 */

#ifndef ORBITAL_H
#define ORBITAL_H

#include "quantum_numbers.h"
#include <complex>

/**
 * @brief Класс, вычисляющий водородоподобную волновую функцию.
 *
 * Содержит квантовые числа и заряд ядра Z.
 * Предоставляет методы для получения значения ψ в декартовых координатах
 * (x,y,z). Может возвращать как комплексную, так и действительную (химическую)
 * функцию.
 */
class Orbital {
public:
  /**
   * @brief Конструктор.
   * @param qn Проверенные квантовые числа.
   * @param Z_in Эффективный заряд ядра (по умолчанию 1 для водорода).
   */
  explicit Orbital(const QuantumNumbers &qn, int Z_in = 1);

  /**
   * @brief Комплексная волновая функция ψ_{nlm}(x,y,z).
   * @return std::complex<double>.
   */
  std::complex<double> complexWave(double x, double y, double z) const;

  /**
   * @brief Действительная (химическая) орбиталь.
   *
   * Используются действительные сферические гармоники:
   *   - m = 0: совпадает с Y_{l0}
   *   - m > 0: ~ (Y_l^m + (-1)^m Y_l^{-m}) / √2   → cos(mφ)
   *   - m < 0: ~ (Y_l^{|m|} - (-1)^{|m|} Y_l^{-|m|}) / (i√2) → sin(|m|φ)
   * Возвращает вещественное число.
   */
  double realWave(double x, double y, double z) const;

  QuantumNumbers qn; ///< Квантовые числа (n,l,m)
  int Z;             ///< Эффективный заряд ядра

private:
  /// Радиальная часть R_{nl}(r) с учётом Z (атомные единицы)
  double radial(double r) const;

  /**
   * @brief Комплексная угловая часть Y_{l}^{m}(θ, φ).
   * @param cos_theta cos(θ) = z/r.
   * @param phi Азимутальный угол.
   * @return Значение сферической гармоники.
   */
  std::complex<double> angularComplex(double cos_theta, double phi) const;
};

#endif