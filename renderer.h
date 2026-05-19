/**
 * @file renderer.h
 * @brief Генерация изображения распределения электронной плотности.
 *
 * Поддерживает несколько цветовых карт, логарифмическую шкалу,
 * рендеринг в файл и в буфер памяти (для композитинга).
 */

#ifndef RENDERER_H
#define RENDERER_H

#include "orbital.h"
#include <cstdio>
#include <string>
#include <vector>

/// Цветовые карты
enum class ColorMap {
  GRAY,      ///< Оттенки серого по плотности
  HSV_PHASE, ///< HSV: Hue = фаза, Saturation = 1, Value = плотность
  JET,       ///< Псевдоцвета JET (синий–зелёный–красный)
  HOT,       ///< Чёрный–красный–жёлтый–белый
  COOL       ///< Циан–магента
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
  double xmin = -10.0;
  double xmax = 10.0;
  double zmin = -10.0;
  double zmax = 10.0;
  int width = 800;
  int height = 800;
  double fixed_coord = 0.0;
  char slice_axis = 'y';
  ColorMap colormap = ColorMap::GRAY;
  bool log_scale = false;
  double log_factor = 100.0;
  bool use_real = true;

  std::string output_filename = "orbital.ppm";
  ImageFormat format = ImageFormat::PPM;
};

/**
 * @brief Рендерер электронного облака в файл или буфер.
 */
class Renderer {
public:
  /**
   * @brief Конструктор.
   * @param orb Подготовленная орбиталь.
   * @param params Параметры визуализации.
   */
  Renderer(const Orbital &orb, const RenderParams &params);

  /// Выполнить расчёт и сохранить изображение в файл (формат задаётся
  /// params.format).
  bool render();

  /**
   * @brief Выполнить расчёт и записать RGB-изображение в буфер.
   * @param[out] buffer Выходной буфер размером 3*width*height (R,G,B
   * чередуются).
   * @return true в случае успеха.
   */
  bool renderToBuffer(std::vector<unsigned char> &buffer);

private:
  const Orbital &orbital_;
  RenderParams params_;
  std::vector<double> density_;
  std::vector<double> phase_;
  double max_density_ = 0.0;

  void computeGrid();

  bool writePPM();
  bool writeBMP();

  void applyColorMap(double density, double phase, unsigned char &r,
                     unsigned char &g, unsigned char &b) const;

  static std::vector<unsigned char> makeLUT(ColorMap cmap);
};

#endif