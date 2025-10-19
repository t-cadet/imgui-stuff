#include <cstdint>
#include <cmath>
#include <cstdio>
#include <ctime>

#include <map>
#include <string>
#include <vector>

#include "implot.h"

using namespace std;

int16_t MIN_YEAR = 2000;
int16_t MAX_YEAR = 2038;
const char* SAVE_PATH = "calendar.bin";

struct Date {
  int16_t year;
  int8_t month;
  int8_t day;

  auto operator<=>(const Date&) const = default;
};

struct App {
  float w, h;

  Date selectedDate;
  map<Date, vector<string>> meetings;

  bool addMeetingWindowOpen;
};

int DaysInMonth(int8_t month, int16_t year) {
  assert(month >= 1 && month <= 12);
  if (month == 2) {
    bool isLeapYear = (year % 4 == 0 && (year % 100 != 0 || year % 400 == 0));
    return isLeapYear ? 29 : 28;
  } else {
    return (month % 2) ? 31 : 30;
  }
}

const char* MonthToShortString(int8_t month) {
  assert(month >= 1 && month <= 12);
  switch (month) {
    case 1 : return "Jan"; break;
    case 2 : return "Feb"; break;
    case 3 : return "Mar"; break;
    case 4 : return "Apr"; break;
    case 5 : return "May"; break;
    case 6 : return "Jun"; break;
    case 7 : return "Jul"; break;
    case 8 : return "Aug"; break;
    case 9 : return "Sep"; break;
    case 10: return "Oct"; break;
    case 11: return "Nov"; break;
    case 12: return "Dec"; break;
    default: return "Err"; break;
  }
}

Date Today() {
  time_t t = time(nullptr);
  struct tm *tm_info = localtime(&t);

  Date today = {};
  today.year  = tm_info->tm_year + 1900;
  today.month = tm_info->tm_mon + 1;
  today.day   = tm_info->tm_mday;

  return today;
}

int fsize(FILE* f) {
  if (fseek(f, 0, SEEK_END) == -1) {
    return -1;
  }
  int size = ftell(f);
  rewind(f);

  return size;
}

struct Reader {
  char* bytes;
  ssize_t size;
  const char* error;
};

int64_t Read_int64_t(Reader &r) {
  int64_t out = 0;
  if (r.error) return out;

  if (r.bytes && sizeof(int64_t) <= r.size) {
    out = *(int64_t*)(r.bytes);
    r.bytes += sizeof(int64_t);
    r.size -= sizeof(int64_t);
  } else {
    r.error = "Read_int64_t: not enough bytes";
  }

  return out;
}

Date Read_Date(Reader &r) {
  Date out = {};
  if (r.error) return out;

  if (r.bytes && sizeof(Date) <= r.size) {
    out = *(Date*)(r.bytes);
    r.bytes += sizeof(Date);
    r.size -= sizeof(Date);
  } else {
    r.error = "Read_Date: not enough bytes";
  }

  return out;
}

string Read_string(Reader &r) {
  string out = {};
  if (r.error) return out;

  int64_t stringSize = Read_int64_t(r);
  if (r.bytes && stringSize <= r.size) {
    out = string(r.bytes, stringSize);
    r.bytes += stringSize;
    r.size -= stringSize;
  } else {
    r.error = "Read_string: not enough bytes";
  }

  return out;
}

void SerializeMeetings(App &app) {
  FILE* f = fopen(SAVE_PATH, "w");

  int64_t dateCount = app.meetings.size();
  assert(fwrite(&dateCount, 1, sizeof(dateCount), f) == sizeof(dateCount));
  for (auto const& [meetingDate, meetings] : app.meetings) {
    assert(fwrite(&meetingDate, 1, sizeof(meetingDate), f) == sizeof(meetingDate));

    int64_t meetingsCount = meetings.size();
    assert(fwrite(&meetingsCount, 1, sizeof(meetingsCount), f) == sizeof(meetingsCount));

    for (string const& meeting : meetings) {
      int64_t meetingNameCount = meeting.size();
      assert(fwrite(&meetingNameCount, 1, sizeof(meetingNameCount), f) == sizeof(meetingNameCount));
      assert(fwrite(meeting.data(), 1, meetingNameCount, f) == meetingNameCount);
    }
  }

  fclose(f);
}

map<Date, vector<string>> DeserializeMeetings() {
  map<Date, vector<string>> newMeetings = {};

  FILE* f = fopen(SAVE_PATH, "r");
  if (!f) {
    perror("fopen: ");
    return newMeetings;
  }
  ssize_t size = fsize(f);
  if (size != -1) {
    char* buffer = (char*)calloc(size, sizeof(char));
    if (fread(buffer, sizeof(*buffer), size, f) == size) {
      Reader r = {};
      r.bytes = buffer;
      r.size = size;

      int64_t dateCount = Read_int64_t(r);
      for (int64_t i = 0; i < dateCount && !r.error; ++i) {
        Date d = Read_Date(r);
        size_t meetingsCount = Read_int64_t(r);
        for (int64_t j = 0; j < meetingsCount && !r.error; ++j) {
          string meeting = Read_string(r);
          if (!r.error) {
            newMeetings[d].push_back(std::move(meeting));
          }
        }
      }
      if (r.error) {
        fprintf(stderr, "DeserializeMeetings: %s", r.error);
      }
    } else {
      perror("fread: ");
    }
    free(buffer);
  } else {
    perror("fsize: ");
  }

  fclose(f);

  return newMeetings;
}

void DrawDatePicker(App &app) {
  ImGui::Text("Select a date: ");
  ImGui::SameLine();
  ImGui::PushItemWidth(60);

  if (ImGui::BeginCombo("##day", to_string(app.selectedDate.day).data())) {
    for (int8_t day = 1; day <= DaysInMonth(app.selectedDate.month, app.selectedDate.year); ++day) {
      if (ImGui::Selectable(to_string(day).data(), day == app.selectedDate.day)) {
        app.selectedDate.day = day;
      }
    }
    ImGui::EndCombo();
  }

  ImGui::SameLine();

  if (ImGui::BeginCombo("##month", MonthToShortString(app.selectedDate.month))) {
    for (int8_t month = 1; month <= 12; ++month) {
      if (ImGui::Selectable(MonthToShortString(month), month == app.selectedDate.month)) {
        app.selectedDate.month = month;
        app.selectedDate.day = min((int)app.selectedDate.day, DaysInMonth(app.selectedDate.month, app.selectedDate.year));
      }
    }
    ImGui::EndCombo();
  }

  ImGui::SameLine();

  if (ImGui::BeginCombo("##year", to_string(app.selectedDate.year).data())) {
    for (int16_t year = MIN_YEAR; year <= MAX_YEAR; ++year) {
      if (ImGui::Selectable(to_string(year).data(), year == app.selectedDate.year)) {
        app.selectedDate.year = year;
        app.selectedDate.day = min((int)app.selectedDate.day, DaysInMonth(app.selectedDate.month, app.selectedDate.year));
      }
    }
    ImGui::EndCombo();
  }

  ImGui::PopItemWidth();

  if (ImGui::Button("Add Meeting"))
    app.addMeetingWindowOpen = true;
}

void DrawCalendar(App &app) {
  ImVec2 childSize = { ImGui::GetContentRegionAvail().x, 400.0f };
  ImGui::BeginChild("###calendar", childSize, false);

  float originalFontSize = ImGui::GetFontSize();
  ImGui::SetWindowFontScale(2.0f);

  Date today = Today();
  for (int8_t month = 1; month <= 12; ++month) {
    ImGui::Text("%s", MonthToShortString(month));
      for (int8_t day = 1; day <= DaysInMonth(month, app.selectedDate.year); ++day) {
        ImGui::SameLine();

        Date date = { app.selectedDate.year, month, day };

        if (date == today)
          ImGui::TextColored(ImVec4(0.0F, 1.0F, 0.0F, 1.0F), "%d", day);
        else if (date == app.selectedDate)
          ImGui::TextColored(ImVec4(0.0F, 0.0F, 1.0F, 1.0F), "%d", day);
        else if (app.meetings.contains(date))
          ImGui::TextColored(ImVec4(1.0F, 0.0F, 0.0F, 1.0F), "%d", day);
        else
          ImGui::Text("%d", day);

        if (ImGui::IsItemClicked()) {
          app.selectedDate = date;
        }
      }
  }

  ImGui::SetWindowFontScale(originalFontSize);
  ImGui::EndChild();
}

void DrawAddMeetingWindow(App &app) {
  static char meetingNameBuffer[128] = {};
  ImVec2 meetingWindowSize = {300.0f, 100.0f};

  ImGui::SetNextWindowSize(meetingWindowSize);
  ImGui::SetNextWindowPos(ImVec2(0.5f*(1280.0f - meetingWindowSize.x), 0.5f*(720.0f - meetingWindowSize.y)));

  ImGui::Begin("###addMeeting", &app.addMeetingWindowOpen, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoScrollbar);
  ImGui::Text("Add meeting to %d.%s.%d", app.selectedDate.day, MonthToShortString(app.selectedDate.month), app.selectedDate.year);
  ImGui::InputText("Meeting Name", meetingNameBuffer, sizeof(meetingNameBuffer));

  if (ImGui::Button("Save")) {
    app.meetings[app.selectedDate].emplace_back(meetingNameBuffer);
    memset(meetingNameBuffer, 0, sizeof(meetingNameBuffer));
    SerializeMeetings(app);
    app.addMeetingWindowOpen = false;
  }

  ImGui::SameLine();

  if (ImGui::Button("Cancel")) {
    memset(meetingNameBuffer, 0, sizeof(meetingNameBuffer));
    app.addMeetingWindowOpen = false;
  }

  ImGui::End();
}

void DrawMeetingList(App &app) {
  if (app.meetings.empty()) {
    ImGui::Text("No meetings at all.");
    return;
  }

  ImGui::Text("Meetings on %d.%s.%d: ", app.selectedDate.day, MonthToShortString(app.selectedDate.month), app.selectedDate.year);

  if (!app.meetings.contains(app.selectedDate)) {
    ImGui::Text("No meetings today.");
    return;
  }

  if (app.meetings[app.selectedDate].empty()) {
    ImGui::Text("No meetings today.");
    return;
  }

  bool deletedMeetings = false;
  for (const auto &meeting : app.meetings[app.selectedDate]) {
    ImGui::BulletText("%s", meeting.data());
    if (ImGui::IsItemClicked(ImGuiMouseButton_Left)) {
      bool deletedMeetings = true;
      std::erase(app.meetings[app.selectedDate], meeting);
      if (app.meetings[app.selectedDate].empty())
        app.meetings.erase(app.selectedDate);
      return;
    }
  }

  if (deletedMeetings)
    SerializeMeetings(app);
}

void AppInit(App& app, float w, float h) {
    app = {};
    app.w = w;
    app.h = h;

    app.selectedDate = Today();
    app.meetings = std::move(DeserializeMeetings());
}

void AppUpdateAndRender(App& app) {
    auto flags = ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoScrollbar;
    ImGui::SetNextWindowSize(ImVec2(app.w, app.h));
    ImGui::SetNextWindowPos(ImVec2(0.0f, 0.0f));
    ImGui::Begin("Calendar", nullptr, flags);

    DrawDatePicker(app);
    ImGui::Separator();
    DrawCalendar(app);
    ImGui::Separator();
    DrawMeetingList(app);

    if (app.addMeetingWindowOpen)
      DrawAddMeetingWindow(app);

    ImGui::End();
}

