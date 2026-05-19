/**
 * @file volume_exporter.h
 * @brief Экспорт 3D-воксельных данных в форматы VTK, RAW/MHD.
 */

#ifndef VOLUME_EXPORTER_H
#define VOLUME_EXPORTER_H

#include "orbital.h"
#include <string>
#include <vector>

/// Трёхмерная воксельная сетка с плотностью вероятности и фазой.
struct VolumeGrid {
  int nx, ny, nz;
  double xmin, xmax, ymin, ymax, zmin, zmax;
  std::vector<double> density;
  std::vector<double> phase;
};

/**
 * @brief Генерирует объёмную сетку значений |ψ|² (и фазы) с многопоточным
 * вычислением.
 * @param orbital Орбиталь.
 * @param xmin,xmax,ymin,ymax,zmin,zmax Границы объёма.
 * @param nx,ny,nz Количество вокселей по осям.
 * @param num_threads Число потоков (0 = автоматически).
 * @return VolumeGrid с вычисленными данными.
 */
VolumeGrid generate_volume(const Orbital &orbital, double xmin, double xmax,
                           double ymin, double ymax, double zmin, double zmax,
                           int nx, int ny, int nz, int num_threads = 0);

/**
 * @brief Сохраняет VolumeGrid в VTK Legacy ASCII (Structured Points).
 * @param grid Сетка.
 * @param filename Имя выходного файла.
 * @param include_phase Добавлять ли фазовый скалярный массив.
 * @return true в случае успеха.
 */
bool write_vtk_legacy(const VolumeGrid &grid, const std::string &filename,
                      bool include_phase = false);

/**
 * @brief Сохраняет VolumeGrid как сырой массив float (little-endian) с
 * MetaImage-заголовком (.mhd).
 * @param grid Сетка.
 * @param raw_filename Имя .raw файла.
 * @param mhd_filename Имя .mhd файла (если пусто, заменяется на raw_filename +
 * ".mhd").
 * @return true в случае успеха.
 */
bool write_raw_mhd(const VolumeGrid &grid, const std::string &raw_filename,
                   const std::string &mhd_filename = "");

#endif