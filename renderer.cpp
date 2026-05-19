#include "renderer.h"
#include <algorithm>
#include <cmath>
#include <cstdio>
#include <cstring>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

Renderer::Renderer(const Orbital &orb, const RenderParams &params)
    : orbital_(orb), params_(params) {
  density_.resize(params_.width * params_.height);
  phase_.resize(params_.width * params_.height);
}

void Renderer::computeGrid() {
  const int w = params_.width;
  const int h = params_.height;
  const double xmin = params_.xmin, xmax = params_.xmax;
  const double zmin = params_.zmin, zmax = params_.zmax;
  const double fixed = params_.fixed_coord;
  const char axis = params_.slice_axis;

  max_density_ = 0.0;

  for (int row = 0; row < h; ++row) {
    double z = zmin + (zmax - zmin) * row / (h - 1);
    for (int col = 0; col < w; ++col) {
      double x = xmin + (xmax - xmin) * col / (w - 1);
      double y = (axis == 'y') ? fixed : 0.0; // По умолчанию срез y=0

      double density = 0.0;
      double phase = 0.0;

      if (params_.use_real) {
        double val = orbital_.realWave(x, y, z);
        density = val * val;
        phase = 0.0; // Фаза для действительной функции не важна
      } else {
        auto psi = orbital_.complexWave(x, y, z);
        density = std::norm(psi);
        phase = std::arg(psi);
      }

      size_t idx = row * w + col;
      density_[idx] = density;
      phase_[idx] = phase;
      if (density > max_density_)
        max_density_ = density;
    }
  }
  if (max_density_ < 1e-30)
    max_density_ = 1.0; // защита
}

void Renderer::hsv2rgb(double h, double s, double v, double &r, double &g,
                       double &b) {
  // h ∈ [0, 360), s,v ∈ [0,1]
  if (s <= 0.0) {
    r = g = b = v;
    return;
  }
  h /= 60.0;
  int i = static_cast<int>(h);
  double f = h - i;
  double p = v * (1.0 - s);
  double q = v * (1.0 - s * f);
  double t = v * (1.0 - s * (1.0 - f));
  switch (i % 6) {
  case 0:
    r = v;
    g = t;
    b = p;
    break;
  case 1:
    r = q;
    g = v;
    b = p;
    break;
  case 2:
    r = p;
    g = v;
    b = t;
    break;
  case 3:
    r = p;
    g = q;
    b = v;
    break;
  case 4:
    r = t;
    g = p;
    b = v;
    break;
  case 5:
    r = v;
    g = p;
    b = q;
    break;
  }
}

bool Renderer::writePPM() {
  FILE *f = fopen(params_.output_filename.c_str(), "wb");
  if (!f)
    return false;
  fprintf(f, "P6\n%d %d\n255\n", params_.width, params_.height);

  for (int i = 0; i < params_.width * params_.height; ++i) {
    double scaled;
    if (params_.log_scale) {
      double A = params_.log_factor;
      scaled =
          std::log1p(A * density_[i]) / std::log1p(A * max_density_) * 255.0;
    } else {
      scaled = density_[i] / max_density_ * 255.0;
    }
    unsigned char val =
        static_cast<unsigned char>(std::min(255.0, std::max(0.0, scaled)));

    if (params_.colorscheme == ColorScheme::GRAY) {
      fputc(val, f);
      fputc(val, f);
      fputc(val, f);
    } else { // HSV
      // Фаза в градусы: [-π, π] -> [0, 360)
      double hue = (phase_[i] + M_PI) / (2.0 * M_PI) * 360.0;
      double r, g, b;
      hsv2rgb(hue, 1.0, scaled / 255.0, r, g, b);
      fputc(static_cast<unsigned char>(r * 255), f);
      fputc(static_cast<unsigned char>(g * 255), f);
      fputc(static_cast<unsigned char>(b * 255), f);
    }
  }
  fclose(f);
  return true;
}

bool Renderer::writeBMP() {
  // 24-битный BMP без сжатия
  int width = params_.width;
  int height = params_.height;
  int row_padded = (width * 3 + 3) & (~3); // выравнивание до 4 байт
  int data_size = row_padded * height;
  int file_size = 54 + data_size;

  unsigned char header[54] = {0};
  header[0] = 'B';
  header[1] = 'M';
  // размер файла
  header[2] = file_size & 0xFF;
  header[3] = (file_size >> 8) & 0xFF;
  header[4] = (file_size >> 16) & 0xFF;
  header[5] = (file_size >> 24) & 0xFF;
  header[10] = 54; // начало данных
  header[14] = 40; // размер заголовка DIB
  // ширина
  header[18] = width & 0xFF;
  header[19] = (width >> 8) & 0xFF;
  header[20] = (width >> 16) & 0xFF;
  header[21] = (width >> 24) & 0xFF;
  // высота
  header[22] = height & 0xFF;
  header[23] = (height >> 8) & 0xFF;
  header[24] = (height >> 16) & 0xFF;
  header[25] = (height >> 24) & 0xFF;
  header[26] = 1;  // planes
  header[28] = 24; // bpp
  // data_size
  header[34] = data_size & 0xFF;
  header[35] = (data_size >> 8) & 0xFF;
  header[36] = (data_size >> 16) & 0xFF;
  header[37] = (data_size >> 24) & 0xFF;

  FILE *f = fopen(params_.output_filename.c_str(), "wb");
  if (!f)
    return false;
  fwrite(header, 1, 54, f);

  std::vector<unsigned char> row_buf(row_padded, 0);
  // BMP хранит строки снизу вверх
  for (int y = height - 1; y >= 0; --y) {
    for (int x = 0; x < width; ++x) {
      int idx = y * width + x;
      double scaled;
      if (params_.log_scale) {
        double A = params_.log_factor;
        scaled = std::log1p(A * density_[idx]) / std::log1p(A * max_density_) *
                 255.0;
      } else {
        scaled = density_[idx] / max_density_ * 255.0;
      }
      unsigned char val =
          static_cast<unsigned char>(std::min(255.0, std::max(0.0, scaled)));

      unsigned char r, g, b;
      if (params_.colorscheme == ColorScheme::GRAY) {
        r = g = b = val;
      } else {
        double hue = (phase_[idx] + M_PI) / (2.0 * M_PI) * 360.0;
        double rd, gd, bd;
        hsv2rgb(hue, 1.0, scaled / 255.0, rd, gd, bd);
        r = static_cast<unsigned char>(rd * 255);
        g = static_cast<unsigned char>(gd * 255);
        b = static_cast<unsigned char>(bd * 255);
      }
      // BMP порядок BGR
      row_buf[x * 3 + 0] = b;
      row_buf[x * 3 + 1] = g;
      row_buf[x * 3 + 2] = r;
    }
    fwrite(row_buf.data(), 1, row_padded, f);
  }
  fclose(f);
  return true;
}

bool Renderer::render() {
  computeGrid();
  if (params_.format == ImageFormat::PPM) {
    return writePPM();
  } else {
    return writeBMP();
  }
}