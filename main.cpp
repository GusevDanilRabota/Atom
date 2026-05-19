#include "multirenderer.h"
#include "orbital.h"
#include "quantum_numbers.h"
#include "renderer.h"
#include "volume_exporter.h"
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <limits>
#include <string>

void print_usage(const char *prog) {
  printf("Использование: %s [опции]\n", prog);
  printf("Опции орбитали:\n");
  printf("  -n N            Главное квантовое число (по умолч. 1)\n");
  printf("  -l L            Орбитальное число (0)\n");
  printf("  -m M            Магнитное число (0)\n");
  printf("  -Z Z            Заряд ядра (1)\n");
  printf("  --real          Действительная орбиталь (по умолчанию)\n");
  printf("  --complex       Комплексная орбиталь с фазой\n");
  printf("Опции одного среза:\n");
  printf("  --xmin=VAL      ( -10 )\n");
  printf("  --xmax=VAL      ( 10 )\n");
  printf("  --zmin=VAL      ( -10 )\n");
  printf("  --zmax=VAL      ( 10 )\n");
  printf("  --width=W       ( 800 )\n");
  printf("  --height=H      ( 800 )\n");
  printf("  --fixed=VAL     Фикс. координата (0)\n");
  printf("  --axis=X|Y|Z    Ось, перпендикулярная плоскости (Y)\n");
  printf("  --colormap=gray|hsv|jet|hot|cool\n");
  printf("  --log           Логарифмическая яркость\n");
  printf("  --logfactor=A   Коэффициент A (100)\n");
  printf("  --format=ppm|bmp (ppm)\n");
  printf("  -o FILE         Имя выходного файла\n");
  printf("Специальные режимы:\n");
  printf("  --volume        Генерация 3D-объёма\n");
  printf("  --volume-size=N Размер воксельной сетки (100)\n");
  printf("  --threads=N     Число потоков (0=auto)\n");
  printf("  --three-view    Три ортопроекции на одном изображении\n");
  printf("  --slice-seq     Серия срезов\n");
  printf("  --start=VAL     Начальное значение фикс. координаты\n");
  printf("  --end=VAL       Конечное значение\n");
  printf("  --steps=N       Количество срезов\n");
  printf("Без аргументов запускается интерактивное меню.\n");
}

int main(int argc, char *argv[]) {
  int n = 1;
  int l = 0;
  int m = 0;
  int Z = 1;
  bool use_real = true;
  RenderParams params;
  bool volume_mode = false;
  int vol_size = 100;
  int threads = 0;
  bool three_view = false;
  bool slice_seq = false;
  char slice_axis = 'z';
  double slice_start = -5.0;
  double slice_end = 5.0;
  int slice_steps = 50;
  std::string output_file = "orbital.ppm";

  if (argc == 1) {
    std::cout << "=== Визуализатор орбиталей ===\n";
    std::cout << "Выберите действие:\n";
    std::cout << "1 - Одиночный срез\n";
    std::cout << "2 - 3D-объём (VTK/RAW)\n";
    std::cout << "3 - Три ортопроекции\n";
    std::cout << "4 - Серия срезов\n";
    std::cout << "Ваш выбор: ";
    int choice = 0;
    std::cin >> choice;
    if (choice < 1 || choice > 4) {
      std::cerr << "Неверный выбор\n";
      return 1;
    }

    std::cout << "Введите n l m Z: ";
    std::cin >> n >> l >> m >> Z;
    std::cout << "Действительная орбиталь? (1-да, 0-нет): ";
    std::cin >> use_real;

    if (choice == 1) {
      std::cout << "Границы X (min max): ";
      std::cin >> params.xmin >> params.xmax;
      std::cout << "Границы Z (min max): ";
      std::cin >> params.zmin >> params.zmax;
      std::cout << "Размер (ширина высота): ";
      std::cin >> params.width >> params.height;
      std::cout << "Ось среза (x/y/z): ";
      std::cin >> params.slice_axis;
      std::cout << "Фикс. координата: ";
      std::cin >> params.fixed_coord;
      std::cout << "Цветовая карта (gray/hsv/jet/hot/cool): ";
      std::string cm;
      std::cin >> cm;
      if (cm == "gray") {
        params.colormap = ColorMap::GRAY;
      } else if (cm == "hsv") {
        params.colormap = ColorMap::HSV_PHASE;
      } else if (cm == "jet") {
        params.colormap = ColorMap::JET;
      } else if (cm == "hot") {
        params.colormap = ColorMap::HOT;
      } else if (cm == "cool") {
        params.colormap = ColorMap::COOL;
      }
      std::cout << "Лог. шкала? (1/0): ";
      std::cin >> params.log_scale;
      if (params.log_scale) {
        std::cout << "logfactor: ";
        std::cin >> params.log_factor;
      }
      std::cout << "Формат (ppm/bmp): ";
      std::string fmt;
      std::cin >> fmt;
      if (fmt == "bmp") {
        params.format = ImageFormat::BMP;
      } else {
        params.format = ImageFormat::PPM;
      }
      std::cout << "Имя файла: ";
      std::cin >> output_file;
      params.output_filename = output_file;
      params.use_real = use_real;
    } else if (choice == 2) {
      volume_mode = true;
      std::cout << "Размер сетки (N): ";
      std::cin >> vol_size;
      std::cout << "Потоков (0-авто): ";
      std::cin >> threads;
      std::cout << "Имя файла (.vtk или .raw): ";
      std::cin >> output_file;
    } else if (choice == 3) {
      three_view = true;
      std::cout << "Размер под-проекции (ширина высота): ";
      std::cin >> params.width >> params.height;
      std::cout << "Границы (+/-): ";
      double b = 0.0;
      std::cin >> b;
      params.xmin = -b;
      params.xmax = b;
      params.zmin = -b;
      params.zmax = b;
      std::cout << "Цветовая карта (gray/jet/hot/cool): ";
      std::string cm;
      std::cin >> cm;
      if (cm == "gray") {
        params.colormap = ColorMap::GRAY;
      } else if (cm == "jet") {
        params.colormap = ColorMap::JET;
      } else if (cm == "hot") {
        params.colormap = ColorMap::HOT;
      } else if (cm == "cool") {
        params.colormap = ColorMap::COOL;
      }
      std::cout << "Имя файла: ";
      std::cin >> output_file;
    } else if (choice == 4) {
      slice_seq = true;
      std::cout << "Ось движения среза (x/y/z): ";
      std::cin >> slice_axis;
      std::cout << "Начало, конец, шагов: ";
      std::cin >> slice_start >> slice_end >> slice_steps;
      std::cout << "Границы по другим осям (min max): ";
      std::cin >> params.xmin >> params.xmax;
      params.zmin = params.xmin;
      params.zmax = params.xmax;
      std::cout << "Размер (ширина высота): ";
      std::cin >> params.width >> params.height;
      std::cout << "Цветовая карта (gray/jet/hot/cool): ";
      std::string cm;
      std::cin >> cm;
      if (cm == "gray") {
        params.colormap = ColorMap::GRAY;
      } else if (cm == "jet") {
        params.colormap = ColorMap::JET;
      } else if (cm == "hot") {
        params.colormap = ColorMap::HOT;
      } else if (cm == "cool") {
        params.colormap = ColorMap::COOL;
      }
      std::cout << "Префикс выходных файлов: ";
      std::cin >> output_file;
    }
  } else {
    for (int i = 1; i < argc; ++i) {
      std::string arg(argv[i]);
      if (arg == "-n" && i + 1 < argc) {
        ++i;
        n = atoi(argv[i]);
      } else if (arg == "-l" && i + 1 < argc) {
        ++i;
        l = atoi(argv[i]);
      } else if (arg == "-m" && i + 1 < argc) {
        ++i;
        m = atoi(argv[i]);
      } else if (arg == "-Z" && i + 1 < argc) {
        ++i;
        Z = atoi(argv[i]);
      } else if (arg == "--real") {
        use_real = true;
      } else if (arg == "--complex") {
        use_real = false;
      } else if (arg.rfind("--xmin=", 0) == 0) {
        params.xmin = atof(arg.substr(7).c_str());
      } else if (arg.rfind("--xmax=", 0) == 0) {
        params.xmax = atof(arg.substr(7).c_str());
      } else if (arg.rfind("--zmin=", 0) == 0) {
        params.zmin = atof(arg.substr(7).c_str());
      } else if (arg.rfind("--zmax=", 0) == 0) {
        params.zmax = atof(arg.substr(7).c_str());
      } else if (arg.rfind("--width=", 0) == 0) {
        params.width = atoi(arg.substr(8).c_str());
      } else if (arg.rfind("--height=", 0) == 0) {
        params.height = atoi(arg.substr(9).c_str());
      } else if (arg.rfind("--fixed=", 0) == 0) {
        params.fixed_coord = atof(arg.substr(8).c_str());
      } else if (arg.rfind("--axis=", 0) == 0) {
        std::string ax = arg.substr(7);
        if (!ax.empty()) {
          params.slice_axis = static_cast<char>(std::toupper(ax[0]));
        }
      } else if (arg.rfind("--colormap=", 0) == 0) {
        std::string cm = arg.substr(11);
        if (cm == "gray") {
          params.colormap = ColorMap::GRAY;
        } else if (cm == "hsv") {
          params.colormap = ColorMap::HSV_PHASE;
        } else if (cm == "jet") {
          params.colormap = ColorMap::JET;
        } else if (cm == "hot") {
          params.colormap = ColorMap::HOT;
        } else if (cm == "cool") {
          params.colormap = ColorMap::COOL;
        }
      } else if (arg == "--log") {
        params.log_scale = true;
      } else if (arg.rfind("--logfactor=", 0) == 0) {
        params.log_factor = atof(arg.substr(12).c_str());
      } else if (arg.rfind("--format=", 0) == 0) {
        std::string fmt = arg.substr(9);
        if (fmt == "ppm") {
          params.format = ImageFormat::PPM;
        } else if (fmt == "bmp") {
          params.format = ImageFormat::BMP;
        }
      } else if (arg == "-o" && i + 1 < argc) {
        ++i;
        output_file = argv[i];
      } else if (arg == "--volume") {
        volume_mode = true;
      } else if (arg.rfind("--volume-size=", 0) == 0) {
        vol_size = atoi(arg.substr(14).c_str());
      } else if (arg.rfind("--threads=", 0) == 0) {
        threads = atoi(arg.substr(10).c_str());
      } else if (arg == "--three-view") {
        three_view = true;
      } else if (arg == "--slice-seq") {
        slice_seq = true;
      } else if (arg.rfind("--start=", 0) == 0) {
        slice_start = atof(arg.substr(8).c_str());
      } else if (arg.rfind("--end=", 0) == 0) {
        slice_end = atof(arg.substr(6).c_str());
      } else if (arg.rfind("--steps=", 0) == 0) {
        slice_steps = atoi(arg.substr(8).c_str());
      } else {
        printf("Неизвестная опция: %s\n", arg.c_str());
        print_usage(argv[0]);
        return 1;
      }
    }
    params.output_filename = output_file;
    params.use_real = use_real;
  }

  try {
    QuantumNumbers qn(n, l, m);
    Orbital orbital(qn, Z);

    if (volume_mode) {
      VolumeGrid grid =
          generate_volume(orbital, -10.0, 10.0, -10.0, 10.0, -10.0, 10.0,
                          vol_size, vol_size, vol_size, threads);
      if (output_file.find(".raw") != std::string::npos) {
        write_raw_mhd(grid, output_file);
      } else {
        write_vtk_legacy(grid, output_file);
      }
      printf("Объём сохранён в %s\n", output_file.c_str());
    } else if (three_view) {
      double bounds = std::max(std::max(params.xmax, -params.xmin),
                               std::max(params.zmax, -params.zmin));
      if (!render_three_ortho(orbital, params.width, params.height, bounds,
                              output_file, params.colormap)) {
        fprintf(stderr,
                "Ошибка при сохранении трёхпроекционного изображения\n");
        return 1;
      }
      printf("Три проекции сохранены в %s\n", output_file.c_str());
    } else if (slice_seq) {
      RenderParams base = params;
      base.output_filename = "";
      base.use_real = use_real;
      if (!render_slice_sequence(orbital, base, slice_axis, slice_start,
                                 slice_end, slice_steps, output_file)) {
        fprintf(stderr, "Ошибка при создании серии срезов\n");
        return 1;
      }
      printf("Серия из %d срезов сохранена с префиксом %s\n", slice_steps,
             output_file.c_str());
    } else {
      params.output_filename = output_file;
      params.use_real = use_real;
      Renderer renderer(orbital, params);
      if (!renderer.render()) {
        fprintf(stderr, "Ошибка при рендеринге изображения\n");
        return 1;
      }
      printf("Изображение сохранено в %s\n", output_file.c_str());
    }
  } catch (const std::exception &e) {
    fprintf(stderr, "Ошибка: %s\n", e.what());
    return 1;
  }

  return 0;
}