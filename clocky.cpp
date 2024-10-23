#include <windows.h>
#include <cmath>
#include <string>
#include <ctime>
#include <sstream>

#define PI 3.14159265

// Function to convert degrees to radians
float DegreesToRadians(float degrees) {
    return degrees * (PI / 180.0f);
}

// Function to draw clock hands
void DrawHand(HDC hdc, int xCenter, int yCenter, int length, float angleDegrees, COLORREF color) {
    int xEnd = xCenter + static_cast<int>(length * cos(DegreesToRadians(angleDegrees - 90)));
    int yEnd = yCenter + static_cast<int>(length * sin(DegreesToRadians(angleDegrees - 90)));

    HPEN pen = CreatePen(PS_SOLID, 2, color);
    SelectObject(hdc, pen);

    MoveToEx(hdc, xCenter, yCenter, NULL);
    LineTo(hdc, xEnd, yEnd);

    DeleteObject(pen);
}

// Function to convert std::string to std::wstring (wide string)
std::wstring StringToWideString(const std::string& str) {
    int size_needed = MultiByteToWideChar(CP_UTF8, 0, &str[0], (int)str.size(), NULL, 0);
    std::wstring wstrTo(size_needed, 0);
    MultiByteToWideChar(CP_UTF8, 0, &str[0], (int)str.size(), &wstrTo[0], size_needed);
    return wstrTo;
}

// Function to display the current date and time
void DisplayCurrentTime(int& hour, int& minute, int& second, std::string& dayOfWeek, int& year, int& month, int& day) {
    time_t now = time(0);
    tm localtm;
    localtime_s(&localtm, &now);

    hour = localtm.tm_hour;
    minute = localtm.tm_min;
    second = localtm.tm_sec;

    // Extract date information
    year = localtm.tm_year + 1900;
    month = localtm.tm_mon + 1;
    day = localtm.tm_mday;

    // Get day of the week
    const char* days[] = { "Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday" };
    dayOfWeek = days[localtm.tm_wday]; // Get day of the week
}

// Function to get the suffix for the day
std::string GetDaySuffix(int day) {
    if (day % 10 == 1 && day % 100 != 11) return "st";
    if (day % 10 == 2 && day % 100 != 12) return "nd";
    if (day % 10 == 3 && day % 100 != 13) return "rd";
    return "th";
}

// Function to draw a progress bar
void DrawProgressBar(HDC hdc, int x, int y, int width, int height, float progress, COLORREF color) {
    HPEN pen = CreatePen(PS_SOLID, 1, color);
    HBRUSH brush = CreateSolidBrush(color);

    SelectObject(hdc, pen);
    SelectObject(hdc, brush);

    // Draw the background of the progress bar
    Rectangle(hdc, x, y, x + width, y + height); // Background
    // Draw the progress
    int progressWidth = static_cast<int>(width * progress);
    SetBkColor(hdc, RGB(255, 255, 255)); // Set background color for text
    Rectangle(hdc, x, y, x + progressWidth, y + height); // Progress

    DeleteObject(pen);
    DeleteObject(brush);
}

// Window procedure to display time, clock, and additional information
LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    static int hour, minute, second, year, month, day;
    static std::string dayOfWeek;

    switch (uMsg) {
    case WM_PAINT: {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hwnd, &ps);

        // Get the current time and date
        DisplayCurrentTime(hour, minute, second, dayOfWeek, year, month, day);

        // Convert hour to 12-hour format and determine AM/PM
        std::string amPm = (hour >= 12) ? "PM" : "AM";
        int displayHour = hour % 12; // Convert to 12-hour format
        if (displayHour == 0) displayHour = 12; // Handle midnight case

        // Create the time string
        std::string timeString = "" + std::to_string(displayHour) + ":" +
            (minute < 10 ? "0" : "") + std::to_string(minute) + ":" +
            " " + amPm;
        std::wstring wideTimeString = StringToWideString(timeString);

        // Draw the time at the top, centered
        RECT timeRect = { 0, 10, 400, 50 };  // Define where to draw the time
        DrawText(hdc, wideTimeString.c_str(), -1, &timeRect, DT_CENTER | DT_SINGLELINE | DT_NOCLIP);

        // Draw the clock below the time
        int xCenter = 200, yCenter = 100;  // Center of the clock (adjust Y position)
        int clockRadius = 50;               // Radius of the clock
        Ellipse(hdc, xCenter - clockRadius, yCenter - clockRadius, xCenter + clockRadius, yCenter + clockRadius);

        // Draw hour marks
        for (int i = 0; i < 12; ++i) {
            float angle = DegreesToRadians(i * 30.0f); // 30 degrees per hour
            int markLength = 5; // Length of each mark
            int outerLength = clockRadius - 5; // Distance from center to outer edge of the mark

            int xStart = xCenter + static_cast<int>((outerLength - markLength) * cos(angle - PI / 2));
            int yStart = yCenter + static_cast<int>((outerLength - markLength) * sin(angle - PI / 2));
            int xEnd = xCenter + static_cast<int>(outerLength * cos(angle - PI / 2));
            int yEnd = yCenter + static_cast<int>(outerLength * sin(angle - PI / 2));

            HPEN pen = CreatePen(PS_SOLID, 1, RGB(0, 0, 0));
            SelectObject(hdc, pen);
            MoveToEx(hdc, xStart, yStart, NULL);
            LineTo(hdc, xEnd, yEnd);
            DeleteObject(pen);
        }

        // Draw clock hands
        float hourAngle = (hour % 12 + minute / 60.0f) * 30.0f; // 30 degrees per hour
        DrawHand(hdc, xCenter, yCenter, 30, hourAngle, RGB(0, 0, 0)); // Hour hand
        float minuteAngle = minute * 6.0f; // 6 degrees per minute
        DrawHand(hdc, xCenter, yCenter, 40, minuteAngle, RGB(0, 0, 0)); // Minute hand
        float secondAngle = second * 6.0f; // 6 degrees per second
        DrawHand(hdc, xCenter, yCenter, 45, secondAngle, RGB(255, 0, 0)); // Second hand

        // Calculate progress through the day (0.0 to 1.0)
        float dayProgress = (hour * 3600 + minute * 60 + second) / 86400.0f; // Percentage of the day passed

        // Draw progress bar beneath the clock
        DrawProgressBar(hdc, 50, 250, 300, 20, dayProgress, RGB(0, 255, 0)); // Progress bar

        // Draw the day of the week below the progress bar, centered
        std::wstring wideDayOfWeek = StringToWideString(dayOfWeek);
        RECT dayRect = { 0, 280, 400, 310 }; // Centered
        DrawText(hdc, wideDayOfWeek.c_str(), -1, &dayRect, DT_CENTER | DT_SINGLELINE | DT_NOCLIP);

        // Draw the current date, month, and year below the day of the week, centered
        std::string daySuffix = GetDaySuffix(day); // Get the day suffix
        std::string dateString = std::to_string(day) + daySuffix + " " +
            (month == 1 ? "January" : month == 2 ? "February" : month == 3 ? "March" :
                month == 4 ? "April" : month == 5 ? "May" : month == 6 ? "June" :
                month == 7 ? "July" : month == 8 ? "August" : month == 9 ? "September" :
                month == 10 ? "October" : month == 11 ? "November" : "December") +
            " " + std::to_string(year);
        std::wstring wideDateString = StringToWideString(dateString);
        RECT dateRect = { 0, 310, 400, 340 }; // Centered
        DrawText(hdc, wideDateString.c_str(), -1, &dateRect, DT_CENTER | DT_SINGLELINE | DT_NOCLIP);

        EndPaint(hwnd, &ps);
    }
                 break;

    case WM_TIMER: {
        // Invalidate the window to trigger a repaint
        InvalidateRect(hwnd, NULL, TRUE);
    }
                 break;

    case WM_DESTROY:
        KillTimer(hwnd, 1); // Stop the timer on window destruction
        PostQuitMessage(0);
        return 0;
    }
    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PWSTR pCmdLine, int nCmdShow) {
    const wchar_t CLASS_NAME[] = L"Time and Clock Window";

    WNDCLASS wc = {};
    wc.lpfnWndProc = WindowProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = CLASS_NAME;

    RegisterClass(&wc);

    HWND hwnd = CreateWindowEx(
        0,
        CLASS_NAME,
        L"Current Time and Clock Display",
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT, 400, 400,  // Size of window
        NULL, NULL, hInstance, NULL
    );

    if (hwnd == NULL) {
        return 0;
    }

    // Set a timer to update the clock every second
    SetTimer(hwnd, 1, 1000, NULL);

    ShowWindow(hwnd, nCmdShow);

    MSG msg = {};
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return 0;
}
