/*
 * Copyright (C) 2017 ~ 2018 Deepin Technology Co., Ltd.
 *
 * Author:     kirigaya <kirigaya@mkacg.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef CONSTANTS_H
#define CONSTANTS_H

namespace DDECalendar {
static const int HeaderItemWidth = 80;
static const int HeaderItemHeight = 40;

static const int CellWidth = 106;
static const int CellHeight = 70;

static const int CellHighlightWidth = 80;
static const int CellHighlightHeight = 70;

static const int QueryEarliestYear = 1900;
static const int QueryLatestYear = 9999;

static const int FestivalTypeID = 4;

static const int FontSizeEleven = 11;
static const int FontSizeTwelve = 12;
static const int FontSizeFourteen = 14;
static const int FontSizeSixteen = 16;
static const int FontSizeTwentyfour = 24;
static const int FontSizeTwenty = 20;
static const int FontSizeTen = 10;
static const int FontSizeOneHundred = 100;

static const int CalendarYearWindow = 0;
static const int CalendarMonthWindow = 1;
static const int CalendarWeekWindow = 2;
static const int CalendarDayWindow = 3;

static const int ThirtyMinutesWithSec = 1800;

static const int AtTimeOfEvent = 0;
static const int OnStartDay = 0;
static const int FifteenMinutesBefore = 15;
static const int ThirtyMinutesBefore = 30;
static const int OneHourBefore = 60;
static const int OneDayBeforeWithDay = 1;
static const int OneDayBeforeWithMinutes = 1440;
static const int TwoDayBeforeWithDay = 2;
static const int TwoDayBeforeWithMinutes = 2880;
static const int OneWeekBeforeWithDay = 7;
static const int OneWeekBeforeWithMinutes = 10080;

static const int NewSchceduleLabelWidth = 70;

static const int QuarterOfAnhourWithMinute = 15;

} // namespace DDECalendar
namespace DDEYearCalendar {

static const int RectSizeofEveyMonth = 42;
static const int FrameSizeofEveryYear = 12;

static const int YearScheduleListMaxcount = 5;
static const int YHeaderItemWidth = 20;
static const int YHeaderItemHeight = 16;

static const int YCellWidth = 26;
static const int YCellHeight = 20;

static const int YCellHighlightWidth = 26;
static const int YCellHighlightHeight = 20;
static const int Y_MLableHeight = 36;
static const int Y_YLableHeight = 45;

static const int Y_MWindowWidth = 202;
static const int Y_MWindowHeight = 159;
} // namespace DDEYearCalendar
namespace DDEMonthCalendar {
static const int ItemSizeofMonthDay = 42;

static const int AFewDaysofWeek = 7;
static const int LinesNumofMonth = 6;

static const int MonthNumofYear = 12;

static const int MHeaderItemWidth = 60;
static const int MHeaderItemHeight = 33;
static const int MEventsItemHeight = 36;

static const int MCellWidth = 120;
static const int MCellHeight = 74;

static const int MCellHighlightWidth = 120;
static const int MCellHighlightHeight = 74;

static const int MWeekCellWidth = 120;
static const int MWeekCellHeight = 54;

static const int MDayCellWidth = 36;
static const int MDayCellHeight = 36;

static const int M_YTopHeight = 76;
static const int M_YLableHeight = 36;
static const int M_YLunatLabelWindth = 66;
static const int M_YLunatLabelHeight = 20;

static const int MTodayWindth = 100;
static const int MTodayHeight = 36;

static const int M_MWindowWidth = 202;
static const int M_MWindowHeight = 162;
}
namespace DDEWeekCalendar {
static const int FirstDayofWeekend = 6;
static const int AFewDaysofWeek = 7;

static const int NumWeeksDisplayed = 10;

static const int WCellHeaderItemWidth = 109;
static const int WCellHeaderItemHeight = 54;

static const int WMCellHeadrWidth = 80;

static const int MCellHighlightWidth = 120;
static const int MCellHighlightHeight = 74;

static const int WWeekCellWidth = 36;
static const int WWeekCellHeight = 36;

static const int W_YLableHeight = 36;
static const int W_YLunatLabelWindth = 66;
static const int W_YLunatLabelHeight = 20;

static const int WTodayWindth = 100;
static const int WTodayHeight = 36;

static const int W_MWindowWidth = 202;
static const int W_MWindowHeight = 162;
}
namespace DDEDayCalendar {
static const int PainterCellNum = 42;

static const int DCellWidth = 35;
static const int DCellHeight = 28;

static const int DCellHighlightWidth = 33;
static const int DCellHighlightHeight = 26;
static const int D_MLableHeight = 36;
static const int D_YLableHeight = 36;
static const int DDLableHeight = 117;
static const int DWLableHeight = 22;
static const int DHualiInfoLableHeight = 17;
static const int DHualiDtailLableHeight = 56;
static const int DHualiDtailLableWidth = 330;
static const int D_MWindowWidth = 350;
static const int D_MWindowHeight = 564;
}
enum Week {
    Monday    = 6,
    Tuesday   = 5,
    Wednesday = 4,
    Thursday  = 3,
    Friday    = 2,
    Saturday  = 1,
    Sunday    = 0,
};

#endif // CONSTANTS_H
