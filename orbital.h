/**
 * @file orbital.h
 * @brief Волновая функция атома водорода (комплексная и действительная).
 *
 * Поддерживает произвольные n, l, m. Использует атомные единицы (a0 = 1).
 */

#ifndef ORBITAL_H
#define ORBITAL_H

#include "quantum_numbers.h"
#include <complex>

/**
 * @brief Класс, вычисляющий водородоподобную волновую функцию.
 *
 * Содержит квантовые числа и предоставляет методы для получения
 * значения ψ в декартовых координатах (x,y,z). Может возвращать
 * как комплексную, так и действительную (реальную комбинацию) функцию.
 */
class Orbital {
public:
  /**
   * @brief Конструктор.
   * @param qn Проверенные квантовые числа.
   */
  explicit Orbital(const QuantumNumbers &qn);

  /**
   * @brief Комплексная волновая функция ψ_{nlm}(x,y,z).
   * @return std::complex<double>.
   */
  std::complex<double> complexWave(double x, double y, double z) const;

  /**
   * @brief Действительная волновая функция (химическая орбиталь).
   *
   * Используются действительные сферические гармоники:
   *   - m = 0: совпадает с Y_{l0}
   *   - m > 0: ~ (Y_l^m + (-1)^m Y_l^{-m}) / √2   → cos(mφ)
   *   - m < 0: ~ (Y_l^{|m|} - (-1)^{|m|} Y_l^{-|m|}) / (i√2) → sin(|m|φ)
   * Возвращает вещественное число.
   */
  double realWave(double x, double y, double z) const;

  /// Квантовые числа
  QuantumNumbers qn;

private:
  /// Радиальная часть R_{nl}(r) (атомные единицы)
  double radial(double r) const;

  /**
   * @brief Комплексная угловая часть Y_{l}^{m}(θ, φ).
   * @param cos_theta cos(θ).
   * @param phi Азимутальный угол.
   * @return Значение сферической гармоники.
   */
  std::complex<double> angularComplex(double cos_theta, double phi) const;
};

#endif