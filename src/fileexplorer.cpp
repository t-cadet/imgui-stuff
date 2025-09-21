#include <filesystem>
namespace fs = std::filesystem;

struct App {
  float w, h;

  fs::path cwd;
  fs::path selectedPath;
  char extensionFilter[16];

  bool shouldShowRenamePopup;
  bool shouldShowDeletePopup;
};

void RenameFilePopup(App& app) {
  ImGui::OpenPopup("Rename File");
  if (ImGui::BeginPopupModal("Rename File")) {
    static char filenameBuffer[1024] = {};
    ImGui::Text("New Name: ");
    ImGui::InputText("###newName", filenameBuffer, sizeof(filenameBuffer));

    fs::path newPath = app.selectedPath.parent_path() / filenameBuffer;
    if (ImGui::Button("Rename")) {
      if (rename(app.selectedPath.string().c_str(), newPath.string().c_str()) == 0) {
        app.selectedPath = newPath;
        app.shouldShowRenamePopup = false;
        memset(filenameBuffer, 0, sizeof(filenameBuffer));
      }
    }

    ImGui::SameLine();

    if (ImGui::Button("Cancel")) {
      app.shouldShowRenamePopup = false;
      memset(filenameBuffer, 0, sizeof(filenameBuffer));
    }

    ImGui::EndPopup();
  }
}

void DeleteFilePopup(App& app) {
  ImGui::OpenPopup("Delete File");
  if (ImGui::BeginPopupModal("Delete File")) {
    ImGui::Text("Are you sure you want to delete %s?", app.selectedPath.filename().string().c_str());

    if (ImGui::Button("Yes")) {
      if (fs::remove(app.selectedPath.string().c_str())) {
        app.selectedPath.clear();
        app.shouldShowDeletePopup = false;
      }
    }

    ImGui::SameLine();

    if (ImGui::Button("No")) {
      app.shouldShowDeletePopup = false;
    }

    ImGui::EndPopup();
  }
}

void OpenFileWithDefaultEditor(fs::path const& filepath) {
#ifdef _WIN32
    const auto command = "start \"\" \"" + filepath.string() + "\"";
#elif __APPLE__
    const auto command = "open \"" + filepath.string() + "\"";
#else
    const auto command = "xdg-open \"" + filepath.string() + "\"";
#endif
    auto _ = std::system(command.c_str());
}

void Menu(App& app) {
  if (ImGui::Button("Go Up")) {
    if (app.cwd.has_parent_path()) {
      app.cwd = app.cwd.parent_path();
    }
  }

  ImGui::SameLine();
  ImGui::Text("Current Directory: %s", app.cwd.c_str());
}

void Content(App& app) {
  for (const auto& entry : fs::directory_iterator(app.cwd)) {
    bool isDirectory = entry.is_directory();
    bool isFile = entry.is_regular_file();
    bool isSelected = entry.path() == app.selectedPath;

    string entryName = "";
    if (isDirectory) {
      entryName = "[D] " + entry.path().filename().string();
    } else if (isFile) {
      entryName = "[F] " + entry.path().filename().string();
    }

    if (ImGui::Selectable(entryName.c_str(), isSelected)) {
      app.selectedPath = entry.path();
      if (isDirectory) {
        app.cwd /= app.selectedPath;
      }
    }
  }
}

void Actions(App& app) {
  if (fs::is_directory(app.selectedPath)) {
    ImGui::Text("Selected Directory: %s", app.selectedPath.string().c_str());
  } else if (fs::is_regular_file(app.selectedPath)) {
    ImGui::Text("Selected File: %s", app.selectedPath.string().c_str());
  } else {
    ImGui::Text("Nothing Selected!");
    // Draw invisible button to add Y offset
    ImGui::PushStyleVar(ImGuiStyleVar_Alpha, 0.0f);
    ImGui::Button("invisible button");
    ImGui::PopStyleVar();
    return;
  }

  if (ImGui::Button("Open") && fs::is_regular_file(app.selectedPath)) {
    OpenFileWithDefaultEditor(app.selectedPath);
  }

  ImGui::SameLine();

  if (ImGui::Button("Rename") || app.shouldShowRenamePopup) {
    app.shouldShowRenamePopup = true;
    RenameFilePopup(app);
  }

  ImGui::SameLine();

  if (ImGui::Button("Delete") || app.shouldShowDeletePopup) {
    app.shouldShowDeletePopup = true;
    DeleteFilePopup(app);
  }
}

void Filter(App& app) {
  ImGui::InputText("Filter by Extension", app.extensionFilter, sizeof(app.extensionFilter));

  int matchCount = 0;
  if (strlen(app.extensionFilter) > 0)
    for (const auto& entry : fs::directory_iterator(app.cwd))
      if (entry.is_regular_file())
        if (entry.path().extension().string() == app.extensionFilter)
          ++matchCount;

  ImGui::Text("Match Count: %i", matchCount);
}

void AppInit(App& app, float w, float h) {
    app = {};
    app.w = w;
    app.h = h;
    app.cwd = fs::current_path();
}

void AppUpdateAndRender(App& app) {
    auto flags = ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoScrollbar;
    ImGui::SetNextWindowSize(ImVec2(app.w, app.h));
    ImGui::SetNextWindowPos(ImVec2(0.0f, 0.0f));
    ImGui::Begin("File Explorer", nullptr, flags);

    Menu(app);
    ImGui::Separator();
    Content(app);
    ImGui::SetCursorPosY(ImGui::GetWindowHeight() - 100.0f);
    ImGui::Separator();
    Actions(app);
    ImGui::Separator();
    Filter(app);

    ImGui::End();
}

