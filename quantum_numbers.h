/**
 * @file quantum_numbers.h
 * @brief Структура для хранения квантовых чисел (n, l, m) с проверками.
 */

#ifndef QUANTUM_NUMBERS_H
#define QUANTUM_NUMBERS_H

#include <stdexcept>
#include <string>

/// Квантовые числа электрона в атоме водорода (или водородоподобном ионе).
struct QuantumNumbers {
  int n; ///< Главное квантовое число, n >= 1
  int l; ///< Орбитальное квантовое число, 0 <= l < n
  int m; ///< Магнитное квантовое число, -l <= m <= l

  /**
   * @brief Конструктор с автоматической проверкой допустимости.
   * @param n_in Главное квантовое число.
   * @param l_in Орбитальное число.
   * @param m_in Магнитное число.
   * @throws std::invalid_argument Если комбинация невозможна.
   */
  QuantumNumbers(int n_in, int l_in, int m_in);
};

#endif