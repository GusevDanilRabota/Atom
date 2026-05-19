#include "volume_exporter.h"
#include <algorithm>
#include <cmath>
#include <cstdio>
#include <fstream>
#include <future>
#include <thread>

VolumeGrid generate_volume(const Orbital &orbital, double xmin, double xmax,
                           double ymin, double ymax, double zmin, double zmax,
                           int nx, int ny, int nz, int num_threads) {
  VolumeGrid grid;
  grid.nx = nx;
  grid.ny = ny;
  grid.nz = nz;
  grid.xmin = xmin;
  grid.xmax = xmax;
  grid.ymin = ymin;
  grid.ymax = ymax;
  grid.zmin = zmin;
  grid.zmax = zmax;
  grid.density.resize(static_cast<size_t>(nx) * static_cast<size_t>(ny) *
                      static_cast<size_t>(nz));
  grid.phase.resize(static_cast<size_t>(nx) * static_cast<size_t>(ny) *
                        static_cast<size_t>(nz),
                    0.0);

  if (num_threads <= 0) {
    num_threads = std::thread::hardware_concurrency();
    if (num_threads == 0) {
      num_threads = 1;
    }
  }

  std::vector<std::future<void>> futures;
  int layers_per_thread = (nz + num_threads - 1) / num_threads;

  for (int t = 0; t < num_threads; ++t) {
    int start_z = t * layers_per_thread;
    int end_z = std::min(start_z + layers_per_thread, nz);
    if (start_z >= nz) {
      break;
    }

    futures.push_back(std::async(std::launch::async, [&, start_z, end_z]() {
      for (int iz = start_z; iz < end_z; ++iz) {
        double z = grid.zmin + (grid.zmax - grid.zmin) *
                                   static_cast<double>(iz) /
                                   static_cast<double>(grid.nz - 1);
        for (int iy = 0; iy < grid.ny; ++iy) {
          double y = grid.ymin + (grid.ymax - grid.ymin) *
                                     static_cast<double>(iy) /
                                     static_cast<double>(grid.ny - 1);
          for (int ix = 0; ix < grid.nx; ++ix) {
            double x = grid.xmin + (grid.xmax - grid.xmin) *
                                       static_cast<double>(ix) /
                                       static_cast<double>(grid.nx - 1);
            auto psi = orbital.complexWave(x, y, z);
            double dens = std::norm(psi);
            double phase = std::arg(psi);
            size_t idx =
                (static_cast<size_t>(iz) * static_cast<size_t>(grid.ny) +
                 static_cast<size_t>(iy)) *
                    static_cast<size_t>(grid.nx) +
                static_cast<size_t>(ix);
            grid.density[idx] = dens;
            grid.phase[idx] = phase;
          }
        }
      }
    }));
  }
  for (auto &f : futures) {
    f.get();
  }
  return grid;
}

bool write_vtk_legacy(const VolumeGrid &grid, const std::string &filename,
                      bool include_phase) {
  std::ofstream f(filename);
  if (!f) {
    return false;
  }
  f << "# vtk DataFile Version 3.0\n";
  f << "Electron density |psi|^2\n";
  f << "ASCII\n";
  f << "DATASET STRUCTURED_POINTS\n";
  f << "DIMENSIONS " << grid.nx << " " << grid.ny << " " << grid.nz << "\n";
  f << "ORIGIN " << grid.xmin << " " << grid.ymin << " " << grid.zmin << "\n";
  double dx = (grid.xmax - grid.xmin) / static_cast<double>(grid.nx - 1);
  double dy = (grid.ymax - grid.ymin) / static_cast<double>(grid.ny - 1);
  double dz = (grid.zmax - grid.zmin) / static_cast<double>(grid.nz - 1);
  f << "SPACING " << dx << " " << dy << " " << dz << "\n";
  f << "POINT_DATA "
    << (static_cast<size_t>(grid.nx) * static_cast<size_t>(grid.ny) *
        static_cast<size_t>(grid.nz))
    << "\n";
  f << "SCALARS density float\n";
  f << "LOOKUP_TABLE default\n";
  for (int iz = 0; iz < grid.nz; ++iz) {
    for (int iy = 0; iy < grid.ny; ++iy) {
      for (int ix = 0; ix < grid.nx; ++ix) {
        size_t idx = (static_cast<size_t>(iz) * static_cast<size_t>(grid.ny) +
                      static_cast<size_t>(iy)) *
                         static_cast<size_t>(grid.nx) +
                     static_cast<size_t>(ix);
        f << grid.density[idx] << "\n";
      }
    }
  }
  if (include_phase) {
    f << "SCALARS phase float\n";
    f << "LOOKUP_TABLE default\n";
    for (int iz = 0; iz < grid.nz; ++iz) {
      for (int iy = 0; iy < grid.ny; ++iy) {
        for (int ix = 0; ix < grid.nx; ++ix) {
          size_t idx = (static_cast<size_t>(iz) * static_cast<size_t>(grid.ny) +
                        static_cast<size_t>(iy)) *
                           static_cast<size_t>(grid.nx) +
                       static_cast<size_t>(ix);
          f << grid.phase[idx] << "\n";
        }
      }
    }
  }
  return true;
}

bool write_raw_mhd(const VolumeGrid &grid, const std::string &raw_filename,
                   const std::string &mhd_filename) {
  std::ofstream fraw(raw_filename, std::ios::binary);
  if (!fraw) {
    return false;
  }
  for (double val : grid.density) {
    float fval = static_cast<float>(val);
    fraw.write(reinterpret_cast<const char *>(&fval), sizeof(float));
  }
  fraw.close();

  std::string mhd = mhd_filename.empty() ? raw_filename + ".mhd" : mhd_filename;
  std::ofstream fmhd(mhd);
  if (!fmhd) {
    return false;
  }
  fmhd << "ObjectType = Image\n";
  fmhd << "NDims = 3\n";
  fmhd << "BinaryData = True\n";
  fmhd << "BinaryDataByteOrderMSB = False\n";
  fmhd << "CompressedData = False\n";
  fmhd << "TransformMatrix = 1 0 0 0 1 0 0 0 1\n";
  fmhd << "Offset = " << grid.xmin << " " << grid.ymin << " " << grid.zmin
       << "\n";
  fmhd << "CenterOfRotation = 0 0 0\n";
  fmhd << "AnatomicalOrientation = RAI\n";
  fmhd << "ElementSpacing = "
       << (grid.xmax - grid.xmin) / static_cast<double>(grid.nx - 1) << " "
       << (grid.ymax - grid.ymin) / static_cast<double>(grid.ny - 1) << " "
       << (grid.zmax - grid.zmin) / static_cast<double>(grid.nz - 1) << "\n";
  fmhd << "DimSize = " << grid.nx << " " << grid.ny << " " << grid.nz << "\n";
  fmhd << "ElementType = MET_FLOAT\n";
  fmhd << "ElementDataFile = " << raw_filename << "\n";
  return true;
}