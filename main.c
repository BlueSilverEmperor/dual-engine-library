#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <time.h>
#include "raylib.h"

#define RAYGUI_IMPLEMENTATION
#include "raygui.h"

#include "bst.h"
#include "controller.h"
#include "benchmark.h"

#define DEFAULT_SCREEN_WIDTH  1120
#define DEFAULT_SCREEN_HEIGHT 720

// --- REFINED DUAL THEME COLOR PALETTE ---

// Light Mode Palette (Clean Paper & Sage)
static const Color BG_LIGHT          = {241, 245, 249, 255}; // #F1F5F9 (Soft Slate/Paper BG)
static const Color PANEL_LIGHT       = {255, 255, 255, 255}; // #FFFFFF (Crisp White Surfaces)
static const Color TEXT_LIGHT        = {15, 23, 42, 255};   // #0F172A (Deep Slate Text)
static const Color MUTED_TEXT_LIGHT  = {71, 85, 105, 255};   // #475569 (Dark Slate Muted Labels - High Contrast)
static const Color ACCENT_SAGE       = {13, 148, 136, 255};  // #0D9488 (Teal/Sage Primary Accent)
static const Color ALERT_AMBER      = {217, 119, 6, 255};   // #D97706 (Amber Alert)
static const Color ALERT_RED_LIGHT   = {225, 29, 72, 255};   // #E11D48 (Vibrant Overdue Red)
static const Color CARD_HEADER_LIGHT = {241, 245, 249, 255}; // #F1F5F9 (Header Surface)
static const Color BORDER_LIGHT      = {203, 213, 225, 255}; // #CBD5E1 (Subtle Border)
static const Color ROW_ALT_LIGHT     = {248, 250, 252, 255}; // #F8FAFC (Alternating Rows)

// Dark Mode Palette (Executive High-Contrast Dark Slate)
static const Color BG_DARK           = {15, 23, 42, 255};   // #0F172A (Deep Navy Dark BG)
static const Color PANEL_DARK        = {30, 41, 59, 255};   // #1E293B (Card Surfaces)
static const Color TEXT_DARK         = {255, 255, 255, 255}; // #FFFFFF (Crisp White Text)
static const Color MUTED_TEXT_DARK   = {203, 213, 225, 255}; // #CBD5E1 (Crisp Slate White Muted Text - Ultra Legible)
static const Color ACCENT_SAGE_DARK  = {52, 211, 153, 255};  // #34D399 (Vibrant Emerald Accent)
static const Color ALERT_AMBER_DARK  = {251, 191, 36, 255};  // #FBBF24 (Bright Amber Alert)
static const Color ALERT_RED_DARK    = {251, 113, 133, 255}; // #FB7185 (Vibrant Crimson Red)
static const Color CARD_HEADER_DARK  = {51, 65, 85, 255};   // #334155 (Header Surface)
static const Color BORDER_DARK       = {94, 114, 140, 255};  // #5E728C (Clear Border)
static const Color ROW_ALT_DARK      = {39, 51, 71, 255};   // #273347 (Alternating Rows)

// High-Legibility Modern TrueType Font Engine
static Font mainFont;
static bool fontLoaded = false;

// Universal DrawTextEx wrapper ensuring DrawTextEx is used for all rendering
void DrawAppText(const char* text, int posX, int posY, float fontSize, Color color) {
    if (!text || text[0] == '\0') return;
    Vector2 pos = { (float)posX, (float)posY };
    Font fontToUse = fontLoaded ? mainFont : GetFontDefault();
    DrawTextEx(fontToUse, text, pos, fontSize, 1.0f, color);
}

int MeasureAppText(const char* text, float fontSize) {
    if (!text || text[0] == '\0') return 0;
    Font fontToUse = fontLoaded ? mainFont : GetFontDefault();
    Vector2 sz = MeasureTextEx(fontToUse, text, fontSize, 1.0f);
    return (int)sz.x;
}

// Active Navigation Tab
typedef enum {
    TAB_CATALOG = 0,
    TAB_OVERDUE = 1,
    TAB_SEARCH  = 2,
    TAB_BENCHMARK = 3
} ActiveTab;

// Custom Text Box Component for crisp input handling
typedef struct {
    Rectangle rect;
    char text[128];
    int maxLen;
    bool active;
    const char* label;
    const char* placeholder;
} CustomInputBox;

// Global Toast Banner
typedef struct {
    char message[128];
    Color color;
    float displayTimer;
} ToastNotification;

static ToastNotification toast = {"Executive Library System initialized.", {16, 185, 129, 255}, 4.0f};

// Benchmark Dataset State
static BenchmarkDataset* benchDs = NULL;
static Benchmark10MDataset* bench10MDs = NULL;
static bool benchMode10M = false;
static BenchmarkResult benchRes;
static StressTestResult benchStress;
static bool benchHasRes = false;
static bool benchHasStress = false;

// Stats Pop-Up Modal State
typedef struct {
    bool active;
} StatsPopupWindowModal;

static StatsPopupWindowModal statsModal = { false };

void showToast(const char* msg, Color col) {
    strncpy(toast.message, msg, sizeof(toast.message) - 1);
    toast.message[sizeof(toast.message) - 1] = '\0';
    toast.color = col;
    toast.displayTimer = 3.5f;
}

// --- DATE FORMATTING HELPER FUNCTIONS (YYYY-MM-DD) ---

int parseDateStringToInt(const char* str) {
    if (!str || str[0] == '\0') return 0;
    int y = 0, m = 0, d = 0;
    if (sscanf(str, "%d-%d-%d", &y, &m, &d) == 3) {
        return y * 10000 + m * 100 + d;
    }
    return atoi(str);
}

void formatDateIntToString(int dateInt, char* outBuf, size_t bufSize) {
    if (dateInt <= 0) {
        snprintf(outBuf, bufSize, "N/A");
        return;
    }
    int y = dateInt / 10000;
    int m = (dateInt % 10000) / 100;
    int d = dateInt % 100;
    snprintf(outBuf, bufSize, "%04d-%02d-%02d", y, m, d);
}

// --- INTERACTIVE CALENDAR PICKER MODULE ---

typedef struct {
    bool active;
    int viewYear;
    int viewMonth;
    CustomInputBox* targetInput;
    Vector2 anchorPos;
} CalendarPicker;

static CalendarPicker calPicker = { false, 2026, 9, NULL, {275, 200} };

static const char* MONTH_NAMES[] = {
    "", "January", "February", "March", "April", "May", "June",
    "July", "August", "September", "October", "November", "December"
};

int getDaysInMonth(int year, int month) {
    static const int days[] = { 0, 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };
    if (month == 2) {
        bool isLeap = (year % 4 == 0 && (year % 100 != 0 || year % 400 == 0));
        return isLeap ? 29 : 28;
    }
    if (month >= 1 && month <= 12) return days[month];
    return 30;
}

int getFirstDayOfWeek(int year, int month) {
    // Sakamoto's algorithm: returns 0 = Sunday, 1 = Monday, ..., 6 = Saturday
    static const int t[] = { 0, 3, 2, 5, 0, 3, 5, 1, 4, 6, 2, 4 };
    int y = year - (month < 3);
    int dow = (y + y/4 - y/100 + y/400 + t[month-1] + 1) % 7;
    // Convert to 0 = Monday, ..., 6 = Sunday
    return (dow + 6) % 7;
}

void openCalendarForInput(CalendarPicker* picker, CustomInputBox* inputBox, Vector2 anchorPos) {
    picker->targetInput = inputBox;
    picker->anchorPos = anchorPos;
    picker->active = true;
    
    int val = parseDateStringToInt(inputBox->text);
    if (val > 10000000) {
        picker->viewYear = val / 10000;
        picker->viewMonth = (val % 10000) / 100;
        if (picker->viewMonth < 1) picker->viewMonth = 1;
        if (picker->viewMonth > 12) picker->viewMonth = 12;
    } else {
        time_t t = time(NULL);
        struct tm tm = *localtime(&t);
        picker->viewYear = tm.tm_year + 1900;
        picker->viewMonth = tm.tm_mon + 1;
    }
}

void addDaysToCurrentDate(int addDays, char* outBuf, size_t bufSize) {
    time_t t = time(NULL);
    struct tm tm = *localtime(&t);
    tm.tm_mday += addDays;
    mktime(&tm);
    snprintf(outBuf, bufSize, "%04d-%02d-%02d", tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday);
}

// Custom Executive Button with 15px Button Text via DrawTextEx
bool DrawExecutiveButton(Rectangle rect, const char* text, Color bgNormal, Color bgHover, Color textCol, Color borderCol) {
    Vector2 mousePos = GetMousePosition();
    bool hovered = CheckCollisionPointRec(mousePos, rect);
    Color bg = hovered ? bgHover : bgNormal;
    
    DrawRectangleRounded(rect, 0.22f, 6, bg);
    DrawRectangleRoundedLinesEx(rect, 0.22f, 6, 1.2f, hovered ? textCol : Fade(borderCol, 0.8f));
    
    float fontSize = 15.0f; // 15px Button Text
    int textWidth = MeasureAppText(text, fontSize);
    DrawAppText(text, (int)(rect.x + (rect.width - textWidth) / 2.0f), (int)(rect.y + (rect.height - fontSize) / 2.0f), fontSize, textCol);
    
    return hovered && IsMouseButtonPressed(MOUSE_BUTTON_LEFT);
}

// Handles mouse click focus, backspace, typing, and rendering for custom input fields
void UpdateCustomInput(CustomInputBox* box) {
    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
        box->active = CheckCollisionPointRec(GetMousePosition(), box->rect);
    }
    
    if (box->active) {
        int key = GetCharPressed();
        while (key > 0) {
            if ((key >= 32) && (key <= 125) && ((int)strlen(box->text) < box->maxLen)) {
                int len = (int)strlen(box->text);
                box->text[len] = (char)key;
                box->text[len + 1] = '\0';
            }
            key = GetCharPressed();
        }
        
        if (IsKeyPressed(KEY_BACKSPACE)) {
            int len = (int)strlen(box->text);
            if (len > 0) {
                box->text[len - 1] = '\0';
            }
        }
    }
}

// Input field rendering: 14px Form Field Labels, 14px Input Text via DrawTextEx
void DrawCustomInput(CustomInputBox* box, Color textColor, Color labelColor, Color bgBoxColor, Color borderColor, Color activeBorderColor) {
    if (box->label) {
        DrawAppText(box->label, (int)box->rect.x, (int)box->rect.y - 19, 14.0f, labelColor); // 14px Form Label
    }
    
    Color border = box->active ? activeBorderColor : borderColor;
    
    DrawRectangleRounded(box->rect, 0.18f, 4, bgBoxColor);
    DrawRectangleRoundedLinesEx(box->rect, 0.18f, 4, 1.4f, border);
    
    // +8px padding from left edge, 14px Input Text
    if (strlen(box->text) > 0) {
        DrawAppText(box->text, (int)box->rect.x + 8, (int)box->rect.y + 8, 14.0f, textColor);
    } else if (box->placeholder) {
        DrawAppText(box->placeholder, (int)box->rect.x + 8, (int)box->rect.y + 8, 14.0f, labelColor);
    }
    
    // Blinking cursor
    if (box->active && (((int)(GetTime() * 2.2)) % 2 == 0)) {
        int textW = MeasureAppText(box->text, 14.0f);
        DrawRectangle((int)box->rect.x + 10 + textW, (int)box->rect.y + 8, 2, 18, activeBorderColor);
    }
}

void DrawCalendarPickerModal(CalendarPicker* picker, Color currentPanel, Color currentText, Color currentMuted, Color currentAccent, Color currentHeader, Color currentBorder, Color currentRed) {
    if (!picker->active || !picker->targetInput) return;
    
    // Popup container dimensions
    float calW = 270.0f;
    float calH = 300.0f;
    float calX = picker->anchorPos.x;
    float calY = picker->anchorPos.y;
    
    // Keep within screen bounds
    if (calX + calW > GetScreenWidth() - 10) calX = (float)GetScreenWidth() - calW - 10;
    if (calY + calH > GetScreenHeight() - 10) calY = (float)GetScreenHeight() - calH - 10;
    if (calX < 10) calX = 10;
    if (calY < 10) calY = 10;
    
    Rectangle calRect = { calX, calY, calW, calH };
    
    // Drop shadow
    DrawRectangleRounded((Rectangle){ calX + 4, calY + 4, calW, calH }, 0.12f, 4, Fade(BLACK, 0.35f));
    
    // Main Panel Background
    DrawRectangleRounded(calRect, 0.12f, 4, currentPanel);
    DrawRectangleRoundedLinesEx(calRect, 0.12f, 4, 1.5f, currentAccent);
    
    // Header Bar (Month & Year + Nav)
    DrawRectangleRounded((Rectangle){ calX + 1, calY + 1, calW - 2, 40 }, 0.12f, 4, currentHeader);
    
    // Prev Month Button '<'
    if (DrawExecutiveButton((Rectangle){ calX + 8, calY + 6, 30, 28 }, "<", currentPanel, currentAccent, currentText, currentBorder)) {
        picker->viewMonth--;
        if (picker->viewMonth < 1) {
            picker->viewMonth = 12;
            picker->viewYear--;
        }
    }
    
    // Month Year Label
    char titleBuf[32];
    snprintf(titleBuf, sizeof(titleBuf), "%s %d", MONTH_NAMES[picker->viewMonth], picker->viewYear);
    int titleW = MeasureAppText(titleBuf, 15.0f);
    DrawAppText(titleBuf, (int)(calX + (calW - titleW) / 2.0f), (int)(calY + 12), 15.0f, currentText);
    
    // Next Month Button '>'
    if (DrawExecutiveButton((Rectangle){ calX + calW - 78, calY + 6, 30, 28 }, ">", currentPanel, currentAccent, currentText, currentBorder)) {
        picker->viewMonth++;
        if (picker->viewMonth > 12) {
            picker->viewMonth = 1;
            picker->viewYear++;
        }
    }
    
    // Close Button 'X'
    if (DrawExecutiveButton((Rectangle){ calX + calW - 38, calY + 6, 30, 28 }, "X", currentPanel, currentRed, currentText, currentBorder)) {
        picker->active = false;
        return;
    }
    
    // Days of week header
    static const char* DOW[] = { "Mo", "Tu", "We", "Th", "Fr", "Sa", "Su" };
    float colW = (calW - 20) / 7.0f;
    float startX = calX + 10;
    float headerY = calY + 46;
    
    for (int i = 0; i < 7; i++) {
        int w = MeasureAppText(DOW[i], 12.0f);
        DrawAppText(DOW[i], (int)(startX + i * colW + (colW - w) / 2.0f), (int)headerY, 12.0f, currentMuted);
    }
    
    DrawLine((int)(calX + 10), (int)(calY + 64), (int)(calX + calW - 10), (int)(calY + 64), Fade(currentBorder, 0.6f));
    
    // Days Grid
    int totalDays = getDaysInMonth(picker->viewYear, picker->viewMonth);
    int firstDow = getFirstDayOfWeek(picker->viewYear, picker->viewMonth); // 0=Mon, ..., 6=Sun
    
    // Currently selected day in target input
    int targetVal = parseDateStringToInt(picker->targetInput->text);
    int selectedY = targetVal / 10000;
    int selectedM = (targetVal % 10000) / 100;
    int selectedD = targetVal % 100;
    
    // Today's date
    time_t tNow = time(NULL);
    struct tm tmNow = *localtime(&tNow);
    int todayY = tmNow.tm_year + 1900;
    int todayM = tmNow.tm_mon + 1;
    int todayD = tmNow.tm_mday;
    
    float gridStartY = calY + 70;
    float cellH = 25.0f;
    Vector2 mouseP = GetMousePosition();
    
    int cellIndex = 0;
    for (int row = 0; row < 6; row++) {
        for (int col = 0; col < 7; col++) {
            int dayNum = cellIndex - firstDow + 1;
            float cx = startX + col * colW;
            float cy = gridStartY + row * (cellH + 2);
            
            if (dayNum >= 1 && dayNum <= totalDays) {
                Rectangle cellRect = { cx + 2, cy, colW - 4, cellH };
                bool isHovered = CheckCollisionPointRec(mouseP, cellRect);
                bool isSelected = (picker->viewYear == selectedY && picker->viewMonth == selectedM && dayNum == selectedD);
                bool isToday = (picker->viewYear == todayY && picker->viewMonth == todayM && dayNum == todayD);
                
                Color bgCol = isSelected ? currentAccent : (isHovered ? Fade(currentAccent, 0.25f) : (isToday ? Fade(currentAccent, 0.12f) : currentPanel));
                Color textCol = isSelected ? RAYWHITE : (isToday ? currentAccent : currentText);
                
                DrawRectangleRounded(cellRect, 0.2f, 4, bgCol);
                if (isToday && !isSelected) {
                    DrawRectangleRoundedLinesEx(cellRect, 0.2f, 4, 1.0f, currentAccent);
                }
                
                char dayStr[16];
                snprintf(dayStr, sizeof(dayStr), "%d", dayNum);
                int dayW = MeasureAppText(dayStr, 13.0f);
                DrawAppText(dayStr, (int)(cx + 2 + (colW - 4 - dayW) / 2.0f), (int)(cy + 5), 13.0f, textCol);
                
                if (isHovered && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
                    snprintf(picker->targetInput->text, picker->targetInput->maxLen, "%04d-%02d-%02d", picker->viewYear, picker->viewMonth, dayNum);
                    char msg[160];
                    snprintf(msg, sizeof(msg), "Selected Date: %s", picker->targetInput->text);
                    showToast(msg, currentAccent);
                    picker->active = false;
                }
            }
            cellIndex++;
        }
    }
    
    // Quick Date Shortcuts Footer
    float footerY = calY + calH - 35;
    DrawLine((int)(calX + 10), (int)footerY - 4, (int)(calX + calW - 10), (int)footerY - 4, Fade(currentBorder, 0.6f));
    
    if (DrawExecutiveButton((Rectangle){ calX + 10, footerY, 75, 26 }, "Today", currentHeader, currentAccent, currentText, currentBorder)) {
        snprintf(picker->targetInput->text, picker->targetInput->maxLen, "%04d-%02d-%02d", todayY, todayM, todayD);
        showToast("Set date to Today", currentAccent);
        picker->active = false;
    }
    
    if (DrawExecutiveButton((Rectangle){ calX + 93, footerY, 78, 26 }, "+7 Days", currentHeader, currentAccent, currentText, currentBorder)) {
        addDaysToCurrentDate(7, picker->targetInput->text, picker->targetInput->maxLen);
        showToast("Set date to +7 Days", currentAccent);
        picker->active = false;
    }
    
    if (DrawExecutiveButton((Rectangle){ calX + 179, footerY, 81, 26 }, "+30 Days", currentHeader, currentAccent, currentText, currentBorder)) {
        addDaysToCurrentDate(30, picker->targetInput->text, picker->targetInput->maxLen);
        showToast("Set date to +30 Days", currentAccent);
        picker->active = false;
    }
}

void DrawStatsPopupWindowModal(StatsPopupWindowModal* modal, LibraryController* ctrl, int refDate, int screenW, int screenH, bool isDarkMode, Color currentBg, Color currentPanel, Color currentText, Color currentMuted, Color currentAccent, Color currentAlert, Color currentRed, Color currentHeader, Color currentBorder) {
    if (!modal || !modal->active) return;
    
    // ESC key closes modal
    if (IsKeyPressed(KEY_ESCAPE)) {
        modal->active = false;
        return;
    }

    float modalW = 820.0f;
    if (modalW > (float)screenW - 40.0f) modalW = (float)screenW - 40.0f;
    float modalH = 560.0f;
    if (modalH > (float)screenH - 40.0f) modalH = (float)screenH - 40.0f;
    
    float modalX = ((float)screenW - modalW) / 2.0f;
    float modalY = ((float)screenH - modalH) / 2.0f;

    // Overlay backdrop
    DrawRectangle(0, 0, screenW, screenH, Fade(BLACK, 0.65f));

    // Modal background box
    DrawRectangleRounded((Rectangle){ modalX, modalY, modalW, modalH }, 0.08f, 4, currentPanel);
    DrawRectangleRoundedLinesEx((Rectangle){ modalX, modalY, modalW, modalH }, 0.08f, 4, 1.5f, currentAccent);

    // Header bar inside modal
    DrawRectangleRounded((Rectangle){ modalX + 1, modalY + 1, modalW - 2, 44 }, 0.08f, 4, currentHeader);
    DrawLine((int)modalX, (int)(modalY + 45), (int)(modalX + modalW), (int)(modalY + 45), currentBorder);
    
    DrawAppText("[STATS] EXECUTIVE SYSTEM STATS & ALGORITHMIC BENCHMARK LAB", (int)(modalX + 18), (int)(modalY + 13), 16.0f, currentAccent);

    // Close button top right [ X ]
    if (DrawExecutiveButton((Rectangle){ modalX + modalW - 44, modalY + 7, 32, 30 }, "X", currentHeader, currentRed, currentText, currentBorder)) {
        modal->active = false;
        return;
    }

    // Quick Stats Grid (4 Cards)
    float gridY = modalY + 58.0f;
    float cardW = (modalW - 50.0f) / 4.0f;
    float cardH = 75.0f;

    int totalB = getTotalBooksController(ctrl);
    
    Book* catalogBuf[200];
    int totalCount = getCatalogController(ctrl, catalogBuf, 200);
    int activeCount = 0;
    int returnedCount = 0;
    for (int i = 0; i < totalCount; i++) {
        if (catalogBuf[i]->isReturned) returnedCount++;
        else activeCount++;
    }

    Book* overdueBuf[100];
    int overdueCount = checkOverdueController(ctrl, refDate, overdueBuf, 100);

    // Card 1: Total Catalog
    Rectangle c1 = { modalX + 12, gridY, cardW, cardH };
    DrawRectangleRounded(c1, 0.15f, 4, Fade(currentAccent, 0.1f));
    DrawRectangleRoundedLinesEx(c1, 0.15f, 4, 1.0f, currentAccent);
    DrawAppText("TOTAL CATALOG", (int)c1.x + 10, (int)c1.y + 8, 12.0f, currentMuted);
    DrawAppText(TextFormat("%d Books", totalB), (int)c1.x + 10, (int)c1.y + 26, 19.0f, currentAccent);
    DrawAppText("BST Substring Index", (int)c1.x + 10, (int)c1.y + 53, 11.0f, currentMuted);

    // Card 2: Active vs Returned
    Rectangle c2 = { modalX + 22 + cardW, gridY, cardW, cardH };
    DrawRectangleRounded(c2, 0.15f, 4, Fade((Color){59, 130, 246, 255}, 0.1f));
    DrawRectangleRoundedLinesEx(c2, 0.15f, 4, 1.0f, (Color){59, 130, 246, 255});
    DrawAppText("ACTIVE / RETURNED", (int)c2.x + 10, (int)c2.y + 8, 12.0f, currentMuted);
    DrawAppText(TextFormat("%d Out | %d Ret", activeCount, returnedCount), (int)c2.x + 10, (int)c2.y + 26, 16.0f, (Color){59, 130, 246, 255});
    DrawAppText("Filtered Priority Heap", (int)c2.x + 10, (int)c2.y + 53, 11.0f, currentMuted);

    // Card 3: Overdue Priority
    Rectangle c3 = { modalX + 32 + cardW * 2, gridY, cardW, cardH };
    DrawRectangleRounded(c3, 0.15f, 4, Fade(currentRed, 0.1f));
    DrawRectangleRoundedLinesEx(c3, 0.15f, 4, 1.0f, currentRed);
    DrawAppText("OVERDUE PRIORITY", (int)c3.x + 10, (int)c3.y + 8, 12.0f, currentMuted);
    DrawAppText(TextFormat("%d Overdue", overdueCount), (int)c3.x + 10, (int)c3.y + 26, 19.0f, currentRed);
    DrawAppText(TextFormat("Ref Date: %d", refDate), (int)c3.x + 10, (int)c3.y + 53, 11.0f, currentMuted);

    // Card 4: Dataset Memory Mode
    Rectangle c4 = { modalX + 42 + cardW * 3, gridY, cardW, cardH };
    DrawRectangleRounded(c4, 0.15f, 4, Fade(currentAlert, 0.1f));
    DrawRectangleRoundedLinesEx(c4, 0.15f, 4, 1.0f, currentAlert);
    DrawAppText("BENCHMARK SLAB", (int)c4.x + 10, (int)c4.y + 8, 12.0f, currentMuted);
    bool isLoaded = benchMode10M ? (bench10MDs != NULL) : (benchDs != NULL);
    DrawAppText(isLoaded ? (benchMode10M ? "10M Slab" : "1M Dataset") : "Not Loaded", (int)c4.x + 10, (int)c4.y + 26, 17.0f, currentAlert);
    DrawAppText(isLoaded ? (benchMode10M ? "~2.82 GB RAM" : "~537 MB RAM") : "0 MB Heap", (int)c4.x + 10, (int)c4.y + 53, 11.0f, currentMuted);

    // Algorithmic Benchmark Section Box
    float benchBoxY = gridY + 84.0f;
    float benchBoxH = modalH - 195.0f;
    Rectangle bBox = { modalX + 12, benchBoxY, modalW - 24, benchBoxH };
    DrawRectangleRounded(bBox, 0.08f, 4, currentBg);
    DrawRectangleRoundedLinesEx(bBox, 0.08f, 4, 1.0f, currentBorder);

    DrawAppText("[FAST] LIVE DUAL-ENGINE ALGORITHMIC PERFORMANCE & AUDIT", (int)bBox.x + 14, (int)bBox.y + 12, 14.0f, currentAccent);
    DrawAppText(TextFormat("Active Mode: [%s]", benchMode10M ? "10,000,000 RECORD SLAB" : "1,000,000 RECORD DATASET"), (int)bBox.x + (int)bBox.width - 310, (int)bBox.y + 12, 12.0f, currentAlert);

    if (!isLoaded) {
        DrawAppText("No synthetic benchmark dataset currently allocated in memory.", (int)bBox.x + 20, (int)bBox.y + 45, 14.0f, currentText);
        DrawAppText("Allocate synthetic records to perform side-by-side microsecond latency benchmarking:", (int)bBox.x + 20, (int)bBox.y + 70, 13.0f, currentMuted);

        if (DrawExecutiveButton((Rectangle){ bBox.x + 20, bBox.y + 105, 270, 36 }, "[FAST] ALLOCATE 1M DATASET (~537MB)", currentAccent, isDarkMode ? (Color){4, 120, 87, 255} : (Color){15, 118, 110, 255}, RAYWHITE, currentAccent)) {
            benchMode10M = false;
            showToast("Initializing 1,000,000 records on heap...", currentAlert);
            benchDs = initBenchmarkDataset(1000000);
            if (benchDs) {
                benchRes = runDualVsTraditionalBenchmark(benchDs, "Clean Code Architecture");
                benchHasRes = true;
                showToast("Loaded 1,000,000 records!", currentAccent);
            }
        }

        if (DrawExecutiveButton((Rectangle){ bBox.x + 305, bBox.y + 105, 290, 36 }, "[FAST] ALLOCATE 10M SLAB (~2.82GB)", currentAlert, isDarkMode ? (Color){180, 83, 9, 255} : (Color){217, 119, 6, 255}, RAYWHITE, currentAlert)) {
            benchMode10M = true;
            showToast("Allocating 10,000,000 contiguous slab on heap...", currentAlert);
            bench10MDs = initBenchmark10MDataset(10000000);
            if (bench10MDs) {
                benchRes = run10MBenchmark(bench10MDs, "Clean Code Architecture 10M");
                benchHasRes = true;
                showToast("Loaded 10,000,000 slab!", currentAccent);
            }
        }
    } else {
        float innerCardW = (bBox.width - 32.0f) / 2.0f;
        float innerCardH = 145.0f;

        // Dual Engine Card
        Rectangle dec = { bBox.x + 10, bBox.y + 36, innerCardW, innerCardH };
        DrawRectangleRounded(dec, 0.1f, 4, Fade(currentAccent, 0.1f));
        DrawRectangleRoundedLinesEx(dec, 0.1f, 4, 1.0f, currentAccent);

        DrawAppText(TextFormat("[FAST] DUAL-ENGINE INDEX (%s)", benchMode10M ? "10M SLAB" : "1M BST"), (int)dec.x + 12, (int)dec.y + 10, 14.0f, currentAccent);
        if (benchHasRes) {
            DrawAppText(TextFormat("BST Title Search: %.3f us (%.5f ms)", benchRes.bstTimeUs, benchRes.bstTimeUs / 1000.0), (int)dec.x + 12, (int)dec.y + 33, 14.0f, currentText);
            DrawAppText(TextFormat("Heap Earliest Due Peek: %.3f us (%.5f ms)", benchRes.minHeapTimeUs, benchRes.minHeapTimeUs / 1000.0), (int)dec.x + 12, (int)dec.y + 55, 14.0f, currentText);
            DrawAppText("Complexity: Iterative O(log n) BST / O(1) Min-Heap", (int)dec.x + 12, (int)dec.y + 80, 12.0f, currentMuted);
            DrawAppText(TextFormat("Comparisons Needed: ~%d Comparisons", benchMode10M ? 24 : 20), (int)dec.x + 12, (int)dec.y + 100, 12.0f, currentMuted);
            DrawAppText(TextFormat("Result Book Match: ID #%d", benchRes.foundId), (int)dec.x + 12, (int)dec.y + 120, 13.0f, currentAccent);
        } else {
            DrawAppText("Click 'RUN BENCHMARK QUERY' below to measure latency.", (int)dec.x + 12, (int)dec.y + 40, 13.0f, currentMuted);
        }

        // Traditional Linear Card
        Rectangle tlc = { bBox.x + 22 + innerCardW, bBox.y + 36, innerCardW, innerCardH };
        DrawRectangleRounded(tlc, 0.1f, 4, Fade(currentRed, 0.1f));
        DrawRectangleRoundedLinesEx(tlc, 0.1f, 4, 1.0f, currentRed);

        DrawAppText(TextFormat("[SLOW] TRADITIONAL LINEAR SCAN (%s)", benchMode10M ? "10M SLAB SCAN" : "1M FLAT ARRAY"), (int)tlc.x + 12, (int)tlc.y + 10, 14.0f, currentRed);
        if (benchHasRes) {
            DrawAppText(TextFormat("Linear Search Latency: %.3f ms (%.1f us)", benchRes.linearSearchTimeMs, benchRes.linearSearchTimeMs * 1000.0), (int)tlc.x + 12, (int)tlc.y + 33, 14.0f, currentText);
            DrawAppText(TextFormat("Linear Overdue Scan: %.3f ms (%.1f us)", benchRes.linearHeapTimeMs, benchRes.linearHeapTimeMs * 1000.0), (int)tlc.x + 12, (int)tlc.y + 55, 14.0f, currentText);
            DrawAppText("Complexity: Full O(n) Contiguous Memory Traversal", (int)tlc.x + 12, (int)tlc.y + 80, 12.0f, currentMuted);
            DrawAppText(TextFormat("Comparisons Needed: %s Memory Structs", benchMode10M ? "10,000,000" : "1,000,000"), (int)tlc.x + 12, (int)tlc.y + 100, 12.0f, currentMuted);
            DrawAppText(TextFormat("Result Book Match: ID #%d", benchRes.foundId), (int)tlc.x + 12, (int)tlc.y + 120, 13.0f, currentRed);
        } else {
            DrawAppText("Click 'RUN BENCHMARK QUERY' below to measure latency.", (int)tlc.x + 12, (int)tlc.y + 40, 13.0f, currentMuted);
        }

        // Speedup Banner Inside Modal
        if (benchHasRes) {
            float sbY = bBox.y + innerCardH + 44.0f;
            Rectangle sbRect = { bBox.x + 10, sbY, bBox.width - 20, 38 };
            DrawRectangleRounded(sbRect, 0.15f, 4, Fade(currentAccent, 0.18f));
            DrawRectangleRoundedLinesEx(sbRect, 0.15f, 4, 1.0f, currentAccent);

            DrawAppText(TextFormat("[SPEEDUP] BST Search %.0fx FASTER | Min-Heap Peek %.0fx FASTER", benchRes.searchSpeedup, benchRes.heapSpeedup), (int)sbRect.x + 12, (int)sbRect.y + 10, 14.0f, currentAccent);
            DrawAppText(TextFormat("Audit: %s", benchRes.correctnessPass ? "PASSED [PASS]" : "FAILED [FAIL]"), (int)sbRect.x + (int)sbRect.width - 250, (int)sbRect.y + 10, 14.0f, benchRes.correctnessPass ? currentAccent : currentRed);
        }

        if (benchHasStress) {
            float stY = bBox.y + innerCardH + 88.0f;
            DrawAppText(TextFormat("10x Stress Loop Average: BST: %.3f us | Min-Heap: %.3f us | Speedup: %.0fx", benchStress.avgBstTimeUs, benchStress.avgMinHeapTimeUs, benchStress.avgSearchSpeedup), (int)bBox.x + 12, (int)stY, 13.0f, currentAlert);
        }
    }

    // Footer Control Buttons inside Modal
    float btnY = modalY + modalH - 42.0f;
    if (isLoaded) {
        if (DrawExecutiveButton((Rectangle){ modalX + 12, btnY, 200, 32 }, "[FAST] RUN BENCHMARK", currentAccent, isDarkMode ? (Color){4, 120, 87, 255} : (Color){15, 118, 110, 255}, RAYWHITE, currentAccent)) {
            if (benchMode10M) {
                benchRes = run10MBenchmark(bench10MDs, "Clean Code Architecture 10M");
            } else {
                benchRes = runDualVsTraditionalBenchmark(benchDs, "Clean Code Architecture");
            }
            benchHasRes = true;
            showToast("Benchmark executed!", currentAccent);
        }

        if (DrawExecutiveButton((Rectangle){ modalX + 220, btnY, 180, 32 }, "[LOOP] STRESS (10x)", currentAlert, isDarkMode ? (Color){180, 83, 9, 255} : (Color){217, 119, 6, 255}, RAYWHITE, currentAlert)) {
            if (benchMode10M) {
                benchStress = run10MStressLoopBenchmark(bench10MDs, 10);
            } else {
                benchStress = runStressLoopBenchmark(benchDs, 10);
            }
            benchHasStress = true;
            showToast("10x Stress Loop completed!", currentAlert);
        }
    }

    if (DrawExecutiveButton((Rectangle){ modalX + modalW - 202, btnY, 190, 32 }, "CLOSE STATS WINDOW", currentHeader, isDarkMode ? (Color){71, 85, 105, 255} : (Color){226, 232, 240, 255}, currentText, currentBorder)) {
        modal->active = false;
    }
}

int main(void) {
    // 1. Initialize Raylib Window with resizable support
    SetConfigFlags(FLAG_MSAA_4X_HINT | FLAG_VSYNC_HINT | FLAG_WINDOW_RESIZABLE);
    InitWindow(DEFAULT_SCREEN_WIDTH, DEFAULT_SCREEN_HEIGHT, "Smart Library Book Inventory & Due-Date Tracker");
    SetWindowMinSize(900, 600);
    SetTargetFPS(60);
    
    // 2. Load High-Legibility Inter / Segoe UI TrueType Font Engine (Antigravity Typography)
    const char* fontPath = NULL;
    if (FileExists("inter.ttf")) {
        fontPath = "inter.ttf";
    } else if (FileExists("segoeui.ttf")) {
        fontPath = "segoeui.ttf";
    } else if (FileExists("C:\\Windows\\Fonts\\segoeui.ttf")) {
        fontPath = "C:\\Windows\\Fonts\\segoeui.ttf";
    }
    
    if (fontPath != NULL) {
        mainFont = LoadFontEx(fontPath, 48, 0, 0);
        SetTextureFilter(mainFont.texture, TEXTURE_FILTER_BILINEAR);
        fontLoaded = true;
    }
    
    // Theme State Variable (false = Light Mode, true = Dark Mode)
    bool isDarkMode = true;
    
    // 3. Initialize Controller & Persistence Engine
    LibraryController* ctrl = initController("library_data.txt");
    
    if (getTotalBooksController(ctrl) == 0) {
        addBookController(ctrl, 101, "The C Programming Language", "Kernighan & Ritchie", 20260920);
        addBookController(ctrl, 102, "Introduction to Algorithms", "Cormen, Leiserson", 20261015);
        addBookController(ctrl, 103, "Clean Code", "Robert C. Martin", 20260910);
        addBookController(ctrl, 104, "Design Patterns", "Gang of Four", 20260925);
        addBookController(ctrl, 105, "Computer Systems: A Programmer's Perspective", "Bryant & O'Hallaron", 20260905);
        addBookController(ctrl, 106, "Operating System Concepts", "Silberschatz", 20261101);
        showToast("Loaded Executive Demo Dataset.", (Color){16, 185, 129, 255});
    }
    
    // Form Input State
    CustomInputBox titleInput   = {(Rectangle){20, 105, 240, 36}, "", 127, false, "Book Title", "e.g. Code Complete"};
    CustomInputBox authorInput  = {(Rectangle){20, 175, 240, 36}, "", 127, false, "Author Name", "e.g. Steve McConnell"};
    CustomInputBox dateInput    = {(Rectangle){20, 245, 190, 36}, "2026-09-30", 15, false, "Due Date (YYYY-MM-DD)", "2026-09-30"};
    CustomInputBox refDateInput = {(Rectangle){20, 370, 190, 36}, "2026-09-20", 15, false, "Reference Date (YYYY-MM-DD)", "2026-09-20"};
    
    CustomInputBox searchInput  = {(Rectangle){0, 75, 190, 34}, "", 127, false, NULL, "Search Title..."};
    
    ActiveTab currentTab = TAB_CATALOG;
    Book* searchResults[100];
    int searchResultCount = 0;
    bool searchPerformed = false;
    
    // Default Today Date Integer YYYYMMDD & String YYYY-MM-DD
    time_t t = time(NULL);
    struct tm tm = *localtime(&t);
    int defaultToday = (tm.tm_year + 1900) * 10000 + (tm.tm_mon + 1) * 100 + tm.tm_mday;
    formatDateIntToString(defaultToday, refDateInput.text, sizeof(refDateInput.text));
    
    // Application Render Loop
    while (!WindowShouldClose()) {
        float deltaTime = GetFrameTime();
        if (toast.displayTimer > 0) toast.displayTimer -= deltaTime;
        
        // Dynamic screen dimensions
        int screenW = GetScreenWidth();
        int screenH = GetScreenHeight();
        
        int mainX = 300;
        int mainWidth = screenW - mainX - 20;
        searchInput.rect.x = (float)(mainX + mainWidth - 370);
        
        // Update Custom Text Inputs
        UpdateCustomInput(&titleInput);
        UpdateCustomInput(&authorInput);
        UpdateCustomInput(&dateInput);
        UpdateCustomInput(&searchInput);
        UpdateCustomInput(&refDateInput);
        
        // Tab key navigation between form inputs
        if (IsKeyPressed(KEY_TAB)) {
            if (titleInput.active) { titleInput.active = false; authorInput.active = true; }
            else if (authorInput.active) { authorInput.active = false; dateInput.active = true; }
            else if (dateInput.active) { dateInput.active = false; titleInput.active = true; }
        }
        
        // Dynamic Palette Colors
        Color currentBg       = isDarkMode ? BG_DARK : BG_LIGHT;
        Color currentPanel    = isDarkMode ? PANEL_DARK : PANEL_LIGHT;
        Color currentText     = isDarkMode ? TEXT_DARK : TEXT_LIGHT;
        Color currentMuted    = isDarkMode ? MUTED_TEXT_DARK : MUTED_TEXT_LIGHT;
        Color currentAccent   = isDarkMode ? ACCENT_SAGE_DARK : ACCENT_SAGE;
        Color currentAlert    = isDarkMode ? ALERT_AMBER_DARK : ALERT_AMBER;
        Color currentRed      = isDarkMode ? ALERT_RED_DARK : ALERT_RED_LIGHT;
        Color currentHeader   = isDarkMode ? CARD_HEADER_DARK : CARD_HEADER_LIGHT;
        Color currentBorder   = isDarkMode ? BORDER_DARK : BORDER_LIGHT;
        Color currentAltRow   = isDarkMode ? ROW_ALT_DARK : ROW_ALT_LIGHT;
        Color currentInputBg  = isDarkMode ? (Color){20, 28, 42, 255} : (Color){255, 255, 255, 255};
        
        BeginDrawing();
        ClearBackground(currentBg);
        
        // --- TOP HEADER BAR ---
        DrawRectangle(0, 0, screenW, 55, currentPanel);
        DrawRectangle(0, 54, screenW, 1, currentBorder);
        
        // Title Banner: 22px
        DrawAppText("SMART LIBRARY", 20, 16, 22.0f, currentText);
        DrawAppText("INVENTORY & DUE-DATE TRACKER", 195, 20, 15.0f, currentMuted);
        
        // Header Status Badges (13px Text)
        int badge1X = screenW - 610;
        int badge2X = screenW - 490;
        int modeBtnX = screenW - 350;
        int toastX   = screenW - 220;
        
        if (badge1X > 400) {
            DrawRectangleRounded((Rectangle){(float)badge1X, 14, 110, 26}, 0.3f, 4, Fade(currentAccent, 0.15f));
            DrawRectangleRoundedLinesEx((Rectangle){(float)badge1X, 14, 110, 26}, 0.3f, 4, 1.0f, currentAccent);
            DrawAppText("BST Catalog", badge1X + 16, 20, 13.0f, currentAccent);
        }
        
        if (badge2X > 510) {
            DrawRectangleRounded((Rectangle){(float)badge2X, 14, 125, 26}, 0.3f, 4, Fade(currentRed, 0.15f));
            DrawRectangleRoundedLinesEx((Rectangle){(float)badge2X, 14, 125, 26}, 0.3f, 4, 1.0f, currentRed);
            DrawAppText("Min-Heap Overdue", badge2X + 13, 20, 13.0f, currentRed);
        }
        
        // Mode Toggle Button (15px Button Text)
        const char* modeText = isDarkMode ? "MODE: DARK" : "MODE: LIGHT";
        if (DrawExecutiveButton((Rectangle){(float)modeBtnX, 12, 115, 30}, modeText, currentPanel, isDarkMode ? (Color){51, 65, 85, 255} : (Color){226, 232, 240, 255}, currentText, currentBorder)) {
            isDarkMode = !isDarkMode;
            showToast(isDarkMode ? "Executive Dark Mode Enabled" : "Paper & Sage Light Mode Enabled", currentAccent);
        }
        
        // Popup Stats Window Button (Top Header)
        if (DrawExecutiveButton((Rectangle){ (float)(screenW - 225), 12, 100, 30 }, "[STATS]", currentAccent, isDarkMode ? (Color){4, 120, 87, 255} : (Color){15, 118, 110, 255}, RAYWHITE, currentAccent)) {
            statsModal.active = !statsModal.active;
        }
        
        // Toast Notification Area (14px Text)
        if (toast.displayTimer > 0) {
            int toastW = 200;
            Rectangle toastRect = {(float)toastX, 12, (float)toastW, 30};
            DrawRectangleRounded(toastRect, 0.4f, 4, Fade(toast.color, 0.2f));
            DrawRectangleRoundedLinesEx(toastRect, 0.4f, 4, 1.0f, toast.color);
            
            int textW = MeasureAppText(toast.message, 14.0f);
            int textX = toastX + (toastW - textW) / 2;
            if (textX < toastX + 8) textX = toastX + 8;
            DrawAppText(toast.message, textX, 20, 14.0f, currentText);
        }
        
        // --- LEFT SIDEBAR PANEL (Width: 280px) ---
        DrawRectangle(0, 55, 280, screenH - 55, currentPanel);
        DrawRectangle(279, 55, 1, screenH - 55, currentBorder);
        
        // Section Header: 16px (Bold/Medium)
        DrawAppText("INVENTORY FORM", 20, 72, 16.0f, currentAccent);
        DrawRectangle(20, 92, 240, 1, Fade(currentAccent, 0.3f));
        
        DrawCustomInput(&titleInput, currentText, currentMuted, currentInputBg, currentBorder, currentAccent);
        DrawCustomInput(&authorInput, currentText, currentMuted, currentInputBg, currentBorder, currentAccent);
        
        DrawCustomInput(&dateInput, currentText, currentMuted, currentInputBg, currentBorder, currentAccent);
        if (DrawExecutiveButton((Rectangle){216, 245, 44, 36}, "CAL", (calPicker.active && calPicker.targetInput == &dateInput) ? currentAccent : currentHeader, isDarkMode ? (Color){4, 120, 87, 255} : (Color){15, 118, 110, 255}, (calPicker.active && calPicker.targetInput == &dateInput) ? RAYWHITE : currentText, currentBorder)) {
            if (calPicker.active && calPicker.targetInput == &dateInput) {
                calPicker.active = false;
            } else {
                openCalendarForInput(&calPicker, &dateInput, (Vector2){275, 200});
            }
        }
        
        // ADD BOOK BUTTON (15px Button Text)
        if (DrawExecutiveButton((Rectangle){20, 292, 240, 38}, "+ ADD TO CATALOG & HEAP", currentAccent, isDarkMode ? (Color){4, 120, 87, 255} : (Color){15, 118, 110, 255}, RAYWHITE, currentAccent)) {
            if (strlen(titleInput.text) == 0 || strlen(authorInput.text) == 0) {
                showToast("Error: Title and Author required!", currentRed);
            } else {
                int dueDateVal = parseDateStringToInt(dateInput.text);
                if (dueDateVal <= 0) dueDateVal = defaultToday;
                
                Book* newB = addBookController(ctrl, 0, titleInput.text, authorInput.text, dueDateVal);
                if (newB) {
                    showToast("Book added to Catalog & Heap!", currentAccent);
                    titleInput.text[0] = '\0';
                    authorInput.text[0] = '\0';
                }
            }
        }
        
        DrawRectangle(20, 348, 240, 1, currentBorder);
        
        DrawCustomInput(&refDateInput, currentText, currentMuted, currentInputBg, currentBorder, currentAlert);
        if (DrawExecutiveButton((Rectangle){216, 370, 44, 36}, "CAL", (calPicker.active && calPicker.targetInput == &refDateInput) ? currentAlert : currentHeader, isDarkMode ? (Color){180, 95, 4, 255} : (Color){180, 83, 9, 255}, (calPicker.active && calPicker.targetInput == &refDateInput) ? RAYWHITE : currentText, currentBorder)) {
            if (calPicker.active && calPicker.targetInput == &refDateInput) {
                calPicker.active = false;
            } else {
                openCalendarForInput(&calPicker, &refDateInput, (Vector2){275, 300});
            }
        }
        
        // EVALUATE OVERDUE BUTTON (15px Button Text)
        if (DrawExecutiveButton((Rectangle){20, 418, 240, 38}, "EVALUATE OVERDUE (MIN-HEAP)", currentAlert, isDarkMode ? (Color){180, 95, 4, 255} : (Color){180, 83, 9, 255}, RAYWHITE, currentAlert)) {
            currentTab = TAB_OVERDUE;
            int refDate = parseDateStringToInt(refDateInput.text);
            Book* overdueBuf[100];
            int count = checkOverdueController(ctrl, refDate, overdueBuf, 100);
            
            char formattedRefDate[16];
            formatDateIntToString(refDate, formattedRefDate, sizeof(formattedRefDate));
            
            char msg[64];
            snprintf(msg, sizeof(msg), "Found %d overdue books for %s", count, formattedRefDate);
            showToast(msg, currentAlert);
        }
        
        // DISK CONTROL BUTTONS (15px Button Text)
        if (DrawExecutiveButton((Rectangle){20, 468, 240, 34}, "RELOAD FROM DISK", currentHeader, isDarkMode ? (Color){71, 85, 105, 255} : (Color){226, 232, 240, 255}, currentText, currentBorder)) {
            loadController(ctrl);
            showToast("Reloaded snapshot from disk.", (Color){59, 130, 246, 255});
        }
        
        if (DrawExecutiveButton((Rectangle){20, 510, 240, 34}, "SAVE DISK SNAPSHOT", currentAccent, isDarkMode ? (Color){4, 120, 87, 255} : (Color){15, 118, 110, 255}, RAYWHITE, currentAccent)) {
            saveController(ctrl);
            showToast("Saved to library_data.txt", currentAccent);
        }
        
        // SIDEBAR STATS POPUP BUTTON
        if (DrawExecutiveButton((Rectangle){ 20, 552, 240, 34 }, "[STATS] VIEW STATS POPUP", ALERT_AMBER, isDarkMode ? (Color){180, 83, 9, 255} : (Color){217, 119, 6, 255}, RAYWHITE, ALERT_AMBER)) {
            statsModal.active = true;
        }
        
        // Bottom Sidebar Status Box (14px Text)
        int totalBooks = getTotalBooksController(ctrl);
        Rectangle totalBox = { 20, (float)(screenH - 55), 240, 40 };
        DrawRectangleRounded(totalBox, 0.2f, 4, currentBg);
        DrawRectangleRoundedLinesEx(totalBox, 0.2f, 4, 1.0f, currentBorder);
        
        int totalTextW = MeasureAppText(TextFormat("Total Inventory: %d Books", totalBooks), 14.0f);
        DrawAppText(TextFormat("Total Inventory: %d Books", totalBooks), (int)totalBox.x + ((int)totalBox.width - totalTextW) / 2, (int)totalBox.y + 12, 14.0f, currentText);
        
        // --- RIGHT MAIN CONTENT AREA ---
        
        // TAB BUTTONS (15px Button Text)
        if (DrawExecutiveButton((Rectangle){(float)mainX, 75, 115, 34}, "Full Catalog", currentTab == TAB_CATALOG ? currentAccent : currentPanel, isDarkMode ? (Color){4, 120, 87, 255} : (Color){15, 118, 110, 255}, currentTab == TAB_CATALOG ? RAYWHITE : currentText, currentBorder)) {
            currentTab = TAB_CATALOG;
        }
        
        if (DrawExecutiveButton((Rectangle){(float)mainX + 123, 75, 125, 34}, "Overdue Engine", currentTab == TAB_OVERDUE ? currentRed : currentPanel, isDarkMode ? (Color){190, 18, 60, 255} : (Color){190, 18, 60, 255}, currentTab == TAB_OVERDUE ? RAYWHITE : currentText, currentBorder)) {
            currentTab = TAB_OVERDUE;
        }
        
        if (DrawExecutiveButton((Rectangle){(float)mainX + 256, 75, 125, 34}, "Search Results", currentTab == TAB_SEARCH ? currentAccent : currentPanel, isDarkMode ? (Color){4, 120, 87, 255} : (Color){15, 118, 110, 255}, currentTab == TAB_SEARCH ? RAYWHITE : currentText, currentBorder)) {
            currentTab = TAB_SEARCH;
        }
        
        if (DrawExecutiveButton((Rectangle){(float)mainX + 389, 75, 145, 34}, "⚡ 1M Benchmark", currentTab == TAB_BENCHMARK ? ALERT_AMBER : currentPanel, isDarkMode ? (Color){180, 83, 9, 255} : (Color){217, 119, 6, 255}, currentTab == TAB_BENCHMARK ? RAYWHITE : currentText, currentBorder)) {
            currentTab = TAB_BENCHMARK;
        }
        
        // TOP RIGHT CONTROLS: SEARCH INPUT & ACTION BUTTONS
        DrawCustomInput(&searchInput, currentText, currentMuted, currentInputBg, currentBorder, currentAccent);
        
        bool triggerSearch = DrawExecutiveButton((Rectangle){(float)(mainX + mainWidth - 170), 75, 90, 34}, "SEARCH", currentAccent, isDarkMode ? (Color){4, 120, 87, 255} : (Color){15, 118, 110, 255}, RAYWHITE, currentAccent);
        if (searchInput.active && (IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_KP_ENTER))) {
            triggerSearch = true;
        }
        
        if (triggerSearch) {
            if (strlen(searchInput.text) > 0) {
                searchResultCount = searchBooksSubstringController(ctrl, searchInput.text, searchResults, 100);
                searchPerformed = true;
                currentTab = TAB_SEARCH;
                if (searchResultCount > 0) {
                    char toastMsg[200];
                    snprintf(toastMsg, sizeof(toastMsg), "Found %d matching books for '%s'", searchResultCount, searchInput.text);
                    showToast(toastMsg, currentAccent);
                } else {
                    char toastMsg[200];
                    snprintf(toastMsg, sizeof(toastMsg), "No books matching '%s' found.", searchInput.text);
                    showToast(toastMsg, currentRed);
                }
            }
        }
        
        if (DrawExecutiveButton((Rectangle){(float)(mainX + mainWidth - 70), 75, 70, 34}, "CLEAR", currentHeader, isDarkMode ? (Color){71, 85, 105, 255} : (Color){226, 232, 240, 255}, currentText, currentBorder)) {
            searchInput.text[0] = '\0';
            searchResultCount = 0;
            searchPerformed = false;
            currentTab = TAB_CATALOG;
        }
        
        // CONTENT PANEL CONTAINER
        Rectangle contentRect = {(float)mainX, 125, (float)mainWidth, (float)(screenH - 145)};
        DrawRectangleRounded(contentRect, 0.08f, 4, currentPanel);
        DrawRectangleRoundedLinesEx(contentRect, 0.08f, 4, 1.0f, currentBorder);
        
        // Dynamic Table Column Offsets
        int colID     = (int)contentRect.x + 15;
        int colTitle  = (int)contentRect.x + 75;
        int colAuthor = (int)contentRect.x + 75 + (int)((contentRect.width - 270) * 0.44f);
        int colDate   = (int)contentRect.x + 75 + (int)((contentRect.width - 270) * 0.72f);
        int colStatus = (int)contentRect.x + (int)contentRect.width - 180;
        int colAction = (int)contentRect.x + (int)contentRect.width - 90;
        
        // Scope Table Header rendering ONLY to catalog/overdue/search tabs (not benchmark tab)
        if (currentTab == TAB_CATALOG || currentTab == TAB_OVERDUE || currentTab == TAB_SEARCH) {
            DrawRectangle((int)contentRect.x + 1, (int)contentRect.y + 1, (int)contentRect.width - 2, 38, currentHeader);
            DrawAppText("ID", colID, (int)contentRect.y + 11, 15.0f, currentMuted);
            DrawAppText("TITLE", colTitle, (int)contentRect.y + 11, 15.0f, currentMuted);
            DrawAppText("AUTHOR", colAuthor, (int)contentRect.y + 11, 15.0f, currentMuted);
            DrawAppText("DUE DATE", colDate, (int)contentRect.y + 11, 15.0f, currentMuted);
            DrawAppText("STATUS", colStatus, (int)contentRect.y + 11, 15.0f, currentMuted);
            DrawAppText("ACTION", colAction, (int)contentRect.y + 11, 15.0f, currentMuted);
        }
        
        int refDateVal = parseDateStringToInt(refDateInput.text);
        if (refDateVal <= 0) refDateVal = defaultToday;
        
        // --- RENDER TAB 1: FULL CATALOG (14px Table Row Data via DrawTextEx) ---
        if (currentTab == TAB_CATALOG) {
            Book* catalogBuf[200];
            int count = getCatalogController(ctrl, catalogBuf, 200);
            
            if (count == 0) {
                DrawAppText("Catalog is empty. Add a book to get started.", (int)contentRect.x + 200, (int)contentRect.y + 150, 15.0f, currentMuted);
            } else {
                int rowY = (int)contentRect.y + 45;
                for (int i = 0; i < count && rowY < screenH - 40; i++) {
                    Book* b = catalogBuf[i];
                    bool isOverdue = (!b->isReturned && b->dueDate <= refDateVal);
                    
                    Color rowBg = (i % 2 == 0) ? currentAltRow : currentPanel;
                    DrawRectangle((int)contentRect.x + 2, rowY, (int)contentRect.width - 4, 38, rowBg);
                    
                    // 14px Table Row Data via DrawTextEx
                    DrawAppText(TextFormat("#%d", b->id), colID, rowY + 10, 14.0f, currentText);
                    
                    char truncTitle[44];
                    strncpy(truncTitle, b->title, 40);
                    truncTitle[40] = '\0';
                    DrawAppText(truncTitle, colTitle, rowY + 10, 14.0f, currentText);
                    
                    char truncAuthor[28];
                    strncpy(truncAuthor, b->author, 25);
                    truncAuthor[25] = '\0';
                    DrawAppText(truncAuthor, colAuthor, rowY + 10, 14.0f, currentMuted);
                    
                    char formattedDate[16];
                    formatDateIntToString(b->dueDate, formattedDate, sizeof(formattedDate));
                    DrawAppText(formattedDate, colDate, rowY + 10, 14.0f, currentText);
                    
                    if (b->isReturned) {
                        DrawRectangleRounded((Rectangle){(float)colStatus - 5, (float)rowY + 7, 85, 24}, 0.3f, 4, isDarkMode ? (Color){30, 58, 138, 255} : (Color){219, 234, 254, 255});
                        DrawAppText("RETURNED", colStatus + 6, rowY + 11, 12.0f, isDarkMode ? (Color){147, 197, 253, 255} : (Color){29, 78, 216, 255});
                    } else if (isOverdue) {
                        DrawRectangleRounded((Rectangle){(float)colStatus - 5, (float)rowY + 7, 85, 24}, 0.3f, 4, currentRed);
                        DrawAppText("OVERDUE", colStatus + 8, rowY + 11, 12.0f, RAYWHITE);
                    } else {
                        DrawRectangleRounded((Rectangle){(float)colStatus - 5, (float)rowY + 7, 75, 24}, 0.3f, 4, currentAccent);
                        DrawAppText("ACTIVE", colStatus + 12, rowY + 11, 12.0f, RAYWHITE);
                    }
                    
                    // RETURN Action Button
                    if (!b->isReturned) {
                        if (DrawExecutiveButton((Rectangle){(float)colAction - 5, (float)rowY + 7, 75, 24}, "RETURN", currentAccent, isDarkMode ? (Color){4, 120, 87, 255} : (Color){15, 118, 110, 255}, RAYWHITE, currentAccent)) {
                            returnBookController(ctrl, b->id);
                            char toastMsg[64];
                            snprintf(toastMsg, sizeof(toastMsg), "Book #%d marked as Returned!", b->id);
                            showToast(toastMsg, currentAccent);
                        }
                    } else {
                        DrawAppText("-", colAction + 30, rowY + 10, 14.0f, currentMuted);
                    }
                    
                    DrawLine((int)contentRect.x + 5, rowY + 38, (int)contentRect.x + (int)contentRect.width - 5, rowY + 38, Fade(currentBorder, 0.5f));
                    
                    rowY += 42;
                }
            }
        }
        
        // --- RENDER TAB 2: OVERDUE ENGINE (14px Table Row Data via DrawTextEx) ---
        else if (currentTab == TAB_OVERDUE) {
            Book* overdueBuf[200];
            int count = checkOverdueController(ctrl, refDateVal, overdueBuf, 200);
            
            char formattedRefDate[16];
            formatDateIntToString(refDateVal, formattedRefDate, sizeof(formattedRefDate));
            
            DrawRectangleRounded((Rectangle){(float)contentRect.x + 10, (float)contentRect.y + 45, (float)contentRect.width - 20, 38}, 0.2f, 4, Fade(currentRed, 0.15f));
            DrawRectangleRoundedLinesEx((Rectangle){(float)contentRect.x + 10, (float)contentRect.y + 45, (float)contentRect.width - 20, 38}, 0.2f, 4, 1.0f, currentRed);
            DrawAppText(TextFormat("MIN-HEAP PRIORITY EVALUATION: %d Overdue Books against Reference Date [%s]", count, formattedRefDate), (int)contentRect.x + 20, (int)contentRect.y + 57, 14.0f, currentRed);
            
            if (count == 0) {
                DrawAppText("No active books are overdue as of this reference date.", (int)contentRect.x + 180, (int)contentRect.y + 160, 15.0f, currentAccent);
            } else {
                int rowY = (int)contentRect.y + 92;
                for (int i = 0; i < count && rowY < screenH - 40; i++) {
                    Book* b = overdueBuf[i];
                    
                    Color rowBg = (i == 0) ? (isDarkMode ? (Color){70, 30, 40, 255} : (Color){254, 226, 226, 255}) : ((i % 2 == 0) ? currentAltRow : currentPanel);
                    DrawRectangle((int)contentRect.x + 2, rowY, (int)contentRect.width - 4, 38, rowBg);
                    
                    if (i == 0) {
                        DrawAppText("URGENT #1", colID - 10, rowY + 10, 12.0f, currentRed);
                    } else {
                        DrawAppText(TextFormat("#%d", b->id), colID, rowY + 10, 14.0f, currentText);
                    }
                    
                    char truncTitle[44];
                    strncpy(truncTitle, b->title, 40);
                    truncTitle[40] = '\0';
                    DrawAppText(truncTitle, colTitle, rowY + 10, 14.0f, currentText);
                    
                    char truncAuthor[28];
                    strncpy(truncAuthor, b->author, 25);
                    truncAuthor[25] = '\0';
                    DrawAppText(truncAuthor, colAuthor, rowY + 10, 14.0f, currentMuted);
                    
                    char formattedDate[16];
                    formatDateIntToString(b->dueDate, formattedDate, sizeof(formattedDate));
                    DrawAppText(formattedDate, colDate, rowY + 10, 14.0f, currentRed);
                    
                    DrawRectangleRounded((Rectangle){(float)colStatus - 5, (float)rowY + 7, 85, 24}, 0.3f, 4, currentRed);
                    DrawAppText("OVERDUE", colStatus + 8, rowY + 11, 12.0f, RAYWHITE);
                    
                    if (DrawExecutiveButton((Rectangle){(float)colAction - 5, (float)rowY + 7, 75, 24}, "RETURN", currentAccent, isDarkMode ? (Color){4, 120, 87, 255} : (Color){15, 118, 110, 255}, RAYWHITE, currentAccent)) {
                        returnBookController(ctrl, b->id);
                        char toastMsg[64];
                        snprintf(toastMsg, sizeof(toastMsg), "Book #%d marked as Returned!", b->id);
                        showToast(toastMsg, currentAccent);
                    }
                    
                    DrawLine((int)contentRect.x + 5, rowY + 38, (int)contentRect.x + (int)contentRect.width - 5, rowY + 38, Fade(currentBorder, 0.5f));
                    
                    rowY += 42;
                }
            }
        }
        
        // --- RENDER TAB 3: SUBSTRING SEARCH RESULTS VIEW ---
        else if (currentTab == TAB_SEARCH) {
            if (!searchPerformed) {
                DrawAppText("Type a book title keyword in the top-right search box and click SEARCH (or press Enter).", (int)contentRect.x + 100, (int)contentRect.y + 150, 15.0f, currentMuted);
            } else if (searchResultCount == 0) {
                DrawRectangleRounded((Rectangle){(float)contentRect.x + 40, (float)contentRect.y + 80, (float)contentRect.width - 80, 100}, 0.2f, 4, Fade(currentRed, 0.15f));
                DrawRectangleRoundedLinesEx((Rectangle){(float)contentRect.x + 40, (float)contentRect.y + 80, (float)contentRect.width - 80, 100}, 0.2f, 4, 1.0f, currentRed);
                DrawAppText("SUBSTRING SEARCH RESULT: NOT FOUND", (int)contentRect.x + 60, (int)contentRect.y + 100, 16.0f, currentRed);
                DrawAppText(TextFormat("No books matching keyword '%s' exist in the BST catalog.", searchInput.text), (int)contentRect.x + 60, (int)contentRect.y + 130, 14.0f, currentText);
            } else {
                DrawRectangleRounded((Rectangle){(float)contentRect.x + 10, (float)contentRect.y + 45, (float)contentRect.width - 20, 38}, 0.2f, 4, Fade(currentAccent, 0.15f));
                DrawRectangleRoundedLinesEx((Rectangle){(float)contentRect.x + 10, (float)contentRect.y + 45, (float)contentRect.width - 20, 38}, 0.2f, 4, 1.0f, currentAccent);
                DrawAppText(TextFormat("SUBSTRING SEARCH MATCHES: Found %d Books for Query ['%s']", searchResultCount, searchInput.text), (int)contentRect.x + 20, (int)contentRect.y + 57, 14.0f, currentAccent);
                
                int rowY = (int)contentRect.y + 92;
                for (int i = 0; i < searchResultCount && rowY < screenH - 40; i++) {
                    Book* b = searchResults[i];
                    bool isOverdue = (!b->isReturned && b->dueDate <= refDateVal);
                    
                    Color rowBg = (i % 2 == 0) ? currentAltRow : currentPanel;
                    DrawRectangle((int)contentRect.x + 2, rowY, (int)contentRect.width - 4, 38, rowBg);
                    
                    DrawAppText(TextFormat("#%d", b->id), colID, rowY + 10, 14.0f, currentText);
                    
                    char truncTitle[44];
                    strncpy(truncTitle, b->title, 40);
                    truncTitle[40] = '\0';
                    DrawAppText(truncTitle, colTitle, rowY + 10, 14.0f, currentText);
                    
                    char truncAuthor[28];
                    strncpy(truncAuthor, b->author, 25);
                    truncAuthor[25] = '\0';
                    DrawAppText(truncAuthor, colAuthor, rowY + 10, 14.0f, currentMuted);
                    
                    char formattedDate[16];
                    formatDateIntToString(b->dueDate, formattedDate, sizeof(formattedDate));
                    DrawAppText(formattedDate, colDate, rowY + 10, 14.0f, currentText);
                    
                    if (b->isReturned) {
                        DrawRectangleRounded((Rectangle){(float)colStatus - 5, (float)rowY + 7, 85, 24}, 0.3f, 4, isDarkMode ? (Color){30, 58, 138, 255} : (Color){219, 234, 254, 255});
                        DrawAppText("RETURNED", colStatus + 6, rowY + 11, 12.0f, isDarkMode ? (Color){147, 197, 253, 255} : (Color){29, 78, 216, 255});
                    } else if (isOverdue) {
                        DrawRectangleRounded((Rectangle){(float)colStatus - 5, (float)rowY + 7, 85, 24}, 0.3f, 4, currentRed);
                        DrawAppText("OVERDUE", colStatus + 8, rowY + 11, 12.0f, RAYWHITE);
                    } else {
                        DrawRectangleRounded((Rectangle){(float)colStatus - 5, (float)rowY + 7, 75, 24}, 0.3f, 4, currentAccent);
                        DrawAppText("ACTIVE", colStatus + 12, rowY + 11, 12.0f, RAYWHITE);
                    }
                    
                    if (!b->isReturned) {
                        if (DrawExecutiveButton((Rectangle){(float)colAction - 5, (float)rowY + 7, 75, 24}, "RETURN", currentAccent, isDarkMode ? (Color){4, 120, 87, 255} : (Color){15, 118, 110, 255}, RAYWHITE, currentAccent)) {
                            returnBookController(ctrl, b->id);
                            char toastMsg[64];
                            snprintf(toastMsg, sizeof(toastMsg), "Book #%d marked as Returned!", b->id);
                            showToast(toastMsg, currentAccent);
                        }
                    } else {
                        DrawAppText("-", colAction + 30, rowY + 10, 14.0f, currentMuted);
                    }
                    
                    DrawLine((int)contentRect.x + 5, rowY + 38, (int)contentRect.x + (int)contentRect.width - 5, rowY + 38, Fade(currentBorder, 0.5f));
                    
                    rowY += 42;
                }
            }
        }
        
        // --- RENDER TAB 4: BENCHMARK LAB (1M & 10M CONTIGUOUS SLAB) ---
        else if (currentTab == TAB_BENCHMARK) {
            DrawRectangleRounded((Rectangle){(float)contentRect.x + 10, (float)contentRect.y + 12, (float)contentRect.width - 20, 38}, 0.2f, 4, Fade(ALERT_AMBER, 0.15f));
            DrawRectangleRoundedLinesEx((Rectangle){(float)contentRect.x + 10, (float)contentRect.y + 12, (float)contentRect.width - 20, 38}, 0.2f, 4, 1.0f, ALERT_AMBER);
            DrawAppText(TextFormat("[BENCHMARK LAB] HIGH-PERFORMANCE SUITE: [%s MODE]", benchMode10M ? "10,000,000 RECORD SLAB" : "1,000,000 RECORD DATASET"), (int)contentRect.x + 20, (int)contentRect.y + 22, 15.0f, ALERT_AMBER);

            // Mode Selector Buttons
            if (DrawExecutiveButton((Rectangle){(float)contentRect.x + (float)contentRect.width - 290, (float)contentRect.y + 14, 130, 34}, "1M Dataset", !benchMode10M ? currentAccent : currentHeader, isDarkMode ? (Color){4, 120, 87, 255} : (Color){15, 118, 110, 255}, !benchMode10M ? RAYWHITE : currentText, currentBorder)) {
                benchMode10M = false;
                benchHasRes = false;
                benchHasStress = false;
            }
            if (DrawExecutiveButton((Rectangle){(float)contentRect.x + (float)contentRect.width - 150, (float)contentRect.y + 14, 140, 34}, "10M Slab Dataset", benchMode10M ? ALERT_AMBER : currentHeader, isDarkMode ? (Color){180, 83, 9, 255} : (Color){217, 119, 6, 255}, benchMode10M ? RAYWHITE : currentText, currentBorder)) {
                benchMode10M = true;
                benchHasRes = false;
                benchHasStress = false;
            }

            bool isDatasetLoaded = benchMode10M ? (bench10MDs != NULL) : (benchDs != NULL);

            if (!isDatasetLoaded) {
                // Initial State: Prompt user to generate dataset
                DrawRectangleRounded((Rectangle){(float)contentRect.x + 40, (float)contentRect.y + 65, (float)contentRect.width - 80, 220}, 0.15f, 4, currentPanel);
                DrawRectangleRoundedLinesEx((Rectangle){(float)contentRect.x + 40, (float)contentRect.y + 65, (float)contentRect.width - 80, 220}, 0.15f, 4, 1.2f, currentBorder);

                DrawAppText(TextFormat("Synthetic Dataset Status: [%s] Not Initialized", benchMode10M ? "10,000,000 Records" : "1,000,000 Records"), (int)contentRect.x + 60, (int)contentRect.y + 90, 16.0f, currentText);
                DrawAppText(TextFormat("Click below to allocate %s dynamically on the heap.", benchMode10M ? "10,000,000 records (~2.82 GB Contiguous Memory Slab)" : "1,000,000 records (~537 MB RAM)"), (int)contentRect.x + 60, (int)contentRect.y + 120, 14.0f, currentMuted);
                DrawAppText("• Dual-Engine Index: Iterative O(log n) BST (Title Order) + O(1) Min-Heap Priority Queue", (int)contentRect.x + 60, (int)contentRect.y + 145, 14.0f, currentAccent);
                DrawAppText("• Traditional Index: Contiguous Memory Array (Full O(n) Linear Scan Baseline)", (int)contentRect.x + 60, (int)contentRect.y + 165, 14.0f, currentRed);

                const char* btnLabel = benchMode10M ? "[FAST] ALLOCATE 10,000,000 SLAB (~2.82 GB)" : "[FAST] ALLOCATE 1,000,000 DATASET";
                if (DrawExecutiveButton((Rectangle){(float)contentRect.x + 60, (float)contentRect.y + 200, 390, 42}, btnLabel, benchMode10M ? ALERT_AMBER : currentAccent, isDarkMode ? (Color){4, 120, 87, 255} : (Color){15, 118, 110, 255}, RAYWHITE, currentAccent)) {
                    if (benchMode10M) {
                        showToast("Allocating 10,000,000 contiguous slab on heap...", ALERT_AMBER);
                        bench10MDs = initBenchmark10MDataset(10000000);
                        if (bench10MDs) {
                            benchRes = run10MBenchmark(bench10MDs, "Clean Code Architecture 10M");
                            benchHasRes = true;
                            showToast("Loaded 10,000,000 slab (~2.82 GB RAM)!", currentAccent);
                        }
                    } else {
                        showToast("Initializing 1,000,000 records on heap...", ALERT_AMBER);
                        benchDs = initBenchmarkDataset(1000000);
                        if (benchDs) {
                            benchRes = runDualVsTraditionalBenchmark(benchDs, "Clean Code Architecture");
                            benchHasRes = true;
                            showToast("Loaded 1,000,000 records (537 MB RAM)!", currentAccent);
                        }
                    }
                }
            } else {
                // Dataset Loaded: Show Controls & Benchmarks
                int ctrlY = (int)contentRect.y + 58;

                // Action Controls
                if (DrawExecutiveButton((Rectangle){(float)contentRect.x + 10, (float)ctrlY, 195, 36}, "[FAST] RUN DUAL-ENGINE", currentAccent, isDarkMode ? (Color){4, 120, 87, 255} : (Color){15, 118, 110, 255}, RAYWHITE, currentAccent)) {
                    if (benchMode10M) {
                        benchRes = run10MBenchmark(bench10MDs, "Clean Code Architecture 10M");
                    } else {
                        benchRes = runDualVsTraditionalBenchmark(benchDs, "Clean Code Architecture");
                    }
                    benchHasRes = true;
                    showToast("Dual-Engine O(log n) & O(1) query complete!", currentAccent);
                }

                if (DrawExecutiveButton((Rectangle){(float)contentRect.x + 215, (float)ctrlY, 205, 36}, "[SLOW] RUN LINEAR SCAN", currentRed, isDarkMode ? (Color){190, 18, 60, 255} : (Color){190, 18, 60, 255}, RAYWHITE, currentRed)) {
                    if (benchMode10M) {
                        benchRes = run10MBenchmark(bench10MDs, "Systems Programming in C 10M");
                    } else {
                        benchRes = runDualVsTraditionalBenchmark(benchDs, "Systems Programming in C");
                    }
                    benchHasRes = true;
                    showToast("Traditional linear scan complete!", currentRed);
                }

                if (DrawExecutiveButton((Rectangle){(float)contentRect.x + 430, (float)ctrlY, 210, 36}, "[LOOP] STRESS LOOP (10x)", ALERT_AMBER, isDarkMode ? (Color){180, 83, 9, 255} : (Color){217, 119, 6, 255}, RAYWHITE, ALERT_AMBER)) {
                    if (benchMode10M) {
                        benchStress = run10MStressLoopBenchmark(bench10MDs, 10);
                    } else {
                        benchStress = runStressLoopBenchmark(benchDs, 10);
                    }
                    benchHasStress = true;
                    showToast("10-iteration continuous stress loop complete!", ALERT_AMBER);
                }

                if (DrawExecutiveButton((Rectangle){(float)contentRect.x + 650, (float)ctrlY, 150, 36}, "[CLEAR] UNLOAD DATA", currentHeader, isDarkMode ? (Color){71, 85, 105, 255} : (Color){226, 232, 240, 255}, currentText, currentBorder)) {
                    if (benchMode10M && bench10MDs) {
                        freeBenchmark10MDataset(bench10MDs);
                        bench10MDs = NULL;
                    } else if (benchDs) {
                        freeBenchmarkDataset(benchDs);
                        benchDs = NULL;
                    }
                    benchHasRes = false;
                    benchHasStress = false;
                    showToast("Released dataset memory in < 1 ms.", currentMuted);
                }

                // Cards Container
                int cardY = ctrlY + 46;
                float halfWidth = ((float)contentRect.width - 30) / 2.0f;

                // Left Card: Dual-Engine Data Structure
                Rectangle dualCard = {(float)contentRect.x + 10, (float)cardY, halfWidth, 205};
                DrawRectangleRounded(dualCard, 0.12f, 4, Fade(currentAccent, 0.1f));
                DrawRectangleRoundedLinesEx(dualCard, 0.12f, 4, 1.2f, currentAccent);

                DrawAppText(TextFormat("[FAST] DUAL-ENGINE INDEX (%s)", benchMode10M ? "10M SLAB" : "1M TREE"), (int)dualCard.x + 15, (int)dualCard.y + 15, 16.0f, currentAccent);
                if (benchHasRes) {
                    DrawAppText(TextFormat("Title Search Latency (BST): %.3f us (%.5f ms)", benchRes.bstTimeUs, benchRes.bstTimeUs / 1000.0), (int)dualCard.x + 15, (int)dualCard.y + 45, 15.0f, currentText);
                    DrawAppText(TextFormat("Earliest Due Peek (Min-Heap): %.3f us (%.5f ms)", benchRes.minHeapTimeUs, benchRes.minHeapTimeUs / 1000.0), (int)dualCard.x + 15, (int)dualCard.y + 72, 15.0f, currentText);
                    DrawAppText("Algorithmic Complexity: Iterative O(log n) Title / O(1) Heap Peek", (int)dualCard.x + 15, (int)dualCard.y + 105, 13.0f, currentMuted);
                    DrawAppText(TextFormat("Comparisons Required: ~%d Tree Comparisons / 1 Root Read", benchMode10M ? 24 : 20), (int)dualCard.x + 15, (int)dualCard.y + 130, 13.0f, currentMuted);
                    DrawAppText(TextFormat("Matched Book ID: #%d (Due Date: %d)", benchRes.foundId, benchRes.dualMinDueDate), (int)dualCard.x + 15, (int)dualCard.y + 162, 15.0f, currentAccent);
                }

                // Right Card: Traditional Linear Array Scan
                Rectangle linearCard = {(float)contentRect.x + 20 + halfWidth, (float)cardY, halfWidth, 205};
                DrawRectangleRounded(linearCard, 0.12f, 4, Fade(currentRed, 0.1f));
                DrawRectangleRoundedLinesEx(linearCard, 0.12f, 4, 1.2f, currentRed);

                DrawAppText(TextFormat("[SLOW] TRADITIONAL LINEAR SCAN (%s)", benchMode10M ? "10M SLAB SCAN" : "1M FLAT ARRAY"), (int)linearCard.x + 15, (int)linearCard.y + 15, 16.0f, currentRed);
                if (benchHasRes) {
                    DrawAppText(TextFormat("Title Search Latency (Linear): %.3f ms (%.1f us)", benchRes.linearSearchTimeMs, benchRes.linearSearchTimeMs * 1000.0), (int)linearCard.x + 15, (int)linearCard.y + 45, 15.0f, currentText);
                    DrawAppText(TextFormat("Earliest Due Scan (Linear): %.3f ms (%.1f us)", benchRes.linearHeapTimeMs, benchRes.linearHeapTimeMs * 1000.0), (int)linearCard.x + 15, (int)linearCard.y + 72, 15.0f, currentText);
                    DrawAppText("Algorithmic Complexity: Full O(n) Contiguous Memory Traversal", (int)linearCard.x + 15, (int)linearCard.y + 105, 13.0f, currentMuted);
                    DrawAppText(TextFormat("Comparisons Required: %s Contiguous Memory Structs", benchMode10M ? "10,000,000" : "1,000,000"), (int)linearCard.x + 15, (int)linearCard.y + 130, 13.0f, currentMuted);
                    DrawAppText(TextFormat("Matched Book ID: #%d (Due Date: %d)", benchRes.foundId, benchRes.traditionalMinDueDate), (int)linearCard.x + 15, (int)linearCard.y + 162, 15.0f, currentRed);
                }

                // Performance Speedup Summary Bar
                if (benchHasRes) {
                    int bannerY = cardY + 218;
                    Rectangle speedupBanner = {(float)contentRect.x + 10, (float)bannerY, (float)contentRect.width - 20, 52};
                    DrawRectangleRounded(speedupBanner, 0.15f, 4, Fade(currentAccent, 0.18f));
                    DrawRectangleRoundedLinesEx(speedupBanner, 0.15f, 4, 1.2f, currentAccent);

                    DrawAppText(TextFormat("SPEEDUP ADVANTAGE: BST Search is %.0fx FASTER | Min-Heap Peek is %.0fx FASTER", benchRes.searchSpeedup, benchRes.heapSpeedup), (int)speedupBanner.x + 15, (int)speedupBanner.y + 16, 16.0f, currentAccent);
                    DrawAppText(TextFormat("Correctness Audit: %s", benchRes.correctnessPass ? "PASSED [PASS] (Identical Results)" : "FAILED [FAIL]"), (int)speedupBanner.x + (int)speedupBanner.width - 340, (int)speedupBanner.y + 16, 15.0f, benchRes.correctnessPass ? currentAccent : currentRed);
                }

                // Continuous Stress Test Results
                if (benchHasStress) {
                    int stressY = cardY + 280;
                    DrawAppText(TextFormat("STRESS LOOP SUMMARY (10 Iterations): Avg BST: %.3f us | Avg Heap Peek: %.3f us | Avg Speedup: %.0fx", benchStress.avgBstTimeUs, benchStress.avgMinHeapTimeUs, benchStress.avgSearchSpeedup), (int)contentRect.x + 15, stressY, 14.0f, ALERT_AMBER);
                }
            }
        }
        
        // RENDER CALENDAR PICKER OVERLAY MODAL
        DrawCalendarPickerModal(&calPicker, currentPanel, currentText, currentMuted, currentAccent, currentHeader, currentBorder, currentRed);
        
        // RENDER STATS POPUP WINDOW MODAL
        int modalRefDate = parseDateStringToInt(refDateInput.text);
        if (modalRefDate <= 0) modalRefDate = defaultToday;
        DrawStatsPopupWindowModal(&statsModal, ctrl, modalRefDate, screenW, screenH, isDarkMode, currentBg, currentPanel, currentText, currentMuted, currentAccent, currentAlert, currentRed, currentHeader, currentBorder);
        
        EndDrawing();
    }
    
    // 4. Clean Up & Exit
    if (benchDs) {
        freeBenchmarkDataset(benchDs);
        benchDs = NULL;
    }
    if (bench10MDs) {
        freeBenchmark10MDataset(bench10MDs);
        bench10MDs = NULL;
    }
    if (fontLoaded) {
        UnloadFont(mainFont);
    }
    freeController(ctrl);
    CloseWindow();
    
    return 0;
}
