#include <cmath>

#include "implot.h"

struct App {
  float w, h;
};

template <typename T>
inline T RandomRange(T min, T max) {
    T scale = rand() / (T) RAND_MAX;
    return min + scale * ( max - min );
}

void Demo_ShadedPlots() {
    static float xs[1001], ys[1001], ys1[1001], ys2[1001], ys3[1001], ys4[1001];
    srand(0);
    for (int i = 0; i < 1001; ++i) {
        xs[i]  = i * 0.001f;
        ys[i]  = 0.25f + 0.25f * sinf(25 * xs[i]) * sinf(5 * xs[i]) + RandomRange(-0.01f, 0.01f);
        ys1[i] = ys[i] + RandomRange(0.1f, 0.12f);
        ys2[i] = ys[i] - RandomRange(0.1f, 0.12f);
        ys3[i] = 0.75f + 0.2f * sinf(25 * xs[i]);
        ys4[i] = 0.75f + 0.1f * cosf(25 * xs[i]);
    }
    static float alpha = 0.25f;
    ImGui::DragFloat("Alpha",&alpha,0.01f,0,1);

    if (ImPlot::BeginPlot("Shaded Plots")) {
        ImPlot::PushStyleVar(ImPlotStyleVar_FillAlpha, alpha);
        ImPlot::PlotShaded("Uncertain Data",xs,ys1,ys2,1001);
        ImPlot::PlotLine("Uncertain Data", xs, ys, 1001);
        ImPlot::PlotShaded("Overlapping",xs,ys3,ys4,1001);
        ImPlot::PlotLine("Overlapping",xs,ys3,1001);
        ImPlot::PlotLine("Overlapping",xs,ys4,1001);
        ImPlot::PopStyleVar();
        ImPlot::EndPlot();
    }
}

void AppInit(App& app, float w, float h) {
    app = {};
    app.w = w;
    app.h = h;
}

void AppUpdateAndRender(App& app) {
    auto flags = ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoScrollbar;
    ImGui::SetNextWindowSize(ImVec2(app.w, app.h));
    ImGui::SetNextWindowPos(ImVec2(0.0f, 0.0f));
    ImGui::Begin("Function Plotter", nullptr, flags);

    Demo_ShadedPlots();

    ImGui::End();
}

