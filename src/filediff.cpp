#include <cmath>
#include <cstdio>
#include <memory>
#include <string>
#include <unordered_set>
#include <vector>

#include "implot.h"
#include "imgui/misc/cpp/imgui_stdlib.h"

using namespace std;

struct App {
  float w, h;

  string leftPath;
  string rightPath;

  vector<string> leftLines;
  vector<string> rightLines;

  unordered_set<size_t> diffs;
};

void SaveLines(std::vector<std::string> &lines, std::string &path) {
  FILE* f = fopen(path.data(), "w");
  if (!f) {
    perror("SaveLines: fopen: ");
    return;
  }

  for (string const& line : lines) {
    size_t n = fwrite(line.data(), 1, line.size(), f);
    size_t one = fwrite("\n", 1, 1, f);

    if (n + one != line.size() + 1) {
      perror("SaveLines: fwrite: ");
      break;
    }
  }

  fclose(f);
}

std::vector<std::string> LoadLines(std::string &path) {
  vector<string> lines = {};

  FILE* f = fopen(path.data(), "r");
  if (!f) {
    perror("LoadLines: fopen: ");
    return lines;
  }

  if (fseek(f, 0, SEEK_END) == -1) {
    perror("LoadLines: fseek: ");
    return lines;
  }

  size_t fileSize = ftell(f);
  if (fileSize == -1) {
    perror("LoadLines: ftell: ");
    return lines;
  }

  rewind(f);

  char* bytes = (char*)calloc(1, fileSize + 1);
  if (fread(bytes, 1, fileSize, f) != fileSize) {
    perror("LoadLines: fread: ");
  } else {
    size_t lineStart = 0;
    for (size_t i = 0; i < fileSize; ++i) {
      char c = bytes[i];
      if (c == '\n') {
        lines.emplace_back(&bytes[lineStart], i - lineStart);
        lineStart = i + 1;
      }
    }
  }

  free(bytes);
  fclose(f);

  return lines;
}

void ComputeDiffs(App & app) {
  app.diffs.clear();

  size_t minSize = min(app.leftLines.size(), app.rightLines.size());
  size_t maxSize = max(app.leftLines.size(), app.rightLines.size());

  for (size_t i = 0; i < minSize; ++i) {
    if (app.leftLines[i] != app.rightLines[i]) {
      app.diffs.insert(i);
    }
  }

  for (size_t i = minSize; i < maxSize; ++i) {
    app.diffs.insert(i);
  }
}

void DrawSelection(App & app) {
  ImGui::InputText("Left", &app.leftPath);
  ImGui::SameLine();
  if (ImGui::Button("Save###Left"))
    SaveLines(app.leftLines, app.leftPath);

  ImGui::InputText("Right", &app.rightPath);
  ImGui::SameLine();
  if (ImGui::Button("Save###Right"))
    SaveLines(app.rightLines, app.rightPath);

  if (ImGui::Button("Compare")) {
    app.leftLines = std::move(LoadLines(app.leftPath));
    app.rightLines = std::move(LoadLines(app.rightPath));
    ComputeDiffs(app);
  }
}

void DrawDiffView(App & app) {
  auto RED = ImVec4(1.0f, 0.0f, 0.0f, 1.0f);

  auto parentSize = ImVec2(ImGui::GetContentRegionAvail().x, ImGui::GetContentRegionAvail().y - 30.0f);
  auto swapSize = ImVec2(40.0f, parentSize.y);
  auto childSize = ImVec2((parentSize.x - swapSize.x) * 0.5, parentSize.y);

  ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));

  ImGui::BeginChild("Parent", parentSize, true);

  if (ImGui::BeginChild("LeftDiff", childSize, false)) {
    for (size_t i = 0; i < app.leftLines.size(); ++i) {
      const char* line = app.leftLines[i].data();
      if (app.diffs.contains(i)) {
         ImGui::TextColored(RED, "%s", line);
      } else {
         ImGui::Text("%s", line);
      }
    }
  }
  ImGui::EndChild();

  ImGui::SameLine();

  float lineHeight = ImGui::GetTextLineHeight();
  auto buttonSize = ImVec2(15.0f, lineHeight);

  if (ImGui::BeginChild("Swap", swapSize, true)) {
    size_t maxSize = max(app.leftLines.size(), app.rightLines.size());
    for (size_t i = 0; i < maxSize; ++i) {
      if (app.diffs.contains(i)) {
        char leftLabel[32] = {0};
        char rightLabel[32] = {0};
        snprintf(&leftLabel[0], 31, "<##%zu", i);
        snprintf(&rightLabel[0], 31, ">##%zu", i);

        if (i < app.rightLines.size()) {
          if (ImGui::Button(&leftLabel[0], buttonSize)) {
              if (i < app.leftLines.size()) app.leftLines[i] = app.rightLines[i];
              else app.leftLines.push_back(app.rightLines[i]);
              ComputeDiffs(app);
          }
        } else {
          ImGui::Dummy(buttonSize);
        }

        ImGui::SameLine();

        if (i < app.leftLines.size()) {
          if (ImGui::Button(&rightLabel[0], buttonSize)) {
              if (i < app.rightLines.size()) app.rightLines[i] = app.leftLines[i];
              else app.rightLines.push_back(app.leftLines[i]);
              ComputeDiffs(app);
          }
        } else {
          ImGui::Dummy(buttonSize);
        }

      } else {
          ImGui::Dummy(buttonSize);
      }
    }
  }
  ImGui::EndChild();
  ImGui::SameLine();

  if (ImGui::BeginChild("RightDiff", childSize, false)) {
    for (size_t i = 0; i < app.rightLines.size(); ++i) {
      const char* line = app.rightLines[i].data();
      if (app.diffs.contains(i)) {
         ImGui::TextColored(RED, "%s", line);
      } else {
         ImGui::Text("%s", line);
      }
    }
  }
  ImGui::EndChild();
  ImGui::EndChild();
  ImGui::PopStyleVar();
}

void DrawStats(App & app) {
  ImGui::SetCursorPosY(ImGui::GetWindowHeight() - 20.0f);
  ImGui::Text("Diff lines count: %zu", app.diffs.size());
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
    ImGui::Begin("File Differ", nullptr, flags);

    DrawSelection(app);
    DrawDiffView(app);
    DrawStats(app);

    ImGui::End();
}


