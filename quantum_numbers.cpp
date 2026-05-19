#include "quantum_numbers.h"

QuantumNumbers::QuantumNumbers(int n_in, int l_in, int m_in)
    : n(n_in), l(l_in), m(m_in) {
  if (n < 1) {
    throw std::invalid_argument("Главное квантовое число n должно быть >= 1");
  }
  if (l < 0 || l >= n) {
    throw std::invalid_argument(
        "Орбитальное число l должно быть в диапазоне [0, n-1]");
  }
  if (m < -l || m > l) {
    throw std::invalid_argument(
        "Магнитное число m должно быть в диапазоне [-l, l]");
  }
}