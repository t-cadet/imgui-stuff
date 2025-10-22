#include <cstdint>
#include <cstdio>
#include <cstdlib>

#include <vector>

#include "implot.h"

using namespace std;

size_t MAX_ROW_COUNT = 30;
size_t MAX_COL_COUNT = 30;
const char* SAVE_PATH = "csvtool.csv";

auto POPUP_FLAGS = ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoScrollbar;

struct App {
  float w, h;

  vector<vector<float>> table;

  size_t clickedRow;
  size_t clickedCol;
};

void AppInit(App& app, float w, float h) {
    app = {};
    app.w = w;
    app.h = h;
}

int fsize(FILE* f) {
  if (fseek(f, 0, SEEK_END) == -1) {
    return -1;
  }
  int size = ftell(f);
  if (size < 0) return -1;
  rewind(f);

  return size;
}

bool IsDigit(char c) { return c >= '0' && c <= '9'; }

void DrawSizeButtons(App& app) {
  int rowCount = app.table.size();
  int colCount = app.table.empty() ? 0 : app.table[0].size();

  ImGui::Text("Num Rows: ");
  ImGui::SameLine();
  ImGui::SliderInt("##numRows", &rowCount, 0, MAX_ROW_COUNT);
  ImGui::SameLine();
  if (ImGui::Button("Add Row")) ++rowCount;
  ImGui::SameLine();
  if (ImGui::Button("Drop Row")) --rowCount;

  ImGui::Text("Num Cols: ");
  ImGui::SameLine();
  ImGui::SliderInt("##numCols", &colCount, 0, MAX_COL_COUNT);
  ImGui::SameLine();
  if (ImGui::Button("Add Col")) ++colCount;
  ImGui::SameLine();
  if (ImGui::Button("Drop Col")) --colCount;

  app.table.resize(rowCount);
  for (auto& row : app.table)
    row.resize(colCount);
}

void DrawIoButtons(App& app) {
  if (ImGui::Button("Save")) {
    FILE* f = fopen(SAVE_PATH, "w");
    if (f) {
      for (auto& row : app.table) {
        for (float value : row) {
          char buffer[128] = {};
          snprintf(&buffer[0], sizeof(buffer) - 1, "%f", value);
          assert(fwrite(buffer, 1, strlen(buffer), f) == strlen(buffer));
          assert(fwrite(",", 1, 1, f) == 1);
        }
        assert(fwrite("\n", 1, 1, f) == 1);
      }
      fclose(f);
    }
  }

  ImGui::SameLine();

  if (ImGui::Button("Load")) {
    FILE* f = fopen(SAVE_PATH, "r");
    if (f) {
      vector<vector<float>> newTable = {};
      newTable.push_back({});

      int size = fsize(f);
      assert(size != -1);
      vector<char> buffer(size+1, 0);
      if (fread(buffer.data(), 1, size, f) == size) {
        int floatStart = 0;
        for (int i = 0; i < size; ++i) {
          char c = buffer[i];
          if (c == ',' || c == '\n') {
            if (i - floatStart > 0) {
              buffer[i] = 0;
              float value = stof(&buffer[floatStart]);
              buffer[i] = c;
              newTable.back().push_back(value);
            }

            floatStart = i + 1;
            if (c == '\n' && i != size - 1) {
              newTable.push_back({});
            }
          } else if (!IsDigit(c) && c != '.' && c != '-' && c != '+') {
            fprintf(stderr, "can't read csv file: found non-numbers: %c\n", c);
          }
        }

        size_t colCount = newTable[0].size();
        for (auto& row : newTable) {
          assert(row.size() == colCount);
        }

        app.table = std::move(newTable);
      }

      fclose(f);
    }
  }

  ImGui::SameLine();

  if (ImGui::Button("Clear")) {
    app.table.clear();
  }
}

void DrawValuePopup(App& app) {
  static ImVec2 popUpButtonSize = {120.0f, 0.0f};
  static ImVec2 popUpSize = {300.0f, 100.0f};
  static ImVec2 popUpPos = {0.5f*(1280.0f - popUpSize.x), 0.5f*(720.0f - popUpSize.y)};

  static char buffer[64] = {};

  ImGui::SetNextWindowSize(popUpSize);
  ImGui::SetNextWindowPos(popUpPos);

  if (ImGui::BeginPopupModal("Change Value", nullptr, POPUP_FLAGS)) {
    char labelBuffer[64] = {};
    snprintf(labelBuffer, sizeof(labelBuffer) - 1, "##%zu_%zu", app.clickedRow, app.clickedCol);
    ImGui::InputText(labelBuffer, buffer, sizeof(buffer));

    if (ImGui::Button("Save", popUpButtonSize)) {
      app.table[app.clickedRow][app.clickedCol] = stof(buffer);
      ImGui::CloseCurrentPopup();
    }

    ImGui::SameLine();

    if (ImGui::Button("Cancel", popUpButtonSize) || ImGui::IsKeyPressed(ImGuiKey_Escape)) {
      ImGui::CloseCurrentPopup();
    }

    ImGui::EndPopup();
  }
}

void DrawTable(App& app) {
  static auto tableFlags = ImGuiTableFlags_BordersV | ImGuiTableFlags_BordersOuter;

  if (app.table.empty()) return;
  if (app.table[0].empty()) return;

  size_t colCount = app.table[0].size();
  ImGui::BeginTable("Table", colCount, tableFlags);

  assert(colCount < 127);
  for (char i = 0; i < colCount; ++i) {
    char colName[2] = {};
    colName[0] = 'A' + i;
    ImGui::TableSetupColumn(&colName[0], ImGuiTableColumnFlags_WidthFixed, 1280.0f / (float)(colCount));
  }

  ImGui::TableNextRow(ImGuiTableRowFlags_Headers);

  for (char i = 0; i < colCount; ++i) {
    ImGui::TableNextColumn();
    ImGui::Text("%c", 'A' + i);
  }

  for (size_t row = 0; row < app.table.size(); ++row) {
    for (size_t col = 0; col < app.table[0].size(); ++col) {
      ImGui::TableNextColumn();
      ImGui::Text("%f", app.table[row][col]);
      if (ImGui::IsItemClicked()) {
        ImGui::OpenPopup("Change Value");
        app.clickedRow = row;
        app.clickedCol = col;
      } else if (ImGui::IsItemHovered()) {
        ImGui::SetTooltip("Cell: (%zu, %zu)", row, col);
      }
    }
    ImGui::TableNextRow();
  }

  DrawValuePopup(app);

  ImGui::EndTable();
}

void AppUpdateAndRender(App& app) {
    auto flags = ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoScrollbar;
    ImGui::SetNextWindowSize(ImVec2(app.w, app.h));
    ImGui::SetNextWindowPos(ImVec2(0.0f, 0.0f));
    ImGui::Begin("Csv Tool", nullptr, flags);

    DrawSizeButtons(app);
    ImGui::Separator();
    DrawIoButtons(app);
    ImGui::Separator();
    DrawTable(app);

    ImGui::End();
}

