#include "multirenderer.h"
#include <algorithm>
#include <cstdio>
#include <iomanip>
#include <sstream>
#include <vector>

static bool saveBufferToFile(const std::vector<unsigned char> &buffer,
                             int width, int height,
                             const std::string &filename) {
  bool bmp = false;
  if (filename.size() >= 4) {
    std::string ext = filename.substr(filename.size() - 4);
    if (ext == ".bmp" || ext == ".BMP") {
      bmp = true;
    }
  }

  if (bmp) {
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

    FILE *f = fopen(filename.c_str(), "wb");
    if (f == nullptr) {
      return false;
    }
    fwrite(header, 1, 54, f);
    std::vector<unsigned char> row(static_cast<size_t>(row_padded), 0);
    for (int y = height - 1; y >= 0; --y) {
      for (int x = 0; x < width; ++x) {
        size_t src = (static_cast<size_t>(y) * static_cast<size_t>(width) +
                      static_cast<size_t>(x)) *
                     3;
        row[static_cast<size_t>(x) * 3 + 0] = buffer[src + 2];
        row[static_cast<size_t>(x) * 3 + 1] = buffer[src + 1];
        row[static_cast<size_t>(x) * 3 + 2] = buffer[src + 0];
      }
      fwrite(row.data(), 1, static_cast<size_t>(row_padded), f);
    }
    fclose(f);
  } else {
    FILE *f = fopen(filename.c_str(), "wb");
    if (f == nullptr) {
      return false;
    }
    fprintf(f, "P6\n%d %d\n255\n", width, height);
    fwrite(buffer.data(), 1, buffer.size(), f);
    fclose(f);
  }
  return true;
}

bool render_slice_sequence(const Orbital &orbital,
                           const RenderParams &base_params, char axis,
                           double start_val, double end_val, int num_steps,
                           const std::string &output_prefix) {
  std::string ext = (base_params.format == ImageFormat::BMP) ? ".bmp" : ".ppm";
  for (int i = 0; i < num_steps; ++i) {
    double t = start_val + (end_val - start_val) * static_cast<double>(i) /
                               static_cast<double>(num_steps - 1);
    RenderParams p = base_params;
    p.fixed_coord = t;
    p.slice_axis = axis;
    Renderer renderer(orbital, p);
    std::vector<unsigned char> buf;
    if (!renderer.renderToBuffer(buf)) {
      return false;
    }

    std::ostringstream fname;
    fname << output_prefix << std::setfill('0') << std::setw(4) << i << ext;
    if (!saveBufferToFile(buf, p.width, p.height, fname.str())) {
      return false;
    }
  }
  return true;
}

bool render_three_ortho(const Orbital &orbital, int size_x, int size_y,
                        double bounds, const std::string &output_filename,
                        ColorMap cmap) {
  RenderParams p_xy, p_xz, p_yz;
  p_xy.width = p_xz.width = p_yz.width = size_x;
  p_xy.height = p_xz.height = p_yz.height = size_y;
  p_xy.colormap = p_xz.colormap = p_yz.colormap = cmap;
  p_xy.use_real = p_xz.use_real = p_yz.use_real = true;

  p_xy.xmin = -bounds;
  p_xy.xmax = bounds;
  p_xy.zmin = -bounds;
  p_xy.zmax = bounds;
  p_xy.slice_axis = 'z';
  p_xy.fixed_coord = 0.0;

  p_xz.xmin = -bounds;
  p_xz.xmax = bounds;
  p_xz.zmin = -bounds;
  p_xz.zmax = bounds;
  p_xz.slice_axis = 'y';
  p_xz.fixed_coord = 0.0;

  p_yz.xmin = -bounds;
  p_yz.xmax = bounds;
  p_yz.zmin = -bounds;
  p_yz.zmax = bounds;
  p_yz.slice_axis = 'x';
  p_yz.fixed_coord = 0.0;

  Renderer r_xy(orbital, p_xy);
  Renderer r_xz(orbital, p_xz);
  Renderer r_yz(orbital, p_yz);

  std::vector<unsigned char> buf_xy, buf_xz, buf_yz;
  if (!r_xy.renderToBuffer(buf_xy)) {
    return false;
  }
  if (!r_xz.renderToBuffer(buf_xz)) {
    return false;
  }
  if (!r_yz.renderToBuffer(buf_yz)) {
    return false;
  }

  int total_width = size_x * 3;
  int total_height = size_y;
  std::vector<unsigned char> combined(static_cast<size_t>(total_width) *
                                      static_cast<size_t>(total_height) * 3);

  for (int y = 0; y < total_height; ++y) {
    for (int x = 0; x < size_x; ++x) {
      size_t dst_idx =
          (static_cast<size_t>(y) * static_cast<size_t>(total_width) +
           static_cast<size_t>(x)) *
          3;
      size_t src_idx = (static_cast<size_t>(y) * static_cast<size_t>(size_x) +
                        static_cast<size_t>(x)) *
                       3;
      combined[dst_idx] = buf_xy[src_idx];
      combined[dst_idx + 1] = buf_xy[src_idx + 1];
      combined[dst_idx + 2] = buf_xy[src_idx + 2];

      size_t dst2 = (static_cast<size_t>(y) * static_cast<size_t>(total_width) +
                     static_cast<size_t>(x + size_x)) *
                    3;
      combined[dst2] = buf_xz[src_idx];
      combined[dst2 + 1] = buf_xz[src_idx + 1];
      combined[dst2 + 2] = buf_xz[src_idx + 2];

      size_t dst3 = (static_cast<size_t>(y) * static_cast<size_t>(total_width) +
                     static_cast<size_t>(x + 2 * size_x)) *
                    3;
      combined[dst3] = buf_yz[src_idx];
      combined[dst3 + 1] = buf_yz[src_idx + 1];
      combined[dst3 + 2] = buf_yz[src_idx + 2];
    }
  }

  return saveBufferToFile(combined, total_width, total_height, output_filename);
}