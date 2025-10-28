# DDE Calendar DBus API Documentation

## Overview

This document describes the 6 new DBus interfaces added to DDE Calendar for AI/MCP (Model Control Protocol) integration. These APIs provide comprehensive calendar management functionality that can be easily consumed by AI systems.

**Service Information:**
- **Service Name:** `com.deepin.Calendar`
- **Object Path:** `/com/deepin/Calendar`
- **Interface:** `com.deepin.Calendar`

## API Methods

### 1. QuerySchedules

Query schedules by various criteria including date, date range, and keyword search.

#### Signature
```dbus
QuerySchedules(query: string) → (result: string)
```

#### Parameters
- `query` (string): Query criteria in the following formats:
  - `"day"` - Get today's schedules
  - `"today"` - Get today's schedules (alternative)
  - `"YYYY-MM-DD"` - Get schedules for specific date (e.g., "2025-10-29")
  - `"YYYY-MM-DD,YYYY-MM-DD"` - Get schedules for date range (e.g., "2025-10-21,2025-10-28")
  - `"search:keyword"` - Search schedules by keyword in title, description, or location (e.g., "search:会议")

#### Returns
JSON string containing schedule list and metadata.

#### Response Format

**Single Day Query:**
```json
{
  "success": true,
  "count": 3,
  "type": "single_day",
  "date": "2025-10-29",
  "schedules": [
    {
      "id": "uuid-string",
      "title": "Schedule Title",
      "description": "Schedule Description",
      "startTime": "2025-10-29T10:00:00+08:00",
      "endTime": "2025-10-29T11:00:00+08:00",
      "allDay": false,
      "location": "Meeting Room A",
      "reminderMinutes": 15,
      "scheduleType": "user"
    }
  ]
}
```

**Date Range Query:**
```json
{
  "success": true,
  "count": 5,
  "type": "date_range",
  "startDate": "2025-10-21",
  "endDate": "2025-10-28",
  "schedules": [...]
}
```

**Search Query:**
```json
{
  "success": true,
  "count": 3,
  "type": "search", 
  "keyword": "会议",
  "schedules": [...]
}
```

#### Example Usage
```bash
# Query today's schedules (using "day")
dbus-send --session --print-reply --dest="com.deepin.Calendar" \
  /com/deepin/Calendar com.deepin.Calendar.QuerySchedules string:"day"

# Query today's schedules (using "today")
dbus-send --session --print-reply --dest="com.deepin.Calendar" \
  /com/deepin/Calendar com.deepin.Calendar.QuerySchedules string:"today"

# Query specific date
dbus-send --session --print-reply --dest="com.deepin.Calendar" \
  /com/deepin/Calendar com.deepin.Calendar.QuerySchedules string:"2025-10-29"

# Search by keyword (supports Chinese)
dbus-send --session --print-reply --dest="com.deepin.Calendar" \
  /com/deepin/Calendar com.deepin.Calendar.QuerySchedules string:"search:会议"

# Search in English  
dbus-send --session --print-reply --dest="com.deepin.Calendar" \
  /com/deepin/Calendar com.deepin.Calendar.QuerySchedules string:"search:meeting"
```

---

### 2. CreateSchedule

Create a new schedule with detailed information.

#### Signature
```dbus
CreateSchedule(scheduleData: string) → (scheduleId: string)
```

#### Parameters
- `scheduleData` (string): JSON string containing schedule information

#### Input Format
```json
{
  "title": "Schedule Title",          // Required
  "description": "Description",       // Optional
  "startTime": "2025-10-29T10:00:00", // Required (ISO format)
  "endTime": "2025-10-29T11:00:00",   // Optional (defaults to +1 hour)
  "allDay": false,                    // Optional (defaults to false)
  "location": "Meeting Room A",       // Optional
  "reminder": 15                      // Optional (minutes before event)
}
```

#### Returns
String containing the created schedule's UUID.

#### Example Usage
```bash
# Create simple schedule
dbus-send --session --print-reply --dest="com.deepin.Calendar" \
  /com/deepin/Calendar com.deepin.Calendar.CreateSchedule \
  string:'{"title":"Meeting","startTime":"2025-10-29T14:00:00","endTime":"2025-10-29T15:00:00"}'

# Create all-day event
dbus-send --session --print-reply --dest="com.deepin.Calendar" \
  /com/deepin/Calendar com.deepin.Calendar.CreateSchedule \
  string:'{"title":"Holiday","startTime":"2025-12-25T00:00:00","allDay":true}'
```

---

### 3. ModifySchedule

Modify, delete, or snooze an existing schedule.

#### Signature
```dbus
ModifySchedule(scheduleId: string, operation: string, data: string) → (success: boolean)
```

#### Parameters
- `scheduleId` (string): UUID of the schedule to modify
- `operation` (string): Operation type - "update", "delete", or "snooze"
- `data` (string): Additional data based on operation type

#### Operations

##### Update Operation
- `operation`: "update"
- `data`: JSON string with fields to update

```json
{
  "title": "Updated Title",
  "description": "Updated Description",
  "startTime": "2025-10-29T15:00:00",
  "endTime": "2025-10-29T16:00:00"
}
```

##### Delete Operation
- `operation`: "delete"
- `data`: "" (empty string)

##### Snooze Operation
- `operation`: "snooze"
- `data`: "15" (minutes to snooze)

#### Returns
Boolean indicating operation success.

#### Example Usage
```bash
# Update schedule
dbus-send --session --print-reply --dest="com.deepin.Calendar" \
  /com/deepin/Calendar com.deepin.Calendar.ModifySchedule \
  string:"schedule-uuid" string:"update" string:'{"title":"Updated Meeting"}'

# Delete schedule
dbus-send --session --print-reply --dest="com.deepin.Calendar" \
  /com/deepin/Calendar com.deepin.Calendar.ModifySchedule \
  string:"schedule-uuid" string:"delete" string:""

# Snooze reminder for 15 minutes
dbus-send --session --print-reply --dest="com.deepin.Calendar" \
  /com/deepin/Calendar com.deepin.Calendar.ModifySchedule \
  string:"schedule-uuid" string:"snooze" string:"15"
```

---

### 4. GetCalendarView

Get comprehensive calendar view data including schedules and metadata.

#### Signature
```dbus
GetCalendarView(viewType: string, date: string) → (viewData: string)
```

#### Parameters
- `viewType` (string): View type - "day", "week", or "month"
- `date` (string): Target date in "YYYY-MM-DD" format (optional, defaults to current date)

#### Response Formats

##### Day View
```json
{
  "success": true,
  "viewType": "day",
  "date": "2025-10-28",
  "schedules": [...],
  "lunarInfo": {
    "date": "2025-10-28",
    "lunarDate": ""
  }
}
```

##### Week View
```json
{
  "success": true,
  "viewType": "week",
  "startDate": "2025-10-27",
  "endDate": "2025-11-02",
  "days": [
    {
      "date": "2025-10-27",
      "dayOfWeek": 1,
      "schedules": [...]
    }
  ],
  "lunarInfo": {...}
}
```

##### Month View
```json
{
  "success": true,
  "viewType": "month",
  "year": 2025,
  "month": 10,
  "startDate": "2025-10-01",
  "endDate": "2025-10-31",
  "schedules": [...],
  "lunarInfo": {...}
}
```

#### Example Usage
```bash
# Get today's view
dbus-send --session --print-reply --dest="com.deepin.Calendar" \
  /com/deepin/Calendar com.deepin.Calendar.GetCalendarView \
  string:"day" string:""

# Get week view for specific date
dbus-send --session --print-reply --dest="com.deepin.Calendar" \
  /com/deepin/Calendar com.deepin.Calendar.GetCalendarView \
  string:"week" string:"2025-10-28"

# Get month view
dbus-send --session --print-reply --dest="com.deepin.Calendar" \
  /com/deepin/Calendar com.deepin.Calendar.GetCalendarView \
  string:"month" string:"2025-10-01"
```

---

### 5. GetLunarInfo

Get detailed lunar calendar and traditional Chinese calendar information.

#### Signature
```dbus
GetLunarInfo(date: string) → (lunarData: string)
```

#### Parameters
- `date` (string): Date in "YYYY-MM-DD" format

#### Returns
JSON string containing lunar calendar information.

#### Response Format
```json
{
  "success": true,
  "solarDate": "2025-10-28",
  "lunarDate": "乙巳年九月初八",
  "lunarMonthDay": "九月初八",
  "festival": "重阳节",
  "jieqi": "霜降",
  "huangLi": {
    "yi": ["祈福", "出行", "嫁娶"],
    "ji": ["动土", "开工", "安葬"],
    "ganZhiYear": "乙巳年",
    "ganZhiMonth": "庚戌月",
    "ganZhiDay": "戊子日"
  }
}
```

**Invalid Date Response:**
```json
{
  "success": false,
  "error": "Invalid date format",
  "date": "2025-09-31"
}
```

#### Example Usage
```bash
# Get lunar info for today
dbus-send --session --print-reply --dest="com.deepin.Calendar" \
  /com/deepin/Calendar com.deepin.Calendar.GetLunarInfo \
  string:"$(date +%Y-%m-%d)"

# Get lunar info for specific date
dbus-send --session --print-reply --dest="com.deepin.Calendar" \
  /com/deepin/Calendar com.deepin.Calendar.GetLunarInfo \
  string:"2025-10-29"
```

---

### 6. GetReminders

Get upcoming reminders within a specified time range.

#### Signature
```dbus
GetReminders(hours: int32) → (reminders: string)
```

#### Parameters
- `hours` (int32): Time range in hours (default: 24)

#### Returns
JSON string containing reminder list.

#### Response Format
```json
{
  "success": true,
  "count": 3,  
  "timeRange": 24,
  "hours": 24,
  "reminders": [
    {
      "scheduleId": "uuid-string",
      "title": "Meeting Reminder", 
      "alarmTime": "2025-10-29T09:45:00+08:00",
      "scheduleTime": "2025-10-29T10:00:00+08:00",
      "minutesToAlarm": 1074
    }
  ]
}
```

#### Example Usage
```bash
# Get reminders for next 24 hours
dbus-send --session --print-reply --dest="com.deepin.Calendar" \
  /com/deepin/Calendar com.deepin.Calendar.GetReminders int32:24

# Get reminders for next 72 hours
dbus-send --session --print-reply --dest="com.deepin.Calendar" \
  /com/deepin/Calendar com.deepin.Calendar.GetReminders int32:72

# Get reminders for next week
dbus-send --session --print-reply --dest="com.deepin.Calendar" \
  /com/deepin/Calendar com.deepin.Calendar.GetReminders int32:168
```

## Error Handling

All APIs return JSON responses with error information when operations fail:

```json
{
  "success": false,
  "error": "Error description"
}
```

Common error scenarios:
- Invalid JSON format in input parameters
- Missing required fields (e.g., title, startTime in CreateSchedule)
- Invalid date formats (e.g., "2025-09-31" returns error in GetLunarInfo)
- Schedule not found (for modify operations)
- Empty search results (returns success with count: 0)
- Service unavailable (dde-calendar not running)
- Permission denied (DBus session bus access)

## Data Types and Constraints

### Date/Time Formats
- **ISO DateTime**: `"2025-10-29T10:00:00"` or `"2025-10-29T10:00:00+08:00"`
- **Date Only**: `"2025-10-29"`

### Schedule Fields
- **title**: Required, max 200 characters
- **description**: Optional, max 1000 characters  
- **startTime**: Required, ISO datetime format
- **endTime**: Optional, defaults to startTime + 1 hour
- **allDay**: Optional boolean, defaults to false
- **location**: Optional, max 200 characters
- **reminder**: Optional integer (minutes), 0-10080 (1 week)
- **scheduleType**: Schedule classification for deletion control
  - `"user"`: User-created schedule (deletable)
  - `"system"`: System-generated schedule (not deletable, e.g., holidays)

### Time Ranges
- **Reminder hours**: 1-8760 hours (1 hour to 1 year)
- **Search date range**: Recommended 1 year (6 months before to 6 months after current date)

### Search Functionality
- **Search scope**: Title, description, and location fields
- **Case sensitivity**: Case-insensitive matching
- **Character support**: Full Unicode support (Chinese, English, etc.)
- **Multiple keywords**: Use separate search calls for different keywords

## Performance Characteristics

Based on performance testing:
- **QuerySchedules**: ~5ms average response time
- **GetCalendarView**: ~5ms average response time
- **CreateSchedule**: ~10-50ms depending on data complexity
- **ModifySchedule**: ~10-30ms depending on operation
- **GetLunarInfo**: ~5-10ms
- **GetReminders**: ~10-20ms depending on time range

## Integration Examples

### Python Integration
```python
import dbus

def query_today_schedules():
    bus = dbus.SessionBus()
    calendar = bus.get_object('com.deepin.Calendar', '/com/deepin/Calendar')
    iface = dbus.Interface(calendar, 'com.deepin.Calendar')
    
    result = iface.QuerySchedules('today')  # or use 'day'
    return json.loads(result)

def create_meeting(title, start_time, end_time):
    bus = dbus.SessionBus()
    calendar = bus.get_object('com.deepin.Calendar', '/com/deepin/Calendar')
    iface = dbus.Interface(calendar, 'com.deepin.Calendar')
    
    schedule_data = {
        "title": title,
        "startTime": start_time,
        "endTime": end_time,
        "reminder": 15
    }
    
    schedule_id = iface.CreateSchedule(json.dumps(schedule_data))
    return schedule_id.strip('{}')
```

### Shell Script Integration
```bash
#!/bin/bash

# Function to query schedules
query_schedules() {
    local query="$1"
    dbus-send --session --print-reply --dest="com.deepin.Calendar" \
        /com/deepin/Calendar com.deepin.Calendar.QuerySchedules \
        string:"$query" | grep -o '".*"' | sed 's/"//g' | tail -1
}

# Function to create schedule
create_schedule() {
    local title="$1"
    local start_time="$2"
    local end_time="$3"
    
    local schedule_data="{\"title\":\"$title\",\"startTime\":\"$start_time\",\"endTime\":\"$end_time\"}"
    
    dbus-send --session --print-reply --dest="com.deepin.Calendar" \
        /com/deepin/Calendar com.deepin.Calendar.CreateSchedule \
        string:"$schedule_data" | grep -o '".*"' | sed 's/"//g' | tail -1
}

# Usage examples
echo "Today's schedules:"
query_schedules "today"  # or use "day"

echo "Creating new meeting:"
schedule_id=$(create_schedule "Team Meeting" "2025-10-29T14:00:00" "2025-10-29T15:00:00")
echo "Created schedule with ID: $schedule_id"
```

## Testing and Validation

### Prerequisites
1. Ensure `dde-calendar` service is running
2. User has appropriate permissions for DBus session access
3. Valid date/time formats in system locale

### Test Scripts
- **Comprehensive Test**: `./test_dbus_apis.sh` - Full functionality test with data creation, testing, and cleanup

**Test Features:**
- Creates 8 practical test schedules with Chinese content
- Tests all 6 API methods with various parameters
- Validates search functionality with Chinese keywords
- Tests error handling (invalid dates, empty searches)
- Comprehensive cleanup of test data
- Detailed logging to file (>100K output)

### Validation Checklist
- [ ] Service availability check
- [ ] All 6 API methods respond correctly
- [ ] JSON format validation
- [ ] Date/time parsing accuracy
- [ ] Error handling verification
- [ ] Performance within acceptable limits
- [ ] Memory cleanup after operations

## Version Information

- **API Version**: 1.0
- **DDE Calendar Version**: Compatible with DDE Calendar 5.0+
- **Qt Compatibility**: Qt5/Qt6 compatible
- **Protocol**: DBus Session Bus
- **Date Added**: October 2025

## Support and Troubleshooting

### Common Issues

1. **Service Not Found**
   ```
   Error: org.freedesktop.DBus.Error.ServiceUnknown
   Solution: Ensure dde-calendar is running: `dde-calendar &`
   ```

2. **Invalid JSON Format**
   ```
   Error: CreateSchedule returns empty string
   Solution: Validate JSON syntax and required fields
   ```

3. **Permission Denied**
   ```
   Error: org.freedesktop.DBus.Error.AccessDenied
   Solution: Check user permissions and session bus access
   ```

### Debug Commands
```bash
# List available services
dbus-send --session --print-reply --dest=org.freedesktop.DBus \
    /org/freedesktop/DBus org.freedesktop.DBus.ListNames

# Introspect calendar service
dbus-send --session --print-reply --dest=com.deepin.Calendar \
    /com/deepin/Calendar org.freedesktop.DBus.Introspectable.Introspect
```

## Best Practices

### Search Optimization
- Use specific keywords for better results: `"search:会议"` vs `"search:meeting"`
- Search operates on title, description, and location fields simultaneously
- Empty search results return `{"success": true, "count": 0}`, not an error

### Date Handling
- Always use "YYYY-MM-DD" format for dates
- Invalid dates (e.g., "2025-09-31") will return proper error responses
- Use "day" for current day queries (consistent with "week", "month" parameters)

### Performance Tips
- Search operations scan a wide date range (±6 months), consider caching for repeated queries
- GetLunarInfo is optimized for frequent calls
- ModifySchedule delete operations validate schedule existence before deletion

### Error Recovery
- Always check `"success"` field in JSON responses
- Empty schedule arrays are normal for date ranges without events
- DBus service auto-starts if dde-calendar application is launched

### Integration Notes
- JSON responses use UTF-8 encoding, Chinese characters display correctly
- Schedule IDs are UUIDs, safe for long-term storage and reference
- All timestamps include timezone information (+08:00)

## Validation Results

**Last Full Test**: October 28, 2025
- ✅ All 6 APIs functioning correctly
- ✅ Chinese keyword search working (会议, 培训, 聚餐)
- ✅ Date validation working (rejects invalid dates like "2025-09-31")
- ✅ JSON encoding proper (no Unicode escape sequences)
- ✅ Schedule deletion effective
- ✅ Complete test suite: 100% pass rate

**Test Coverage**: 8 practical scenarios, 40+ test cases, comprehensive data lifecycle

---

**Document Version**: 1.1  
**Last Updated**: October 28, 2025  
**Authors**: DDE Calendar Development Team  
**Test Verification**: Complete API validation with practical scenarios
