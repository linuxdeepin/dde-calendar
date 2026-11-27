# Work Order — DSchedule recurrence/exception/alarm/pinyin/ICS UT supplement

## Target
- Class: DSchedule (src/calendar-common/src/dschedule.cpp / .h)
- Test target: tests/dde-calendar-service-test/src/test_dschedule_recurrence.cpp
- CMake: APPEND to tests/dde-calendar-service-test/CMakeLists.txt (CALDAV_TEST_SRC list)

## Methods to cover (new, uncovered by existing test_dschedule.cpp)

### 1. Recurrence exception management
- [x] setRecurrenceException
- [x] updateRecurrenceException
- [x] clearRecurrenceExceptions
- [x] hasStoredRecurrenceExceptions
- [x] setRecurrenceExceptionsStored
- [x] pruneRecurrenceExceptionsAfter
- [x] removeRecurrenceExceptionsFrom
- [x] inheritRecurrenceExceptions
- [x] hasRecurrenceException
- [x] recurrenceExceptions
- [x] recurrenceExceptionsInInterval

### 2. Occurrence/range handling (public API)
- [x] recurrenceException (single)
- [x] hasRecurrenceOverride
- [x] hasRecurrenceRangeAt
- [x] hasRecurrenceExceptions (inline)

### 3. ICS parsing new path
- [x] fromStoredIcsString
- [x] parseIcsString (existingData branch — via fromStoredIcsString)

### 4. Pinyin search
- [x] titleMatches

### 5. Alarm detection
- [x] hasEnabledAlarms (DSchedule-level, checking exceptions)
