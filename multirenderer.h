/**
 * @file multirenderer.h
 * @brief Генерация серии срезов и трёхпроекционного изображения.
 */

#ifndef MULTIRENDERER_H
#define MULTIRENDERER_H

#include "orbital.h"
#include "renderer.h"
#include <string>

/**
 * @brief Создаёт последовательность изображений, варьируя фиксированную
 * координату среза.
 *
 * Имена файлов формируются как <prefix>_<index>.<ext>, где index – номер среза
 * от 0 до steps-1. Расширение определяется format (bmp или ppm).
 *
 * @param orbital Орбиталь.
 * @param base_params Базовые параметры (границы по двум осям, размер, цветовая
 * карта и т.д.).
 * @param axis Ось, по которой движется срез ('x','y','z').
 * @param start_val Начальное значение фиксированной координаты.
 * @param end_val Конечное значение.
 * @param num_steps Количество срезов.
 * @param output_prefix Префикс имени файла (например, "slice_").
 * @return true при успешном сохранении всех файлов.
 */
bool render_slice_sequence(const Orbital &orbital,
                           const RenderParams &base_params, char axis,
                           double start_val, double end_val, int num_steps,
                           const std::string &output_prefix);

/**
 * @brief Создаёт одно изображение с тремя ортогональными проекциями (XY, XZ,
 * YZ).
 *
 * @param orbital Орбиталь.
 * @param size_x Ширина каждой под-проекции в пикселях.
 * @param size_y Высота каждой под-проекции.
 * @param bounds Границы [-bounds, bounds] по всем осям.
 * @param output_filename Имя файла (расширение определяет формат: .ppm или
 * .bmp).
 * @param cmap Цветовая карта (общая для всех трёх).
 * @return true в случае успеха.
 */
bool render_three_ortho(const Orbital &orbital, int size_x, int size_y,
                        double bounds, const std::string &output_filename,
                        ColorMap cmap = ColorMap::GRAY);

#endif