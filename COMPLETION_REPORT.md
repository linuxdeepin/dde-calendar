<!--
  SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
  SPDX-License-Identifier: CC-BY-4.0
-->

# AT-SPI Completion Report - dde-calendar

## Files Modified

| # | File | Change Type | Details |
|---|------|-------------|---------|
| 1 | `src/calendar-client/src/main.cpp` | **Critical Fix** | Added `QAccessible::installFactory(accessibleFactory)` to activate the accessibility framework |
| 2 | `src/calendar-client/src/widget/dayWidget/daywindow.cpp` | Enhancement | Added `setAccessibleName`/`setObjectName` for 5 widgets |
| 3 | `src/calendar-client/src/widget/monthWidget/monthwindow.cpp` | Enhancement | Added `setAccessibleName`/`setObjectName` for 4 widgets |
| 4 | `src/calendar-client/src/widget/weekWidget/weekwindow.cpp` | Enhancement | Added `setAccessibleName`/`setObjectName` for 6 widgets |
| 5 | `src/calendar-client/src/widget/cschedulebasewidget.cpp` | Enhancement | Added `setAccessibleName`/`setObjectName` for CDialogIconButton |
| 6 | `src/calendar-client/src/dialog/timejumpdialog.cpp` | Enhancement | Added `setAccessibleName`/`setObjectName` for 4 widgets |
| 7 | `src/calendar-client/src/widget/sidebarWidget/sidebarview.cpp` | Enhancement | Added `setAccessibleName`/`setObjectName` for QTreeWidget |
| 8 | `src/calendar-client/src/dialog/settingdialog.cpp` | Enhancement | Added `setAccessibleName` for CSettingDialog |

## Description of Changes

### 1. Critical Fix: Install Accessible Factory (`main.cpp`)

The most impactful change. The project already had a comprehensive `accessibleFactory` function in `accessible.h` defining AT-SPI wrappers for 20+ widget types, but it was never registered with Qt. Adding `QAccessible::installFactory(accessibleFactory)` activates the entire framework.

### 2-5. View Window Enhancements

Added accessible names to the key UI elements of the four calendar view windows:
- **YearWindow**: Already had `PrevButton`, `NextButton`, `yearToDay`, `CYearWindow` (via factory)
- **MonthWindow**: Added `MonthTodayButton`, `MonthYearLabel`, `MonthLunarLabel`, `MonthTopWidget`
- **WeekWindow**: Added `WeekTodayButton`, `WeekYearLabel`, `WeekLunarLabel`, `WeekLabel`, `WeekHeadView`, `WeekScheduleView`
- **DayWindow**: Added `DayYearLabel`, `DayLunarLabel`, `DaySolarDayLabel`, `DayScheduleView`, `DayMonthView`
- **Base class**: Added `DateJumpButton` for the date picker icon button shared across all views

### 6. Time Jump Dialog

Added accessible names to year/month/day input fields and the "Go" button, making the date navigation dialog fully accessible.

### 7. Sidebar

Added accessible name to the account/schedule type tree widget.

### 8. Settings Dialog

Added `setAccessibleName("SettingDialog")` to match the existing `setObjectName("SettingDialog")`.