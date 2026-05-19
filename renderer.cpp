#include "renderer.h"
#include <algorithm>
#include <cmath>
#include <cstdio>
#include <vector>

static constexpr double kPi = 3.14159265358979323846;

Renderer::Renderer(const Orbital &orb, const RenderParams &params)
    : orbital_(orb), params_(params) {
  density_.resize(static_cast<size_t>(params_.width) *
                  static_cast<size_t>(params_.height));
  phase_.resize(static_cast<size_t>(params_.width) *
                static_cast<size_t>(params_.height));
}

void Renderer::computeGrid() {
  const int w = params_.width;
  const int h = params_.height;
  const double xmin = params_.xmin;
  const double xmax = params_.xmax;
  const double zmin = params_.zmin;
  const double zmax = params_.zmax;
  const double fixed = params_.fixed_coord;
  const char axis = params_.slice_axis;

  max_density_ = 0.0;

  for (int row = 0; row < h; ++row) {
    double z = zmin + (zmax - zmin) * static_cast<double>(row) /
                          static_cast<double>(h - 1);
    for (int col = 0; col < w; ++col) {
      double x = xmin + (xmax - xmin) * static_cast<double>(col) /
                            static_cast<double>(w - 1);
      double y = 0.0;
      if (axis == 'y') {
        y = fixed;
      } else if (axis == 'x') {
        // Для среза XZ при axis='x' фиксируется x
        x = fixed;
        y = 0.0;
      } else if (axis == 'z') {
        // Для среза XY при axis='z' фиксируется z
        z = fixed;
      }

      double density = 0.0;
      double phase = 0.0;

      if (params_.use_real) {
        double val = orbital_.realWave(x, y, z);
        density = val * val;
        phase = 0.0;
      } else {
        auto psi = orbital_.complexWave(x, y, z);
        density = std::norm(psi);
        phase = std::arg(psi);
      }

      size_t idx = static_cast<size_t>(row) * static_cast<size_t>(w) +
                   static_cast<size_t>(col);
      density_[idx] = density;
      phase_[idx] = phase;
      if (density > max_density_) {
        max_density_ = density;
      }
    }
  }
  if (max_density_ < 1e-30) {
    max_density_ = 1.0;
  }
}

std::vector<unsigned char> Renderer::makeLUT(ColorMap cmap) {
  std::vector<unsigned char> lut(256 * 3);
  for (int i = 0; i < 256; ++i) {
    double t = i / 255.0;
    unsigned char r = 0, g = 0, b = 0;
    switch (cmap) {
    case ColorMap::JET: {
      if (t < 0.25) {
        r = 0;
        g = 0;
        b = static_cast<unsigned char>(255 * (t / 0.25));
      } else if (t < 0.5) {
        r = 0;
        g = static_cast<unsigned char>(255 * ((t - 0.25) / 0.25));
        b = 255;
      } else if (t < 0.75) {
        r = static_cast<unsigned char>(255 * ((t - 0.5) / 0.25));
        g = 255;
        b = static_cast<unsigned char>(255 * (1.0 - (t - 0.5) / 0.25));
      } else {
        r = 255;
        g = static_cast<unsigned char>(255 * (1.0 - (t - 0.75) / 0.25));
        b = 0;
      }
      break;
    }
    case ColorMap::HOT: {
      if (t < 0.3333333333333333) {
        r = static_cast<unsigned char>(255 * (t / 0.3333333333333333));
        g = 0;
        b = 0;
      } else if (t < 0.6666666666666666) {
        r = 255;
        g = static_cast<unsigned char>(
            255 * ((t - 0.3333333333333333) / 0.3333333333333333));
        b = 0;
      } else {
        r = 255;
        g = 255;
        b = static_cast<unsigned char>(
            255 * ((t - 0.6666666666666666) / 0.3333333333333333));
      }
      break;
    }
    case ColorMap::COOL: {
      r = static_cast<unsigned char>(255 * t);
      g = static_cast<unsigned char>(255 * (1.0 - t));
      b = 255;
      break;
    }
    default:
      r = g = b = static_cast<unsigned char>(i);
      break;
    }
    lut[3 * i] = r;
    lut[3 * i + 1] = g;
    lut[3 * i + 2] = b;
  }
  return lut;
}

void Renderer::applyColorMap(double density, double phase, unsigned char &r,
                             unsigned char &g, unsigned char &b) const {
  double scaled;
  if (params_.log_scale) {
    double A = params_.log_factor;
    scaled = std::log1p(A * density) / std::log1p(A * max_density_);
  } else {
    scaled = density / max_density_;
  }
  scaled = std::clamp(scaled, 0.0, 1.0);

  if (params_.colormap == ColorMap::GRAY) {
    unsigned char val = static_cast<unsigned char>(scaled * 255.0);
    r = val;
    g = val;
    b = val;
  } else if (params_.colormap == ColorMap::HSV_PHASE) {
    double hue = (phase + kPi) / (2.0 * kPi) * 360.0;
    hue = std::fmod(hue, 360.0);
    double s = 1.0;
    double v = scaled;
    if (s <= 0.0) {
      unsigned char val = static_cast<unsigned char>(v * 255.0);
      r = val;
      g = val;
      b = val;
      return;
    }
    hue /= 60.0;
    int i = static_cast<int>(hue);
    double f = hue - static_cast<double>(i);
    double p = v * (1.0 - s);
    double q = v * (1.0 - s * f);
    double t = v * (1.0 - s * (1.0 - f));
    switch (i % 6) {
    case 0:
      r = static_cast<unsigned char>(v * 255);
      g = static_cast<unsigned char>(t * 255);
      b = static_cast<unsigned char>(p * 255);
      break;
    case 1:
      r = static_cast<unsigned char>(q * 255);
      g = static_cast<unsigned char>(v * 255);
      b = static_cast<unsigned char>(p * 255);
      break;
    case 2:
      r = static_cast<unsigned char>(p * 255);
      g = static_cast<unsigned char>(v * 255);
      b = static_cast<unsigned char>(t * 255);
      break;
    case 3:
      r = static_cast<unsigned char>(p * 255);
      g = static_cast<unsigned char>(q * 255);
      b = static_cast<unsigned char>(v * 255);
      break;
    case 4:
      r = static_cast<unsigned char>(t * 255);
      g = static_cast<unsigned char>(p * 255);
      b = static_cast<unsigned char>(v * 255);
      break;
    case 5:
      r = static_cast<unsigned char>(v * 255);
      g = static_cast<unsigned char>(p * 255);
      b = static_cast<unsigned char>(q * 255);
      break;
    }
  } else {
    static const std::vector<unsigned char> lut_jet = makeLUT(ColorMap::JET);
    static const std::vector<unsigned char> lut_hot = makeLUT(ColorMap::HOT);
    static const std::vector<unsigned char> lut_cool = makeLUT(ColorMap::COOL);
    const std::vector<unsigned char> *lut = nullptr;
    switch (params_.colormap) {
    case ColorMap::JET:
      lut = &lut_jet;
      break;
    case ColorMap::HOT:
      lut = &lut_hot;
      break;
    case ColorMap::COOL:
      lut = &lut_cool;
      break;
    default:
      break;
    }
    if (lut != nullptr) {
      int idx = static_cast<int>(scaled * 255.0);
      r = (*lut)[3 * idx];
      g = (*lut)[3 * idx + 1];
      b = (*lut)[3 * idx + 2];
    } else {
      unsigned char val = static_cast<unsigned char>(scaled * 255);
      r = val;
      g = val;
      b = val;
    }
  }
}

bool Renderer::writePPM() {
  std::vector<unsigned char> buf(static_cast<size_t>(params_.width) *
                                 static_cast<size_t>(params_.height) * 3);
  if (!renderToBuffer(buf)) {
    return false;
  }

  FILE *f = fopen(params_.output_filename.c_str(), "wb");
  if (f == nullptr) {
    return false;
  }
  std::fprintf(f, "P6\n%d %d\n255\n", params_.width, params_.height);
  std::fwrite(buf.data(), 1, buf.size(), f);
  std::fclose(f);
  return true;
}

bool Renderer::writeBMP() {
  std::vector<unsigned char> buf(static_cast<size_t>(params_.width) *
                                 static_cast<size_t>(params_.height) * 3);
  if (!renderToBuffer(buf)) {
    return false;
  }

  int width = params_.width;
  int height = params_.height;
  int row_padded = (width * 3 + 3) & (~3);
  int data_size = row_padded * height;
  int file_size = 54 + data_size;

  unsigned char header[54] = {0};
  header[0] = 'B';
  header[1] = 'M';
  header[2] = static_cast<unsigned char>(file_size & 0xFF);
  header[3] = static_cast<unsigned char>((file_size >> 8) & 0xFF);
  header[4] = static_cast<unsigned char>((file_size >> 16) & 0xFF);
  header[5] = static_cast<unsigned char>((file_size >> 24) & 0xFF);
  header[10] = 54;
  header[14] = 40;
  header[18] = static_cast<unsigned char>(width & 0xFF);
  header[19] = static_cast<unsigned char>((width >> 8) & 0xFF);
  header[20] = static_cast<unsigned char>((width >> 16) & 0xFF);
  header[21] = static_cast<unsigned char>((width >> 24) & 0xFF);
  header[22] = static_cast<unsigned char>(height & 0xFF);
  header[23] = static_cast<unsigned char>((height >> 8) & 0xFF);
  header[24] = static_cast<unsigned char>((height >> 16) & 0xFF);
  header[25] = static_cast<unsigned char>((height >> 24) & 0xFF);
  header[26] = 1;
  header[28] = 24;
  header[34] = static_cast<unsigned char>(data_size & 0xFF);
  header[35] = static_cast<unsigned char>((data_size >> 8) & 0xFF);
  header[36] = static_cast<unsigned char>((data_size >> 16) & 0xFF);
  header[37] = static_cast<unsigned char>((data_size >> 24) & 0xFF);

  FILE *f = fopen(params_.output_filename.c_str(), "wb");
  if (f == nullptr) {
    return false;
  }
  std::fwrite(header, 1, 54, f);

  std::vector<unsigned char> row_buf(static_cast<size_t>(row_padded), 0);
  for (int y = height - 1; y >= 0; --y) {
    for (int x = 0; x < width; ++x) {
      size_t src_idx = (static_cast<size_t>(y) * static_cast<size_t>(width) +
                        static_cast<size_t>(x)) *
                       3;
      row_buf[static_cast<size_t>(x) * 3 + 0] = buf[src_idx + 2];
      row_buf[static_cast<size_t>(x) * 3 + 1] = buf[src_idx + 1];
      row_buf[static_cast<size_t>(x) * 3 + 2] = buf[src_idx + 0];
    }
    std::fwrite(row_buf.data(), 1, static_cast<size_t>(row_padded), f);
  }
  std::fclose(f);
  return true;
}

bool Renderer::renderToBuffer(std::vector<unsigned char> &buffer) {
  computeGrid();
  buffer.resize(static_cast<size_t>(params_.width) *
                static_cast<size_t>(params_.height) * 3);
  for (size_t i = 0; i < density_.size(); ++i) {
    unsigned char r, g, b;
    applyColorMap(density_[i], phase_[i], r, g, b);
    buffer[3 * i] = r;
    buffer[3 * i + 1] = g;
    buffer[3 * i + 2] = b;
  }
  return true;
}

bool Renderer::render() {
  if (params_.format == ImageFormat::PPM) {
    return writePPM();
  } else {
    return writeBMP();
  }
}