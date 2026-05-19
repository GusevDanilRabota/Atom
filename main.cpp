#include "orbital.h"
#include "quantum_numbers.h"
#include "renderer.h"
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>

void print_usage(const char *prog) {
  printf("Usage: %s -n N -l L -m M [options]\n", prog);
  printf("Options:\n");
  printf("  --xmin=VAL        (default -10)\n");
  printf("  --xmax=VAL        (default 10)\n");
  printf("  --zmin=VAL        (default -10)\n");
  printf("  --zmax=VAL        (default 10)\n");
  printf("  --width=W         (default 800)\n");
  printf("  --height=H        (default 800)\n");
  printf(
      "  --fixed=VAL       fixed coordinate for slicing plane (default 0)\n");
  printf("  --axis=X|Y|Z      axis perpendicular to slice (default Y => XZ "
         "plane)\n");
  printf("  --real            use real (chemical) orbital (default)\n");
  printf("  --complex         use complex orbital with phase coloring\n");
  printf("  --colorscheme=gray|hsv   (gray default)\n");
  printf("  --log             enable logarithmic brightness scale\n");
  printf("  --logfactor=A     factor for log (default 100)\n");
  printf("  --format=ppm|bmp  (default ppm)\n");
  printf("  -o FILENAME       output file name\n");
}

int main(int argc, char *argv[]) {
  // Исправлено: каждое объявление отдельно
  int n = 1;
  int l = 0;
  int m = 0;
  RenderParams params;
  bool use_real = true;

  for (int i = 1; i < argc; ++i) {
    std::string arg(argv[i]);
    if (arg == "-n" && i + 1 < argc) {
      n = atoi(argv[++i]);
    } else if (arg == "-l" && i + 1 < argc) {
      l = atoi(argv[++i]);
    } else if (arg == "-m" && i + 1 < argc) {
      m = atoi(argv[++i]);
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
        params.slice_axis = std::toupper(ax[0]);
      }
    } else if (arg == "--real") {
      use_real = true;
    } else if (arg == "--complex") {
      use_real = false;
    } else if (arg.rfind("--colorscheme=", 0) == 0) {
      std::string cs = arg.substr(14);
      if (cs == "gray") {
        params.colorscheme = ColorScheme::GRAY;
      } else if (cs == "hsv") {
        params.colorscheme = ColorScheme::HSV_PHASE;
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
      params.output_filename = argv[++i];
    } else {
      printf("Unknown option: %s\n", arg.c_str());
      print_usage(argv[0]);
      return 1;
    }
  }

  params.use_real = use_real;

  try {
    QuantumNumbers qn(n, l, m);
    Orbital orbital(qn);
    Renderer renderer(orbital, params);
    if (!renderer.render()) {
      fprintf(stderr, "Error writing image.\n");
      return 1;
    }
    printf("Image saved to %s\n", params.output_filename.c_str());
  } catch (const std::exception &e) {
    fprintf(stderr, "Error: %s\n", e.what());
    return 1;
  }
  return 0;
}