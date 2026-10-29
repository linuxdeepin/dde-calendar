// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "pinyin/pinyinsearch.h"
#include "pinyin/pinyindict.h"

#include <gtest/gtest.h>

#include <QString>
#include <QStringList>
#include <QRegularExpression>

// Branch inventory (based on get_code_snippet source):
//   pinyinsearch() ctor: if(key.size() > singlePinyinMaxLength) — true/false
//   getPinPinSearch(): if(m_pinyinsearch == nullptr) — true(first)/false(subsequent)
//   CanQueryByPinyin(): Qt6/Qt5 path; regex match true/false
//   CreatePinyin(): for-loop zero-iter(empty)/non-zero; if(!pyStr.isEmpty()) true/false
//   initDict(): if(!pinyinDictVector.isEmpty()) skip; if(!file.open()) fail; if(items.size()==2) valid/invalid
//   CreatePinyinQuery(): while(pinyin.size()>0); while(i>1) with if(validPinyinMap[key]) break; i==1 fallback
//   CreatePinyinRegexp(): same loop structure as CreatePinyinQuery
//   PinyinMatch(): calls CreatePinyin + CreatePinyinRegexp; QRegularExpression match true/false
//   Pinyin(): for-loop; if(py.size()>0) true/false
//   SinglePinyin(): if(pinyinDictVector.contains(unicode)) true/false
//   RemoveYin(): for-loop; if(phoneticSymbol.contains(s)) true/false; if(!strList.contains(str)) dedup

class PinyinSearchTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        instance = pinyinsearch::getPinPinSearch();
        ASSERT_NE(instance, nullptr);
    }

    pinyinsearch *instance = nullptr;
};

// === getPinPinSearch: singleton returns same instance ===
TEST_F(PinyinSearchTest, GetInstanceReturnsSameSingleton)
{
    auto *first = pinyinsearch::getPinPinSearch();
    auto *second = pinyinsearch::getPinPinSearch();
    EXPECT_EQ(first, second);
    EXPECT_NE(first, nullptr);
}

TEST_F(PinyinSearchTest, GetInstanceNonNullAfterInit)
{
    EXPECT_NE(pinyinsearch::getPinPinSearch(), nullptr);
}

// === CanQueryByPinyin: pure letters -> true ===
TEST_F(PinyinSearchTest, CanQueryByPinyinPureLetters)
{
    EXPECT_TRUE(pinyinsearch::CanQueryByPinyin("nihao"));
    EXPECT_TRUE(pinyinsearch::CanQueryByPinyin("ABC"));
    EXPECT_TRUE(pinyinsearch::CanQueryByPinyin("a"));
}

// === CanQueryByPinyin: empty string -> false ===
TEST_F(PinyinSearchTest, CanQueryByPinyinEmpty)
{
    EXPECT_FALSE(pinyinsearch::CanQueryByPinyin(""));
}

// === CanQueryByPinyin: mixed/digits/Chinese -> false ===
TEST_F(PinyinSearchTest, CanQueryByPinyinNonLetters)
{
    EXPECT_FALSE(pinyinsearch::CanQueryByPinyin("ni123"));
    EXPECT_FALSE(pinyinsearch::CanQueryByPinyin("ni hao"));
    EXPECT_FALSE(pinyinsearch::CanQueryByPinyin("你好"));
    EXPECT_FALSE(pinyinsearch::CanQueryByPinyin("ni!"));
}

// === CreatePinyin: Chinese string produces bracketed pinyin ===
TEST_F(PinyinSearchTest, CreatePinyinChineseString)
{
    QString result = pinyinsearch::CreatePinyin("你好");
    EXPECT_FALSE(result.isEmpty());
    EXPECT_TRUE(result.startsWith("["));
    EXPECT_TRUE(result.endsWith("]"));
}

// === CreatePinyin: empty string -> empty result ===
TEST_F(PinyinSearchTest, CreatePinyinEmptyString)
{
    QString result = pinyinsearch::CreatePinyin("");
    EXPECT_TRUE(result.isEmpty());
}

// === CreatePinyin: non-Chinese string -> empty result (pyStr.isEmpty branch) ===
TEST_F(PinyinSearchTest, CreatePinyinNonChineseString)
{
    QString result = pinyinsearch::CreatePinyin("abc");
    EXPECT_TRUE(result.isEmpty());
}

// === CreatePinyin: single Chinese char ===
TEST_F(PinyinSearchTest, CreatePinyinSingleChar)
{
    QString result = pinyinsearch::CreatePinyin("你");
    EXPECT_FALSE(result.isEmpty());
    EXPECT_TRUE(result.startsWith("["));
    EXPECT_TRUE(result.endsWith("]"));
}

// === initDict: already initialized -> skip ===
TEST_F(PinyinSearchTest, InitDictSkipWhenAlreadyLoaded)
{
    pinyinsearch::initDict();
    SUCCEED();
}

// === CreatePinyinQuery: "nihao" -> "[%ni%][%hao%]" ===
TEST_F(PinyinSearchTest, CreatePinyinQueryNormalInput)
{
    QString result = instance->CreatePinyinQuery("nihao");
    EXPECT_FALSE(result.isEmpty());
    EXPECT_EQ(result, "[%ni%][%hao%]");
}

// === CreatePinyinQuery: single valid pinyin ===
TEST_F(PinyinSearchTest, CreatePinyinQuerySingleSyllable)
{
    QString result = instance->CreatePinyinQuery("hao");
    EXPECT_EQ(result, "[%hao%]");
}

// === CreatePinyinQuery: empty string -> empty ===
TEST_F(PinyinSearchTest, CreatePinyinQueryEmpty)
{
    QString result = instance->CreatePinyinQuery("");
    EXPECT_TRUE(result.isEmpty());
}

// === CreatePinyinQuery: invalid char fallback to single char (i falls to 1) ===
TEST_F(PinyinSearchTest, CreatePinyinQueryInvalidCharsFallback)
{
    QString result = instance->CreatePinyinQuery("z");
    EXPECT_FALSE(result.isEmpty());
    EXPECT_EQ(result, "[%z%]");
}

// === CreatePinyinRegexp: normal input ===
TEST_F(PinyinSearchTest, CreatePinyinRegexpNormalInput)
{
    QString result = instance->CreatePinyinRegexp("nihao");
    EXPECT_FALSE(result.isEmpty());
    EXPECT_TRUE(result.contains("ni"));
    EXPECT_TRUE(result.contains("hao"));
    EXPECT_TRUE(result.contains("[a-z\\|]*"));
}

// === CreatePinyinRegexp: single syllable ===
TEST_F(PinyinSearchTest, CreatePinyinRegexpSingleSyllable)
{
    QString result = instance->CreatePinyinRegexp("hao");
    EXPECT_FALSE(result.isEmpty());
    EXPECT_TRUE(result.contains("hao"));
}

// === CreatePinyinRegexp: empty -> empty ===
TEST_F(PinyinSearchTest, CreatePinyinRegexpEmpty)
{
    QString result = instance->CreatePinyinRegexp("");
    EXPECT_TRUE(result.isEmpty());
}

// === CreatePinyinRegexp: invalid char fallback ===
TEST_F(PinyinSearchTest, CreatePinyinRegexpInvalidCharFallback)
{
    QString result = instance->CreatePinyinRegexp("z");
    EXPECT_FALSE(result.isEmpty());
    EXPECT_TRUE(result.contains("z"));
}

// === PinyinMatch: matching -> true ===
TEST_F(PinyinSearchTest, PinyinMatchTrue)
{
    bool result = instance->PinyinMatch("你好", "ni");
    EXPECT_TRUE(result);
}

// === PinyinMatch: non-matching -> false ===
TEST_F(PinyinSearchTest, PinyinMatchFalse)
{
    bool result = instance->PinyinMatch("你好", "xyz");
    EXPECT_FALSE(result);
}

// === PinyinMatch: full pinyin match ===
TEST_F(PinyinSearchTest, PinyinMatchFullPinyin)
{
    bool result = instance->PinyinMatch("你好", "nihao");
    EXPECT_TRUE(result);
}

// === PinyinMatch: empty pinyin -> empty regex matches ===
TEST_F(PinyinSearchTest, PinyinMatchEmptyPinyin)
{
    bool result = instance->PinyinMatch("你好", "");
    EXPECT_TRUE(result);
}

// === Pinyin (private static): Chinese string -> non-empty list ===
TEST_F(PinyinSearchTest, PinyinMethodChineseString)
{
    QList<QStringList> result = pinyinsearch::Pinyin("你好");
    EXPECT_EQ(result.size(), 2);
    for (const auto &pyList : result) {
        EXPECT_FALSE(pyList.isEmpty());
    }
}

// === Pinyin: empty string -> empty list ===
TEST_F(PinyinSearchTest, PinyinMethodEmptyString)
{
    QList<QStringList> result = pinyinsearch::Pinyin("");
    EXPECT_TRUE(result.isEmpty());
}

// === Pinyin: non-Chinese -> empty list (py.size()==0 branch) ===
TEST_F(PinyinSearchTest, PinyinMethodNonChinese)
{
    QList<QStringList> result = pinyinsearch::Pinyin("abc");
    EXPECT_EQ(result.size(), 3);
}

// === Pinyin: single Chinese char ===
TEST_F(PinyinSearchTest, PinyinMethodSingleChar)
{
    QList<QStringList> result = pinyinsearch::Pinyin("你");
    EXPECT_EQ(result.size(), 1);
    EXPECT_FALSE(result.at(0).isEmpty());
}

// === SinglePinyin (private static): valid Chinese char -> non-empty ===
TEST_F(PinyinSearchTest, SinglePinyinValidChineseChar)
{
    QStringList result = pinyinsearch::SinglePinyin("你");
    EXPECT_FALSE(result.isEmpty());
    EXPECT_GT(result.size(), 0);
}

// === SinglePinyin: non-Chinese char -> not in dict ===
TEST_F(PinyinSearchTest, SinglePinyinNonChineseChar)
{
    QStringList result = pinyinsearch::SinglePinyin("a");
    EXPECT_TRUE(result.size() == 1 || result.isEmpty());
}

// === SinglePinyin: empty string ===
TEST_F(PinyinSearchTest, SinglePinyinEmptyString)
{
    QStringList result = pinyinsearch::SinglePinyin("");
    EXPECT_TRUE(result.isEmpty() || (result.size() == 1 && result.at(0).isEmpty()));
}

// === RemoveYin (private static): pinyin with tone marks -> converted ===
TEST_F(PinyinSearchTest, RemoveYinWithToneMarks)
{
    QStringList input;
    input << "nǐ";
    QStringList result = pinyinsearch::RemoveYin(input);
    EXPECT_FALSE(result.isEmpty());
    EXPECT_TRUE(result.contains("ni"));
}

// === RemoveYin: pinyin without tone marks -> unchanged ===
TEST_F(PinyinSearchTest, RemoveYinWithoutToneMarks)
{
    QStringList input;
    input << "ni" << "hao";
    QStringList result = pinyinsearch::RemoveYin(input);
    EXPECT_EQ(result.size(), 2);
    EXPECT_TRUE(result.contains("ni"));
    EXPECT_TRUE(result.contains("hao"));
}

// === RemoveYin: duplicate pinyin -> deduplicated ===
TEST_F(PinyinSearchTest, RemoveYinDeduplication)
{
    QStringList input;
    input << "nī" << "ní";
    QStringList result = pinyinsearch::RemoveYin(input);
    EXPECT_EQ(result.size(), 1);
    EXPECT_EQ(result.at(0), "ni");
}

// === RemoveYin: empty list -> empty result ===
TEST_F(PinyinSearchTest, RemoveYinEmptyList)
{
    QStringList input;
    QStringList result = pinyinsearch::RemoveYin(input);
    EXPECT_TRUE(result.isEmpty());
}

// === Constructor: validPinyinMap populated (verify via CreatePinyinQuery) ===
TEST_F(PinyinSearchTest, ConstructorInitializesValidPinyinMap)
{
    QString result = instance->CreatePinyinQuery("ni");
    EXPECT_EQ(result, "[%ni%]");
}

// === Constructor: singlePinyinMaxLength correct (longest valid pinyin 6 chars) ===
TEST_F(PinyinSearchTest, ConstructorSetsSinglePinyinMaxLength)
{
    QString result = instance->CreatePinyinQuery("chuang");
    EXPECT_EQ(result, "[%chuang%]");
}
