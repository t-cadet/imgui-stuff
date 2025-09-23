#include <array>
#include <cmath>
#include <vector>

#include "implot.h"

enum FunctionId {
  None = 0,
  Sin,
  Cos,
  Sq,
};

struct FunctionPlot {
  FunctionId id;
  const char* label;
  bool selected;

  vector<float> xs;
  vector<float> ys;

  void computePoints(float start, float end, size_t pointCount) {
    xs.reserve(pointCount);
    ys.reserve(pointCount);

    float dist = (end - start) / ((float)pointCount - 1.0f);
    for (size_t i = 0; i < pointCount; ++i) {
      float x = start + i*dist;
      xs.push_back(x);
      switch (id) {
        case Sin: ys.push_back(sin(x)); break;
        case Cos: ys.push_back(cos(x)); break;
        case Sq : ys.push_back(x*x); break;
        default: break;
      }
    }
  }
};

struct App {
  float w, h;
  array<FunctionPlot, 3> plots;
};

void AppInit(App& app, float w, float h) {
    app = {};
    app.w = w;
    app.h = h;

    app.plots[0] = FunctionPlot { Sin, "sin(x)", false };
    app.plots[1] = FunctionPlot { Cos, "cos(x)", false };
    app.plots[2] = FunctionPlot { Sq , "sq(x)" , false };
}

void AppUpdateAndRender(App& app) {
    auto flags = ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoScrollbar;
    ImGui::SetNextWindowSize(ImVec2(app.w, app.h));
    ImGui::SetNextWindowPos(ImVec2(0.0f, 0.0f));
    ImGui::Begin("Function Plotter", nullptr, flags);

    constexpr float start = -100.0;
    constexpr float end = 100.0;
    constexpr size_t POINT_COUNT = 10000;

    for (auto& plot : app.plots) {
      if (ImGui::Checkbox(plot.label, &plot.selected)) {
        if (plot.xs.empty()) {
          plot.computePoints(start, end, POINT_COUNT);
        }
      }
    }

    ImPlot::BeginPlot("###plot", ImVec2(-1.0f, -1.0f));
    for (auto& plot : app.plots) {
      if (plot.selected) {
        ImPlot::PlotLine(plot.label, plot.xs.data(), plot.ys.data(), plot.xs.size());
      }
    }
    ImPlot::EndPlot();

    ImGui::End();
}

