#include <cstdint>
#include <cmath>
#include <cstdio>

#include "implot.h"

using namespace std;

struct MyColor {
  uint8_t r, g, b, a;
  auto operator<=>(const MyColor&) const = default;
};

MyColor WHITE = { 255, 255, 255, 255 };
MyColor RED   = { 255, 0  , 0  , 255 };
MyColor GREEN = { 0  , 255, 0  , 255 };
MyColor BLUE  = { 0  , 0  , 255, 255 };

constexpr size_t CANVAS_WIDTH = 800;
constexpr size_t CANVAS_HEIGHT = 600;
constexpr size_t CANVAS_COUNT = CANVAS_WIDTH*CANVAS_HEIGHT;
typedef MyColor Canvas[CANVAS_HEIGHT][CANVAS_WIDTH];
static Canvas CANVAS = {};

struct App {
  float w, h;

  char savePath[256];

  MyColor brushColor;
  int brushSize;
};

void SaveCanvas(const char* path) {
  FILE* f = fopen(path, "w");
  if (!f) {
    perror("SaveCanvas: fopen: ");
    return;
  }

  size_t n = fwrite(&CANVAS[0], 1, sizeof(CANVAS), f);
  if (n != sizeof(CANVAS)) {
    perror("SaveCanvas: fwrite: ");
  }

  fclose(f);
}

void LoadCanvas(const char* path) {
  FILE* f = fopen(path, "r");
  if (!f) {
    perror("LoadCanvas: fopen: ");
    return;
  }

  if (fseek(f, 0, SEEK_END) == -1) {
    perror("LoadCanvas: fseek: ");
    return;
  }

  size_t fileSize = ftell(f);
  if (fileSize == -1) {
    perror("LoadCanvas: ftell: ");
    return;
  }

  if (fileSize != sizeof(CANVAS)) {
    perror("LoadCanvas: invalid file size");
    return;
  }

  rewind(f);

  if (fread(&CANVAS[0], 1, fileSize, f) != fileSize) {
    perror("LoadCanvas: fread: ");
  }

  fclose(f);
}

void ClearCanvas() {
  memset(&CANVAS[0], 0, sizeof(CANVAS));
}

void DrawMenu(App& app) {
  if (ImGui::Button("Save")) {
    SaveCanvas(&app.savePath[0]);
  }

  ImGui::SameLine();

  if (ImGui::Button("Load")) {
    LoadCanvas(&app.savePath[0]);
  }

  ImGui::SameLine();

  if (ImGui::Button("Clear")) {
    ClearCanvas();
  }

  ImGui::SameLine();

  ImGui::InputText("Save/Load Path", app.savePath, sizeof(app.savePath));
}

void DrawColorPickers(App& app) {
  constexpr auto ORANGE = ImVec4(1.0f, 0.5f, 0.0f, 1.0f);

  bool brushIsWhite = app.brushColor == WHITE;
  bool brushIsRed   = app.brushColor == RED;
  bool brushIsGreen = app.brushColor == GREEN;
  bool brushIsBlue  = app.brushColor == BLUE;

  if (brushIsWhite) ImGui::PushStyleColor(ImGuiCol_Button, ORANGE);
  if (ImGui::Button("White")) app.brushColor = WHITE;
  if (brushIsWhite) ImGui::PopStyleColor();
  ImGui::SameLine();

  if (brushIsRed) ImGui::PushStyleColor(ImGuiCol_Button, ORANGE);
  if (ImGui::Button("Red")) app.brushColor = RED;
  if (brushIsRed) ImGui::PopStyleColor();
  ImGui::SameLine();

  if (brushIsGreen) ImGui::PushStyleColor(ImGuiCol_Button, ORANGE);
  if (ImGui::Button("Green")) app.brushColor = GREEN;
  if (brushIsGreen) ImGui::PopStyleColor();
  ImGui::SameLine();

  if (brushIsBlue) ImGui::PushStyleColor(ImGuiCol_Button, ORANGE);
  if (ImGui::Button("Blue")) app.brushColor = BLUE;
  if (brushIsBlue) ImGui::PopStyleColor();

  ImGui::SliderInt("brush size", &app.brushSize, 1, 50);
}

void DrawSquare(ImVec2 canvasTopLeft, ImVec2 mouse, int brushSize, MyColor brushColor) {
  size_t originX = mouse.x - canvasTopLeft.x - brushSize*0.5;
  size_t originY = mouse.y - canvasTopLeft.y - brushSize*0.5;

  for (size_t y = max(size_t(0), originY); y < min(originY + size_t(brushSize), CANVAS_HEIGHT); ++y) {
    for (size_t x = max(size_t(0), originX); x < min(originX + size_t(brushSize), CANVAS_WIDTH); ++x) {
      CANVAS[y][x] = brushColor;
    }
  }
}

void DrawCanvas(App& app) {
  ImVec2 canvasTopLeft = ImGui::GetCursorScreenPos();
  ImGui::InvisibleButton("##canvas", ImVec2((float)CANVAS_WIDTH, (float)CANVAS_HEIGHT));

  ImVec2 mouse = ImGui::GetMousePos();

  if (ImGui::IsMouseDown(ImGuiMouseButton_Left) &&
      mouse.x >= canvasTopLeft.x && mouse.x <= canvasTopLeft.x + CANVAS_WIDTH &&
      mouse.y >= canvasTopLeft.y && mouse.y <= canvasTopLeft.y + CANVAS_HEIGHT) {
    // fprintf("drawsquare (x=%f, y=%f)\n");
    DrawSquare(canvasTopLeft, mouse, app.brushSize, app.brushColor);
  }

  // TODO: PERF
  ImDrawList* draw_list = ImGui::GetWindowDrawList();

  for (size_t y = 0; y < CANVAS_HEIGHT; ++y) {
    for (size_t x = 0; x < CANVAS_WIDTH; ++x) {
      const MyColor& color = CANVAS[y][x];
      if (color.a == 0) continue;

      ImU32 c = IM_COL32(color.r, color.g, color.b, color.a);

      ImVec2 position = {canvasTopLeft.x + x, canvasTopLeft.y + y};
      ImVec2 position2 = {position.x + 1.0f, position.y + 1.0f};
      draw_list->AddRectFilled(position, position2, c);
    }
  }

  draw_list->AddRect(canvasTopLeft,
                     ImVec2(canvasTopLeft.x + CANVAS_WIDTH, canvasTopLeft.y + CANVAS_HEIGHT),
                     IM_COL32(255, 255, 255, 255),
                     0.0f,
                     0,
                     2.0);
}

void AppInit(App& app, float w, float h) {
    app = {};
    app.w = w;
    app.h = h;

    strncpy(app.savePath, "canvas.bin", sizeof(app.savePath) - 1);

    app.brushSize = 3;
    app.brushColor = WHITE;
}

void AppUpdateAndRender(App& app) {
    auto flags = ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoScrollbar;
    ImGui::SetNextWindowSize(ImVec2(app.w, app.h));
    ImGui::SetNextWindowPos(ImVec2(0.0f, 0.0f));
    ImGui::Begin("My Paint", nullptr, flags);

    DrawMenu(app);
    DrawColorPickers(app);
    DrawCanvas(app);

    ImGui::End();
}

