void App() {
    auto flags = ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoScrollbar;
    ImGui::SetNextWindowSize(ImVec2(W, H));
    ImGui::SetNextWindowPos(ImVec2(0.0f, 0.0f));
    ImGui::Begin("Hello", nullptr, flags);
    ImGui::End();
}
