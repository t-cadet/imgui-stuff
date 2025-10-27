#include <cstdint>
#include <cmath>
#include <ctime>

#include "implot.h"

using namespace std;
float PI = std::numbers::pi_v<float>;

struct Time {
  int32_t hour;
  int32_t min;
  int32_t sec;
};

struct App {
  float w, h;
};

Time Now() {
  time_t t = time(nullptr);
  struct tm *tm_info = localtime(&t);

  Time now = {};
  now.hour = tm_info->tm_hour;
  now.min  = tm_info->tm_min;
  now.sec  = tm_info->tm_sec;

  return now;
}

void AppInit(App& app, float w, float h) {
    app = {};
    app.w = w;
    app.h = h;
}


void DrawCircle(ImVec2 center, float radius) {
  ImGui::GetWindowDrawList()->AddCircle(center, radius, ImGui::GetColorU32(ImGuiCol_Text), 100, 2.0f);
}

void DrawClockHand(ImVec2 center, float radius, float angle, ImColor color) {
  ImVec2 endPoint = { center.x + cos(angle)*radius, center.y + sin(angle)*radius };
  ImGui::GetWindowDrawList()->AddLine(center, endPoint, color, 3.0f);
}

void DrawAllHourStrokes(ImVec2 center, float radius) {
  for (int32_t hour = 0; hour < 12; ++hour) {
    float angle = (hour * (2.0f * PI) / 12.0f) + 0.5*PI;
    float cosAngle = cos(angle);
    float sinAngle = sin(angle);
    float strokeLen = 0.90f;

    ImVec2 start = { center.x + (strokeLen*radius) * cosAngle, center.y + (strokeLen*radius) * sinAngle };
    ImVec2 end = { center.x + cosAngle*radius, center.y + sinAngle*radius };

    ImGui::GetWindowDrawList()->AddLine(start, end, ImGui::GetColorU32(ImGuiCol_Text), 2.0f);
  }
}

void DrawAllMinuteStrokes(ImVec2 center, float radius) {
  for (int32_t min = 0; min < 60; ++min) {
    float angle = (min * (2.0f * PI) / 60.0f) + 0.5*PI;
    float cosAngle = cos(angle);
    float sinAngle = sin(angle);
    float strokeLen = 0.95f;

    ImVec2 start = { center.x + (strokeLen*radius) * cosAngle, center.y + (strokeLen*radius) * sinAngle };
    ImVec2 end = { center.x + cosAngle*radius, center.y + sinAngle*radius };

    ImGui::GetWindowDrawList()->AddLine(start, end, ImGui::GetColorU32(ImGuiCol_Text), 1.0f);
  }
}

void DrawDigitalClock(int32_t hours, int32_t mins, int32_t secs) {
  ImGui::Text("%02d:%02d:%02d", hours, mins, secs);
}

void DrawClock() {
  float radius = 250.0f;
  ImVec2 cursor = ImGui::GetCursorScreenPos();
  ImVec2 center = { cursor.x + radius, cursor.y + radius };

  DrawCircle(center, radius);

  Time now = Now();

  float seconds_frac = static_cast<float>(now.sec);
  float minutes_frac = static_cast<float>(now.min) + seconds_frac / 60.0f;
  float hours_frac = static_cast<float>(now.hour) + minutes_frac / 60.0f;

  float offset = 0.5*PI;
  float hour_theta = (hours_frac * ((2.0f * PI) / 12.0f)) - offset;
  float minute_theta = (minutes_frac * ((2.0f * PI) / 60.0f)) - offset;
  float second_theta = (seconds_frac * ((2.0f * PI) / 60.0f)) - offset;

  DrawClockHand(center, radius * 0.70f, hour_theta, ImColor(1.0f, 0.0f, 0.0f, 1.0f));
  DrawClockHand(center, radius * 0.80f, minute_theta, ImColor(0.0f, 1.0f, 0.0f, 1.0f));
  DrawClockHand(center, radius * 0.90f, second_theta, ImColor(0.0f, 0.0f, 1.0f, 1.0f));

  DrawAllHourStrokes(center, radius);
  DrawAllMinuteStrokes(center, radius);

  DrawCircle(center, 5.0f);

  DrawDigitalClock(now.hour, now.min, now.sec);
}

void AppUpdateAndRender(App& app) {
    auto flags = ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoScrollbar;
    ImGui::SetNextWindowSize(ImVec2(app.w, app.h));
    ImGui::SetNextWindowPos(ImVec2(0.0f, 0.0f));
    ImGui::Begin("Clock", nullptr, flags);

    DrawClock();

    ImGui::End();
}

