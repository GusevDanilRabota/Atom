/**
 * @file renderer.h
 * @brief Генерация изображения распределения электронной плотности.
 *
 * Поддерживает серую шкалу и HSV-кодирование фазы,
 * логарифмический масштаб, форматы PPM (P6) и BMP (24 бита).
 */

#ifndef RENDERER_H
#define RENDERER_H

#include "orbital.h"
#include <cstdio>
#include <string>
#include <vector>

/// Цветовая схема
enum class ColorScheme {
  GRAY,     ///< Оттенки серого по плотности
  HSV_PHASE ///< HSV: Hue = фаза, Saturation = 1, Value = плотность
            ///< (логарифмическая опционально)
};

/// Выходной графический формат
enum class ImageFormat {
  PPM, ///< Portable Pixmap (P6 binary)
  BMP  ///< Windows Bitmap (24-bit, несжатый)
};

/**
 * @brief Параметры визуализации двумерного среза.
 */
struct RenderParams {
  double xmin = -10.0, xmax = 10.0; ///< Диапазон по X (в a0)
  double ymin = -10.0,
         ymax = 10.0; ///< По Y (для плоскости y=const может быть фиксировано)
  double zmin = -10.0, zmax = 10.0; ///< По Z
  int width = 800;                  ///< Ширина изображения в пикселях
  int height = 800;                 ///< Высота
  double fixed_coord =
      0.0; ///< Фиксированное значение координаты, перпендикулярной срезу
  ///< Ось среза: 'x' — плоскость YZ (фиксирован x), 'y' — XZ, 'z' — XY
  char slice_axis =
      'y'; ///< Ось, ортогональная плоскости среза (y означает срез XZ)
  ColorScheme colorscheme = ColorScheme::GRAY;
  bool log_scale = false;    ///< Логарифмическое сжатие яркости
  double log_factor = 100.0; ///< Коэффициент A в log(1+A*prob)
  ImageFormat format = ImageFormat::PPM;
  std::string output_filename = "orbital.ppm";
  bool use_real = true; ///< true — действительная орбиталь, false — комплексная
};

/**
 * @brief Рендерер электронного облака в файл.
 */
class Renderer {
public:
  /**
   * @brief Конструктор.
   * @param orb Подготовленная орбиталь.
   * @param params Параметры визуализации.
   */
  Renderer(const Orbital &orb, const RenderParams &params);

  /// Выполнить расчёт и сохранить изображение.
  bool render();

private:
  const Orbital &orbital_;
  RenderParams params_;
  std::vector<double> density_; ///< Плотность вероятности в каждом пикселе
  std::vector<double> phase_;   ///< Фаза (радианы), используется для HSV
  double max_density_ = 0.0;

  /// Вычислить плотность (и фазу) на сетке
  void computeGrid();

  /// Запись в PPM
  bool writePPM();
  /// Запись в BMP
  bool writeBMP();

  /// Преобразование HSV в RGB (r,g,b в диапазоне [0,1])
  static void hsv2rgb(double h, double s, double v, double &r, double &g,
                      double &b);
};

#endif