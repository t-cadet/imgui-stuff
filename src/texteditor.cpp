#include <cstdio>
#include <filesystem>
namespace fs = std::filesystem;

#include "implot.h"

constexpr size_t BUFFER_CAPACITY = 4096;
constexpr ImVec2 POPUP_SIZE = { 300.0f, 100.0f };
constexpr ImVec2 POPUP_BUTTON_SIZE = { 120.0f, 0.0f };
constexpr int POPUP_FLAGS = ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoScrollbar;

struct App {
  float w, h;

  fs::path currentFile;
  char buffer[BUFFER_CAPACITY];

  bool showOpenPopup;
  bool showSavePopup;
};

void SavePopup(App& app) {
  ImGui::OpenPopup("Save File");
  float w = (app.w - POPUP_SIZE.x) * 0.5;
  float h = (app.h - POPUP_SIZE.y) * 0.5;
  ImGui::SetNextWindowSize(POPUP_SIZE);
  ImGui::SetNextWindowPos(ImVec2(w, h));
  if (ImGui::BeginPopupModal("Save File")) {
    static char filenameBuffer[1024] = {};
    ImGui::Text("Save As: ");
    ImGui::InputText("###newName", filenameBuffer, sizeof(filenameBuffer));

    fs::path savePath = fs::current_path() / filenameBuffer;
    if (ImGui::Button("Save")) {
      FILE* f = fopen(savePath.string().c_str(), "w");
      if (f) {
        if (fwrite(app.buffer, 1, BUFFER_CAPACITY, f) == BUFFER_CAPACITY) {
          app.showSavePopup = false;
          app.currentFile = savePath;
          memset(filenameBuffer, 0, sizeof(filenameBuffer));
        }
        fclose(f);
      }
    }

    ImGui::SameLine();

    if (ImGui::Button("Cancel")) {
      app.showSavePopup = false;
      memset(filenameBuffer, 0, sizeof(filenameBuffer));
    }

    ImGui::EndPopup();
  }
}

void OpenPopup(App& app) {
  ImGui::OpenPopup("Open File");
  float w = (app.w - POPUP_SIZE.x) * 0.5;
  float h = (app.h - POPUP_SIZE.y) * 0.5;
  ImGui::SetNextWindowSize(POPUP_SIZE);
  ImGui::SetNextWindowPos(ImVec2(w, h));
  if (ImGui::BeginPopupModal("Open File")) {
    static char filenameBuffer[1024] = {};
    ImGui::Text("Path: ");
    ImGui::InputText("###newName", filenameBuffer, sizeof(filenameBuffer));

    fs::path openPath = fs::current_path() / filenameBuffer;
    if (ImGui::Button("Open")) {
      FILE* f = fopen(openPath.string().c_str(), "a+");
      if (f) {
        size_t readBytesCount = fread(app.buffer, 1, BUFFER_CAPACITY, f);
        app.currentFile = openPath;
        if (readBytesCount >= 0) {
          memset(app.buffer + readBytesCount, 0, BUFFER_CAPACITY - readBytesCount);
          memset(filenameBuffer, 0, sizeof(filenameBuffer));
        }
        app.showOpenPopup = false;
        fclose(f);
      }
    }

    ImGui::SameLine();

    if (ImGui::Button("Cancel")) {
      app.showOpenPopup = false;
      memset(filenameBuffer, 0, sizeof(filenameBuffer));
    }

    ImGui::EndPopup();
  }
}

void Menu(App& app) {
  #define IS_KEY_PRESSED(key) (ImGui::IsKeyPressed(ImGuiKey_##key))
  #define CTRL(key) (ImGui::GetIO().KeyCtrl && IS_KEY_PRESSED(key))

  if (ImGui::Button("Open") || CTRL(O) || app.showOpenPopup) {
    app.showOpenPopup = true;
    OpenPopup(app);
  }

  ImGui::SameLine();

  if (ImGui::Button("Save") || CTRL(S) || app.showSavePopup) {
    app.showSavePopup = true;
    SavePopup(app);
  }

  ImGui::SameLine();
  if (ImGui::Button("Clear")) {
    memset(app.buffer, 0, BUFFER_CAPACITY);
  }

  #undef IS_KEY_PRESSED
  #undef CTRL
}

void Content(App& app) {
  int textInputFlags = ImGuiInputTextFlags_AllowTabInput | ImGuiInputTextFlags_NoHorizontalScroll;
  ImVec2 textInputSize = { app.w - 35.0f, app.h - 85.0f };
  ImVec2 textLineCountSize  = { 15.0f, textInputSize.y };

  ImGui::BeginChild("###textLineCountSize", textLineCountSize);
    size_t lineCount = 1;
    for (size_t i = 0; i < BUFFER_CAPACITY && app.buffer[i]; ++i) {
      if (app.buffer[i] == '\n') {
        ImGui::Text("%zu", lineCount);
        lineCount++;
      }
    }
    ImGui::Text("%zu", lineCount);
  ImGui::EndChild();

  ImGui::SameLine();

  ImGui::InputTextMultiline("###textInput", app.buffer, BUFFER_CAPACITY, textInputSize, textInputFlags);
}

void Info(App& app) {
  if (app.currentFile.empty()) {
    ImGui::Text("File: N/A | Extension: N/A");
  } else {
    ImGui::Text("File: %s | Extension: %s", app.currentFile.string().c_str(), app.currentFile.extension().string().c_str());
  }
}

void AppInit(App& app, float w, float h) {
    app = {};
    app.w = w;
    app.h = h;
    memset(app.buffer, 0, BUFFER_CAPACITY);
}

void AppUpdateAndRender(App& app) {
    auto flags = ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoScrollbar;
    ImGui::SetNextWindowSize(ImVec2(app.w, app.h));
    ImGui::SetNextWindowPos(ImVec2(0.0f, 0.0f));
    ImGui::Begin("Text Editor", nullptr, flags);

    Menu(app);
    ImGui::Separator();
    Content(app);
    ImGui::SetCursorPosY(ImGui::GetWindowHeight() - 25.0f);
    ImGui::Separator();
    Info(app);

    ImGui::End();
}
