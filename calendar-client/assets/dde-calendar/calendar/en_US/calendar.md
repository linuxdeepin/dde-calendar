# Calendar | dde-calendar

## Overview

Calendar is a small tool for viewing dates and managing schedules. It supports viewing lunar calendars, Chinese almanacs, solar terms, and common festival information, with built-in schedule reminders to help you better plan your time.

![0|main](fig/main.png)

## Getting Started

Run or close Calendar, or create shortcuts using the following methods.

### Running Calendar

1. Click the Launcher icon ![deepin_launcher](../common/deepin_launcher.svg) on the taskbar to enter the Launcher interface.
2. Scroll the mouse wheel or search to locate the Calendar icon ![draw](../common/dde_calendar.svg), then click to run.
3. Right-click ![draw](../common/dde_calendar.svg) to:
   - Click **Send to Desktop** to create a desktop shortcut.
   - Click **Pin to Taskbar** to pin the application to the taskbar.
   - Click **Start on Boot** to add the application to startup items, automatically running when the computer starts.

### Closing Calendar

- Click ![close_icon](../common/close_icon.svg) in the Calendar interface to exit.
- Right-click ![draw](../common/dde_calendar.svg) on the taskbar and select **Close All** to exit Calendar.
- Click ![icon_menu](../common/icon_menu.svg) in the Calendar interface and select **Exit** to exit Calendar.

### Viewing Shortcuts

In the Calendar interface, use the shortcut **Ctrl + Shift + ?** to open the shortcut preview. Mastering shortcuts will greatly improve your efficiency.

![0|view](fig/hotkey.png)

## Operations Guide

Calendar displays date attributes in Year, Month, Week, and Day views.

The system defaults to Month view. Switch between views by clicking the corresponding buttons.

- Only in Chinese systems: Calendar displays lunar dates, Chinese almanacs, and festival information.
- Dates start from **1900**; dates earlier than **1900** cannot be viewed.
- In Month and Week views, Saturdays and Sundays are visually distinguished from weekdays.

The sidebar shows calendar accounts and a mini-calendar view.

1. Click ![side_menu](../common/side_menu.svg) to toggle the sidebar visibility.
2. Check calendar types under Calendar Accounts to display their schedules; uncheck to hide.
3. Click dates in the mini-calendar to navigate the main calendar view.

<table border="1">
<tr>
   <th width="80px">View</th>
   <th width="200px">Description</th>
</tr>
<tr>
   <td>Year</td>
   <td>Displays all months and days of the year.</td>
</tr>
<tr>
   <td>Month</td>
    <td>Shows festival information and schedules.</td>
</tr>
<tr>
   <td>Week</td>
   <td>Displays daily schedules for the week.</td>
</tr>
<tr>
   <td>Day</td>
   <td>Shows festival info, detailed schedules, and Chinese almanac.</td>
</tr> 
</table>

### Creating Schedules

1. Create a schedule using one of these methods:
   - Click the add button <img src="../common/add.svg" alt="plus" style="zoom:50%;" /> in the menu bar.
   - Double-click a blank date area or right-click and select **New Schedule** in Month/Week/Day view.
   - Click and drag to create a schedule in Month/Week/Day view.
   
2. The "New Schedule" window appears. Configure calendar account, schedule type, content, time, reminders, etc.

<img src="fig/create.png" alt="pic" style="zoom:67%;" />

<table border="1">
<tr>
   <th width="50px">Parameter</th>
   <th width="200px">Description</th>
</tr>
<tr>
   <td>Calendar Account</td>
   <td>Defaults to local account; supports UOS ID and other network accounts.</td>
</tr>
<tr>
   <td>Type</td>
   <td>Default types: "Work", "Life", "Other". Custom types can be added:
       <ol><li>Select <b>New Schedule Type</b> from the dropdown.</li>
          <li>Enter type name and set color.</li></ol>
        You can also add/edit/delete types via the main menu (see "Manage" section).</td>
</tr>
<tr>
   <td>Content</td>
   <td>Schedule description.</td>
</tr>
<tr>
   <td>Schedule Time</td>
   <td>Set all-day or timed schedules; Gregorian or lunar calendar dates/times.
   <ul>
       <li>All-day
        <ul><li>Checked: Only dates can be set for start/end times.</li>
           <li>Unchecked: Dates, hours, and minutes can be set.</li></ul>
        </li>
    </ul>
   <ul>
     <li>Calendar Type
        <ul><li>Gregorian: Displays Gregorian dates only.</li>
            <li>Lunar: Displays both Gregorian and lunar dates.</li></ul>
     </li>
   </ul></td>
</tr>
<tr>
   <td>Reminder</td>
   <td>
All-day: Options include Never, Day of event (9 AM), 1 day before, 2 days before, or 1 week before.<br>
Timed: Options include Never, At start time, 15 min before, 30 min before, 1 hour before, 1 day before, 2 days before, or 1 week before.</td>
</tr>
<tr>
   <td>Repeat</td>
   <td>
Gregorian: Never, Daily, Weekdays, Weekly, Monthly, Yearly.<br>
Lunar: Never, Monthly, Yearly.</td>
</tr>
<tr>
   <td>End Repeat</td>
   <td>Appears only if repeat is enabled. Options: Never, After n times, or By date.</td>
</tr> 
</table>

3. Click **Save** to create the schedule. Schedules appear as tags in calendar views.

### Editing Schedules

1. Double-click or right-click a schedule tag in Month/Week/Day view.
2. Select **Edit**.
3. Modify schedule properties in the "Edit Schedule" window and click **Save**.
4. If editing all-day or repeating schedules, confirm the prompt to complete changes.

> ![notes](../common/notes.svg) Note: Drag schedule tags to modify start/end times.

Prompt buttons during editing:

<table border="1">
<tr>
   <th width="130px">Button</th>
   <th width="200px">Description</th>
</tr>
<tr>
   <td>All Schedules</td>
   <td>Modify all related recurring schedules.</td>
</tr>
<tr>
   <td>This Schedule Only</td>
   <td>Modify only the current schedule.</td>
</tr>
<tr>
   <td>All Future Schedules</td>
   <td>Modify schedules from the selected date onward; earlier schedules remain unchanged.</td>
</tr>
<tr>
   <td>Change All</td>
   <td>Modify all recurring schedules.</td>
</tr> 
</table>

### Setting All-Day/Multi-Day Schedules

When creating/editing schedules, set **Start Time** and **End Time** to create all-day or multi-day schedules.

### Setting Recurring Schedules

1. When creating/editing schedules, select a repeat cycle (e.g., Monthly) from the **Repeat** dropdown.
2. Set end conditions in the **End Repeat** dropdown.

<img src="fig/repeat.png" alt="pic" style="zoom:67%;" />

### Searching Schedules

1. Click ![search](../common/search.svg) in the top search box.
2. Enter keywords and press **Enter**.
3. Click ![0|close](../common/close_icon.svg) or delete text to clear keywords/cancel search.

### Viewing Schedules

Double-click a schedule tag in Month/Week/Day view to open the "My Schedule" window for viewing, editing, or deleting.

### Viewing Schedule Reminder Details

Click notification prompts to view schedule details when reminders appear.

Reminder action buttons:

<table border="1">
<tr>
   <th width="130px">Button</th>
   <th width="200px">Description</th>
</tr>
<tr>
   <td>Snooze</td>
   <td>First reminder: Snooze for 10 min; subsequent snoozes add 5 min each time.<br>Options: 15 min later, 1 hour later, 4 hours later, or tomorrow.</td>
</tr>
<tr>
   <td>Remind Tomorrow</td>
   <td>Appears for "1 day before" or "2 days before" reminders.</td>
</tr>
<tr>
   <td>Remind 1 Day Earlier</td>
   <td>Appears for "1 week before" reminders.</td>
</tr>
<tr>
   <td>Close</td>
   <td>Close the prompt.</td>
</tr> 
</table>

### Deleting Schedules

1. Double-click or right-click a schedule tag in Month/Week/Day view.
2. Select **Delete**.
3. Confirm deletion in the prompt.

Deletion prompt buttons:

<table border="1">
<tr>
   <th width="130px">Button</th>
   <th width="200px">Description</th>
</tr>
<tr>
   <td>Delete Schedule</td>
   <td>Delete non-recurring schedules.</td>
</tr>
<tr>
   <td>Delete All</td>
   <td>Delete all recurring schedules.</td>
</tr>
<tr>
   <td>Delete This Only</td>
   <td>Delete only the selected recurring schedule.</td>
</tr>
<tr>
   <td>Delete Future Schedules</td>
   <td>Delete recurring schedules from the selected date onward; earlier schedules remain.</td>
</tr> 
</table>

## Main Menu

Access management settings, view privacy policy, switch themes, view help, or learn more about Calendar.

### Manage

Click ![icon_menu](../common/icon_menu.svg) > **Manage** to:
- Manage schedule types
- Sync schedule data/calendar settings to cloud via UOS/deepin ID
- Configure general settings

#### Cloud Sync

Sync schedule data and settings to cloud via UOS/deepin ID.

> ![notes](../common/notes.svg) Note: Not available on older UOS/deepin versions.

**Logging In/Out**
1. Click **Log In** in Account Settings.
2. Log in via UOS/deepin ID/password, SMS, or WeChat scan.
3. Click **Log Out** to sign out.

**Sync Settings**
1. While logged in, toggle **Schedules** and **General Settings** to enable/disable sync.
2. Select sync frequency or choose **Manual Sync**.
3. Click **Sync Now** for immediate sync.

> ![notes](../common/notes.svg) Note: Requires enabling **UOS/deepin Cloud Sync** in Control Center.

#### Managing Schedule Types

**Add Type**
1. Click ![icon](../common/add1.svg) in Calendar Management.
2. Enter name and set color.
3. Click **Save**.

**Edit Type**
1. Select a custom type.
2. Click ![icon](../common/edit.svg).
3. Modify name/color.
4. Click **Save**.

**Delete Type**
1. Select a custom type.
2. Click ![icon](../common/delete.svg).

#### General Settings

**First Day of Week**  
Set **Sunday** or **Monday** as the start of the week.

**Time Format**  
Choose **24-hour** or **12-hour** format.

### Theme

Window themes include Light, Dark, and System.
1. Click ![icon_menu](../common/icon_menu.svg) in Calendar.
2. Select **Theme** > Choose a theme.

### Help

View help manual for Calendar usage.
1. Click ![icon_menu](../common/icon_menu.svg).
2. Select **Help**.

### About

View Calendar version and info.
1. Click ![icon_menu](../common/icon_menu.svg).
2. Select **About**.

### Exit

1. Click ![icon_menu](../common/icon_menu.svg).
2. Select **Exit**.