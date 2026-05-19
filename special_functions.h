/**
 * @file special_functions.h
 * @brief Присоединённые полиномы Лагерра и Лежандра, необходимые для атомных
 * орбиталей.
 *
 * Реализация через устойчивые рекуррентные соотношения без внешних библиотек.
 */

#ifndef SPECIAL_FUNCTIONS_H
#define SPECIAL_FUNCTIONS_H

/**
 * @brief Присоединённый полином Лагерра L_k^{alpha}(x).
 * @param k Степень полинома (k >= 0).
 * @param alpha Параметр (вещественный, обычно целый).
 * @param x Аргумент.
 * @return L_k^{alpha}(x).
 */
double laguerre_assoc(int k, double alpha, double x);

/**
 * @brief Присоединённый полином Лежандра P_l^m(x) для x ∈ [-1,1].
 *
 * Используется рекуррентная формула, устойчивая при |x| <= 1.
 * Для m < 0 возвращается 0 (в нашем случае m всегда >= 0).
 *
 * @param l Степень полинома.
 * @param m Порядок (0 <= m <= l).
 * @param x Аргумент (обычно cosθ).
 * @return P_l^m(x).
 */
double legendre_assoc(int l, int m, double x);

#endif