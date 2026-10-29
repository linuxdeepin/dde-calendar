// SPDX-FileCopyrightText: 2019 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "test_method_interface.h"
#include <QVector>
#include <QDebug>
#include <QtGlobal>

test_method_interface::test_method_interface()
{

}

//QString GetLunarMonthName(int lunarmonthname, bool isleap)
TEST_F(test_method_interface, GetLunarMonthName)
{
    // 腊月
    int lunarmonthname = 12;
    bool isleap = false;
    QString lunarMName = GetLunarMonthName(lunarmonthname, isleap);
    assert("腊月" == lunarMName);

    // 闰腊月
    isleap = true;
    lunarMName = GetLunarMonthName(lunarmonthname, isleap);
    assert("闰腊月" == lunarMName);
}

//QString GetLunarDayName(int lundayname)
TEST_F(test_method_interface, GetLunarDayName)
{
    // "初八"
    int lundayname = 8;
    QString lunarDayName = GetLunarDayName(lundayname);
    //qCInfo(CommonLogger) << lunarDayName;
    assert("初八" == lunarDayName);

    // "廿五"
    lundayname = 25;
    lunarDayName = GetLunarDayName(lundayname);
    //qCInfo(CommonLogger) << lunarDayName;
    assert("廿五" == lunarDayName);
}

//QString GetLunarDayFestival(int monthname, int lunarday, int lunarmonthdays, int solarterm)
TEST_F(test_method_interface, GetLunarDayFestival)
{
    // "除夕"
    int monthname = 12;
    int lunarday = 30;
    int lunarmonthdays = 30;
    int solarterm = 1;
    QString dayFestival = GetLunarDayFestival(monthname, lunarday, lunarmonthdays, solarterm);
    //qCInfo(CommonLogger) << dayFestival;
    assert("除夕" == dayFestival);

    // "清明节"
    monthname = 4;
    lunarday = 2;
    lunarmonthdays = 23;
    solarterm = 1;
    dayFestival = GetLunarDayFestival(monthname, lunarday, lunarmonthdays, solarterm);
    //qCInfo(CommonLogger) << dayFestival;
    assert("清明节" == dayFestival);

    // ""
    monthname = 5;
    lunarday = 1;
    lunarmonthdays = 13;
    solarterm = 0;
    dayFestival = GetLunarDayFestival(monthname, lunarday, lunarmonthdays, solarterm);
    //qCInfo(CommonLogger) << dayFestival;
    assert("" == dayFestival);
}

//QString GetSolarTermName(int order)
TEST_F(test_method_interface, GetSolarTermName)
{
    // "小满"
    QString stName = GetSolarTermName(4);
    assert("小满" == stName);

    // ""
    stName = GetSolarTermName(24);
    assert("" == stName);
}

//QString GetGanZhiMonth(int year, int monthzhi)
TEST_F(test_method_interface, GetGanZhiMonth)
{
    QString ganzhi = GetGanZhiMonth(2020, 12);
    assert("戊子" == ganzhi);
}

//QString GetGanZhiYear(int lunaryear)
TEST_F(test_method_interface, GetGanZhiYear)
{
    // "庚子"
    QString ganzhi = GetGanZhiYear(2020);
    assert("庚子" == ganzhi);
}

//QString GetGanZhiDay(int year, int month, int day)
TEST_F(test_method_interface, GetGanZhiDay)
{
    // "己丑"
    QString ganzhi = GetGanZhiDay(2020, 12, 12);
    assert("己丑" == ganzhi);
}

//QString GetYearZodiac(int lunaryear)
TEST_F(test_method_interface, GetYearZodiac)
{
    // "鼠"
    QString Zodiac = GetYearZodiac(2020);
    assert("鼠" == Zodiac);

    // "猪"
    Zodiac = GetYearZodiac(2019);
    assert("猪" == Zodiac);
}

//QVector<double> get25SolarTermJDs(int year, int start)
TEST_F(test_method_interface, get25SolarTermJDs)
{
    QVector<double> jds = get25SolarTermJDs(0, 0);
}

//double GetSolarTermJD(int year, int order)
TEST_F(test_method_interface, GetSolarTermJD)
{
    //春分 3月20日
    assert(2.45893e+06 > GetSolarTermJD(2020, 0));
}

//bool IsLeapYear(int year)
TEST_F(test_method_interface, IsLeapYear)
{
    assert(true == IsLeapYear(2020) && false == IsLeapYear(2019));
}

// int GetSolarMonthDays(int year, int month)
TEST_F(test_method_interface, GetSolarMonthDays)
{
    assert(29 == GetSolarMonthDays(2020, 2));
}

//int GetWeekday(int y, int m, int d)
TEST_F(test_method_interface, GetWeekday)
{
    assert(6 == GetWeekday(2020, 2, 29));
}

//double DmsToDegrees(int degrees, int mintues, double seconds)
TEST_F(test_method_interface, DmsToDegrees)
{
    assert(qAbs(6.05 - DmsToDegrees(4, 3, 7200)) < 0.001);
}

//double DmsToSeconds(int d, int m, double s)
TEST_F(test_method_interface, DmsToSeconds)
{
    assert(qAbs(21780 - DmsToSeconds(4, 3, 7200)) < 0.001);
}

//double DmsToRadians(int d, int m, int s)
TEST_F(test_method_interface, DmsToRadians)
{
    assert(0.105592 <= DmsToRadians(4, 3, 7200));
}

//QDateTime GetDateTimeFromJulianDay(double jd)
TEST_F(test_method_interface, GetDateTimeFromJulianDay)
{
    // 春分(2020)约为 2020-03-20，使用 locale 无关的日期分量校验
    QDateTime dt = GetDateTimeFromJulianDay(GetSolarTermJD(2020, 0));
    assert(dt.date().year() == 2020);
    assert(dt.date().month() == 3);
    assert(dt.isValid());
}

//double GetDeltaT(int year, int month)
TEST_F(test_method_interface, GetDeltaT)
{
    const int count = 15;
    const int year[count] = {-501, 0, 1599, 1699, 1799,
                             1859, 1899, 1919, 1940, 1960,
                             1985, 2004, 2049, 2149, 2150
                            };
    for (int i = 0; i < count; ++i) {
        GetDeltaT(year[i], 6);
    }
}

//double JDBeijingTime2UTC(double bjtJD)
TEST_F(test_method_interface, JDBeijingTime2UTC)
{
    assert(0.67667 > JDBeijingTime2UTC(1.01));
}

//QString GetSolarDayFestival(int year, int month, int day)
TEST_F(test_method_interface, GetSolarDayFestival)
{
    QString getFesStr = GetSolarDayFestival(2020, 8, 1);
    assert("建军节" == getFesStr);

    getFesStr = GetSolarDayFestival(2020, 6, 1);
    assert("儿童节" == getFesStr);
}

//double CalcEarthObliquityNutation(double dt)
TEST_F(test_method_interface, CalcEarthObliquityNutation)
{
    CalcEarthObliquityNutation(GetJulianCentury(1));
}

//double lightAberration()
TEST_F(test_method_interface, lightAberration)
{
    lightAberration();
    //qCInfo(CommonLogger) << lightAb;
}

//QString GetTianGanDiZhi(int num)
TEST_F(test_method_interface, GetTianGanDiZhi)
{
    // num=0 -> TianGan[0]+DiZhi[0] = 甲子
    QString result = GetTianGanDiZhi(0);
    assert("甲子" == result);
    // num=1 -> 乙丑
    result = GetTianGanDiZhi(1);
    assert("乙丑" == result);
}

//void GetMoonEclipticParameter(MoonEclipticParameter &moonEclipticParameter, double T)
TEST_F(test_method_interface, GetMoonEclipticParameter)
{
    MoonEclipticParameter param;
    GetMoonEclipticParameter(param, 0.2);
    assert(qIsFinite(param.Lp));
    assert(qIsFinite(param.D));
    assert(qIsFinite(param.M));
    assert(qIsFinite(param.Mp));
    assert(qIsFinite(param.F));
    assert(qIsFinite(param.E));
    // E should be close to 1 for small T
    assert(param.E > 0.99 && param.E < 1.01);
}

//double ToRadians(double degrees)
TEST_F(test_method_interface, ToRadians)
{
    // 180 degrees = M_PI radians
    assert(qAbs(ToRadians(180) - M_PI) < 1e-9);
    // 0 degrees = 0 radians
    assert(qAbs(ToRadians(0)) < 1e-9);
    // 90 degrees = M_PI/2
    assert(qAbs(ToRadians(90) - M_PI / 2) < 1e-9);
}

//double Mod2Pi(double r)
TEST_F(test_method_interface, Mod2Pi)
{
    // value already in range
    assert(qAbs(Mod2Pi(M_PI) - M_PI) < 1e-9);
    // value over 2*PI should wrap
    assert(qAbs(Mod2Pi(2 * M_PI + 0.5) - 0.5) < 1e-9);
    // negative value should wrap to positive
    assert(qAbs(Mod2Pi(-0.5) - (2 * M_PI - 0.5)) < 1e-9);
    // zero stays zero
    assert(qAbs(Mod2Pi(0)) < 1e-9);
}

//double CalcMoonECLongitudePeriodic(MoonEclipticParameter &moonEclipticParameter)
TEST_F(test_method_interface, CalcMoonECLongitudePeriodic)
{
    MoonEclipticParameter param;
    GetMoonEclipticParameter(param, 0.2);
    double result = CalcMoonECLongitudePeriodic(param);
    assert(qIsFinite(result));
}

//double CalcMoonLongitudePerturbation(double T, MoonEclipticParameter &moonEclipticParameter)
TEST_F(test_method_interface, CalcMoonLongitudePerturbation)
{
    MoonEclipticParameter param;
    double T = 0.2;
    GetMoonEclipticParameter(param, T);
    double result = CalcMoonLongitudePerturbation(T, param);
    assert(qIsFinite(result));
}

//double GetMoonEclipticLongitudeEC(double julianDay)
TEST_F(test_method_interface, GetMoonEclipticLongitudeEC)
{
    double jd = 2458920.0; // 2020-03-20
    double result = GetMoonEclipticLongitudeEC(jd);
    assert(qIsFinite(result));
}

//double GetJulianCentury(double julianDay)
TEST_F(test_method_interface, GetJulianCentury)
{
    // J2000 should give T=0
    assert(qAbs(GetJulianCentury(J2000)) < 1e-9);
    // One century later (36525 days)
    assert(qAbs(GetJulianCentury(J2000 + 36525.0) - 1.0) < 1e-9);
}

//void GetEarthNutationParameter(EarthNutationParameter &earthNutationParameter, double T)
TEST_F(test_method_interface, GetEarthNutationParameter)
{
    EarthNutationParameter param;
    GetEarthNutationParameter(param, 0.2);
    assert(qIsFinite(param.D));
    assert(qIsFinite(param.M));
    assert(qIsFinite(param.Mp));
    assert(qIsFinite(param.F));
    assert(qIsFinite(param.Omega));
}

//double CalcEarthLongitudeNutation(double T)
TEST_F(test_method_interface, CalcEarthLongitudeNutation)
{
    double result = CalcEarthLongitudeNutation(0.2);
    assert(qIsFinite(result));
}

//double SecondsToDegrees(double seconds)
TEST_F(test_method_interface, SecondsToDegrees)
{
    // 3600 arcseconds = 1 degree
    assert(qAbs(SecondsToDegrees(3600) - 1.0) < 1e-9);
    // 0 arcseconds = 0 degrees
    assert(qAbs(SecondsToDegrees(0)) < 1e-9);
}

//double coefficient()
TEST_F(test_method_interface, coefficient)
{
    double result = coefficient();
    assert(qIsFinite(result));
    assert(result > 0); // should be a small positive value
}

//double SecondsToRadians(double seconds)
TEST_F(test_method_interface, SecondsToRadians)
{
    // 3600 arcseconds = 1 degree = M_PI/180 radians
    assert(qAbs(SecondsToRadians(3600) - M_PI / 180) < 1e-9);
    // 0 arcseconds = 0 radians
    assert(qAbs(SecondsToRadians(0)) < 1e-9);
}

//double ToJulianDateHMS(int year, int month, int day, int hour, int minute, double second)
TEST_F(test_method_interface, ToJulianDateHMS)
{
    // 2000-01-01 12:00:00 = J2000 = 2451545.0
    double jd = ToJulianDateHMS(2000, 1, 1, 12, 0, 0.0);
    assert(qAbs(jd - 2451545.0) < 1e-6);
    // 2020-03-20 12:00:00
    jd = ToJulianDateHMS(2020, 3, 20, 12, 0, 0.0);
    assert(jd > 2458900 && jd < 2459000);
}

//int ToJulianDate(int year, int month, int day)
TEST_F(test_method_interface, ToJulianDate)
{
    // 2000-01-01 noon = 2451545
    int jdn = ToJulianDate(2000, 1, 1);
    assert(jdn == 2451545);
    // 2020-03-20
    jdn = ToJulianDate(2020, 3, 20);
    assert(jdn > 2458900 && jdn < 2459000);
}

//double GetEarthEclipticLongitudeForSun(double jd)
TEST_F(test_method_interface, GetEarthEclipticLongitudeForSun)
{
    double jd = 2458920.0; // ~2020-03-20
    double result = GetEarthEclipticLongitudeForSun(jd);
    assert(qIsFinite(result));
    // Result should be in [0, 2*PI)
    assert(result >= 0 && result < 2 * M_PI);
}

//double GetSunEclipticLongitudeForEarth(double jd)
TEST_F(test_method_interface, GetSunEclipticLongitudeForEarth)
{
    double jd = 2458920.0;
    double result = GetSunEclipticLongitudeForEarth(jd);
    assert(qIsFinite(result));
    // Result should be in [0, 2*PI)
    assert(result >= 0 && result < 2 * M_PI);
}

//double GetJulianThousandYears(double jd)
TEST_F(test_method_interface, GetJulianThousandYears)
{
    // J2000 should give 0
    assert(qAbs(GetJulianThousandYears(J2000)) < 1e-9);
    // 365250 days = 1 thousand years
    assert(qAbs(GetJulianThousandYears(J2000 + 365250.0) - 1.0) < 1e-9);
}

//double GetSunEclipticLatitudeForEarth(double jd)
TEST_F(test_method_interface, GetSunEclipticLatitudeForEarth)
{
    double jd = 2458920.0;
    double result = GetSunEclipticLatitudeForEarth(jd);
    assert(qIsFinite(result));
}

//double Vsop2Fk5LongitudeCorrection(double l, double b, double jd)
TEST_F(test_method_interface, Vsop2Fk5LongitudeCorrection)
{
    double jd = 2458920.0;
    double result = Vsop2Fk5LongitudeCorrection(1.0, 0.01, jd);
    assert(qIsFinite(result));
}

//double GetSunRadiusForEarth(double jd)
TEST_F(test_method_interface, GetSunRadiusForEarth)
{
    double jd = 2458920.0;
    double result = GetSunRadiusForEarth(jd);
    assert(qIsFinite(result));
    // Sun-Earth distance should be around 1 AU
    assert(result > 0.98 && result < 1.02);
}

//double NewtonIteration(double angle, double x0, bool IsGetSolarTermJD)
TEST_F(test_method_interface, NewtonIteration_solarTerm)
{
    // Use same call as GetSolarTermJD: angle=0 (spring equinox), x0 near J2000+20 years
    double jd = NewtonIteration(0.0, 2458920.0, true);
    assert(qIsFinite(jd));
    assert(jd > 2458900 && jd < 2458940);
}

TEST_F(test_method_interface, NewtonIteration_newMoon)
{
    double jd = NewtonIteration(0.0, 2458920.0, false);
    assert(qIsFinite(jd));
}

//double ModPi(double r)
TEST_F(test_method_interface, ModPi)
{
    // value in range stays
    assert(qAbs(ModPi(1.0) - 1.0) < 1e-9);
    // value > PI wraps down
    assert(qAbs(ModPi(M_PI + 0.5) - (0.5 - M_PI)) < 1e-9);
    // value < -PI wraps up
    assert(qAbs(ModPi(-M_PI - 0.5) - (M_PI - 0.5)) < 1e-9);
    // zero stays zero
    assert(qAbs(ModPi(0)) < 1e-9);
}

//void GetDateFromJulianDay(double jd, int &yy, int &mm, int &dd)
TEST_F(test_method_interface, GetDateFromJulianDay)
{
    int year, month, day;
    GetDateFromJulianDay(2451545.0, year, month, day);
    assert(year == 2000);
    assert(month == 1);
    assert(day == 1);
    // 2020-03-20 noon JD
    GetDateFromJulianDay(2458929.0, year, month, day);
    assert(year == 2020);
    assert(month == 3);
    assert(day == 20);
}

//void GetTimeFromJulianDay(double jd, int &hour, int &minute, int &second)
TEST_F(test_method_interface, GetTimeFromJulianDay)
{
    int hour, minute, second;
    // JD 2451545.0 = 2000-01-01 12:00:00
    GetTimeFromJulianDay(2451545.0, hour, minute, second);
    assert(hour == 12);
    assert(minute == 0);
    assert(second == 0);
}

//double JDUTC2BeijingTime(double utcJD)
TEST_F(test_method_interface, JDUTC2BeijingTime)
{
    // UTC to Beijing = UTC + 8/24
    assert(qAbs(JDUTC2BeijingTime(1.0) - (1.0 + 8.0 / 24.0)) < 1e-9);
    assert(qAbs(JDUTC2BeijingTime(0.0) - 8.0 / 24.0) < 1e-9);
}

//double getNewMoonJD(double jd0)
TEST_F(test_method_interface, getNewMoonJD)
{
    double jd = getNewMoonJD(2458920.0);
    assert(qIsFinite(jd));
    assert(jd > 2458900 && jd < 2458960);
}

//QVector<double> get15NewMoonJDs(double jd)
TEST_F(test_method_interface, get15NewMoonJDs)
{
    QVector<double> list = get15NewMoonJDs(2458920.0);
    assert(list.size() == 15);
    for (int i = 0; i < list.size(); ++i) {
        assert(qIsFinite(list[i]));
    }
    // Each successive new moon should be ~29.53 days apart
    for (int i = 1; i < list.size(); ++i) {
        double diff = list[i] - list[i - 1];
        assert(diff > 29.0 && diff < 30.0);
    }
}

//qint64 deltaDays(QDateTime t1, QDateTime t2)
TEST_F(test_method_interface, deltaDays)
{
    QDateTime t1(QDate(2020, 1, 1), QTime(0, 0, 0));
    QDateTime t2(QDate(2020, 1, 11), QTime(0, 0, 0));
    assert(deltaDays(t1, t2) == 10);
    // t2 < t1 gives negative
    assert(deltaDays(t2, t1) == -10);
    // same day = 0
    assert(deltaDays(t1, t1) == 0);
}

//QString festivalForFatherAndMother(int year, int month, int day)
TEST_F(test_method_interface, festivalForFatherAndMother_mother)
{
    // Mother's day 2020 is May 10 (second Sunday of May)
    QString result = festivalForFatherAndMother(2020, 5, 10);
    assert("母亲节" == result);
    // non-matching day returns empty
    result = festivalForFatherAndMother(2020, 5, 1);
    assert("" == result);
}

TEST_F(test_method_interface, festivalForFatherAndMother_father)
{
    // Father's day 2020 is June 21 (third Sunday of June)
    QString result = festivalForFatherAndMother(2020, 6, 21);
    assert("父亲节" == result);
    // non-matching day returns empty
    result = festivalForFatherAndMother(2020, 6, 1);
    assert("" == result);
}

TEST_F(test_method_interface, festivalForFatherAndMother_otherMonth)
{
    // Other months return empty
    QString result = festivalForFatherAndMother(2020, 7, 1);
    assert("" == result);
}
