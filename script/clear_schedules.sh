#!/bin/bash

# Clear User Schedules Script - DDE Calendar
# 清理用户日程的脚本 (保留系统日程)
# 
# 功能：先查询所有日程，识别用户创建的日程，然后逐个删除用户日程，保留系统日程(节假日等)
# Usage: ./clear_schedules.sh

# set -e  # Commented out to allow proper error handling in deletion loop

# Configuration
SERVICE="com.deepin.Calendar"
OBJECT_PATH="/com/deepin/Calendar"
INTERFACE="com.deepin.Calendar"

# Color codes for terminal output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
PURPLE='\033[0;35m'
CYAN='\033[0;36m'
NC='\033[0m' # No Color

# Statistics
total_found=0
total_deleted=0
total_failed=0

# Logging functions
log() {
    local message="$1"
    local timestamp=$(date "+%Y-%m-%d %H:%M:%S")
    echo -e "[$timestamp] $message"
}

log_colored() {
    local color="$1"
    local message="$2"
    local timestamp=$(date "+%Y-%m-%d %H:%M:%S")
    echo -e "${color}[$timestamp] $message${NC}"
}

info_log() {
    log_colored "$YELLOW" "ℹ INFO: $1"
}

success_log() {
    log_colored "$GREEN" "✓ SUCCESS: $1"
}

error_log() {
    log_colored "$RED" "✗ ERROR: $1"
}

# D-Bus call wrapper with better error handling
call_dbus() {
    local method="$1"
    shift
    local result
    
    if result=$(dbus-send --session --print-reply --dest="$SERVICE" "$OBJECT_PATH" "$INTERFACE.$method" "$@" 2>&1); then
        if echo "$result" | grep -q "string"; then
            # Extract string value, handling both quoted and unquoted formats
            echo "$result" | sed -n 's/.*string "\(.*\)"/\1/p'
        elif echo "$result" | grep -q "boolean"; then
            echo "$result" | grep "boolean" | tail -1 | awk '{print $2}'
        else
            echo "$result"
        fi
    else
        error_log "DBus call failed: $result"
        return 1
    fi
}

# Check if a schedule is a system schedule (festival/holiday)
# Now uses the scheduleType field provided by the D-Bus API - more intuitive!
is_system_schedule() {
    local schedule_block="$1"
    
    # Extract scheduleType field from the API response
    local schedule_type=$(echo "$schedule_block" | grep -o '"scheduleType":"[^"]*"' | sed 's/"scheduleType":"//g' | sed 's/"//g')
    
    if [ "$schedule_type" = "system" ]; then
        return 0  # Is system schedule
    else
        return 1  # Is user schedule (including "user" or any other type)
    fi
}

# Extract user schedule IDs only (exclude system schedules)
extract_user_schedule_ids() {
    local json_response="$1"
    local temp_results="/tmp/schedule_results.$$"
    local temp_ids="/tmp/user_ids.$$"
    
    # Clear temp files
    > "$temp_results"
    > "$temp_ids"
    
    # Extract schedule objects and save to temp file for processing
    echo "$json_response" | grep -o '{[^}]*"id":"[^"]*"[^}]*}' > "$temp_results"
    
    local user_count=0
    local system_count=0
    
    # Process each schedule object - redirect all logs to stderr
    while IFS= read -r schedule_obj; do
        if [ -n "$schedule_obj" ]; then
            # Extract schedule ID and title
            local schedule_id=$(echo "$schedule_obj" | grep -o '"id":"[^"]*"' | sed 's/"id":"//g' | sed 's/"//g')
            local title=$(echo "$schedule_obj" | grep -o '"title":"[^"]*"' | sed 's/"title":"//g' | sed 's/"//g')
            
            if [ -n "$schedule_id" ] && [ -n "$title" ]; then
                if is_system_schedule "$schedule_obj"; then
                    echo -e "${YELLOW}[$(date "+%Y-%m-%d %H:%M:%S")] ℹ INFO: 跳过系统日程: $schedule_id ($title)${NC}" >&2
                    system_count=$((system_count + 1))
                else
                    echo -e "${YELLOW}[$(date "+%Y-%m-%d %H:%M:%S")] ℹ INFO: 发现用户日程: $schedule_id ($title)${NC}" >&2
                    echo "$schedule_id" >> "$temp_ids"
                    user_count=$((user_count + 1))
                fi
            fi
        fi
    done < "$temp_results"
    
    echo -e "${YELLOW}[$(date "+%Y-%m-%d %H:%M:%S")] ℹ INFO: 分析完成: $user_count 个用户日程，$system_count 个系统日程${NC}" >&2
    
    # Output only the user schedule IDs
    if [ -f "$temp_ids" ] && [ -s "$temp_ids" ]; then
        cat "$temp_ids"
    fi
    
    # Cleanup
    rm -f "$temp_results" "$temp_ids"
}

# Get all schedules in a wide date range
get_all_schedules() {
    info_log "查询所有日程..."
    
    # Query a wide date range to get all schedules (past 1 year to future 1 year)
    local start_date=$(date -d "-1 year" "+%Y-%m-%d")
    local end_date=$(date -d "+1 year" "+%Y-%m-%d")
    local query="${start_date},${end_date}"
    
    info_log "查询日期范围: $start_date 到 $end_date"
    
    if result=$(call_dbus "QuerySchedules" string:"$query"); then
        echo "$result"
    else
        error_log "无法查询日程"
        return 1
    fi
}

# Delete a single schedule
delete_schedule() {
    local schedule_id="$1"
    
    local result
    if result=$(call_dbus "ModifySchedule" string:"$schedule_id" string:"delete" string:"" 2>&1); then
        if echo "$result" | grep -q "true"; then
            ((total_deleted++))
            success_log "已删除日程: $schedule_id"
            return 0
        else
            ((total_failed++))
            error_log "删除失败: $schedule_id (服务返回: $result)"
            return 1
        fi
    else
        ((total_failed++))
        error_log "删除失败: $schedule_id (调用失败: $result)"
        return 1
    fi
}

# Main execution
main() {
    log_colored "$CYAN" "=========================================="
    log_colored "$CYAN" "DDE Calendar - Clear User Schedules Script"
    log_colored "$CYAN" "保留系统日程，仅清理用户创建的日程"
    log_colored "$CYAN" "=========================================="
    
    # Step 1: Check if D-Bus service is available
    info_log "检查D-Bus服务可用性..."
    if ! dbus-send --session --dest="$SERVICE" "$OBJECT_PATH" org.freedesktop.DBus.Introspectable.Introspect >/dev/null 2>&1; then
        error_log "D-Bus服务不可用，请确保DDE Calendar正在运行"
        exit 1
    fi
    success_log "D-Bus服务可用"
    
    # Step 2: Query all schedules
    log_colored "$BLUE" "=========================================="
    log_colored "$BLUE" "步骤 1: 查询所有日程"
    log_colored "$BLUE" "=========================================="
    
    local all_schedules_json
    if ! all_schedules_json=$(get_all_schedules); then
        error_log "查询日程失败"
        exit 1
    fi
    
    # Extract schedule count
    if echo "$all_schedules_json" | grep -q '"count"'; then
        total_found=$(echo "$all_schedules_json" | grep -o '"count":[0-9]*' | sed 's/"count"://')
    else
        total_found=0
    fi
    
    info_log "找到 $total_found 个日程"
    
    if [ "$total_found" -eq 0 ]; then
        success_log "没有找到需要删除的日程"
        log_colored "$PURPLE" "=========================================="
        log_colored "$PURPLE" "清理完成 - 无需删除任何日程"
        log_colored "$PURPLE" "=========================================="
        exit 0
    fi
    
    # Step 3: Extract user schedule IDs (excluding system schedules)
    log_colored "$BLUE" "=========================================="
    log_colored "$BLUE" "步骤 2: 提取用户日程ID (排除系统日程)"
    log_colored "$BLUE" "=========================================="
    
    local schedule_ids
    schedule_ids=$(extract_user_schedule_ids "$all_schedules_json")
    
    if [ -z "$schedule_ids" ]; then
        success_log "没有找到需要删除的用户日程"
        log_colored "$PURPLE" "=========================================="
        log_colored "$PURPLE" "清理完成 - 无用户日程需要删除"
        log_colored "$PURPLE" "=========================================="
        exit 0
    fi
    
    local user_schedule_count=$(echo "$schedule_ids" | wc -l)
    info_log "找到 $user_schedule_count 个用户日程需要删除"
    
    # Step 4: Delete user schedules only
    log_colored "$BLUE" "=========================================="
    log_colored "$BLUE" "步骤 3: 删除用户日程 (保留系统日程)"
    log_colored "$BLUE" "=========================================="
    
    # Add safety limit to prevent runaway deletion
    local max_deletions=100
    local count=0
    
    while IFS= read -r schedule_id; do
        if [ -n "$schedule_id" ]; then
            ((count++))
            
            # Safety check to prevent infinite loops
            if [ "$count" -gt "$max_deletions" ]; then
                error_log "安全限制: 已达到最大删除数量 ($max_deletions)，停止删除"
                break
            fi
            
            info_log "删除用户日程 $count/$user_schedule_count: $schedule_id"
            # Continue even if deletion fails
            delete_schedule "$schedule_id" || true
            
            # Add a small delay to avoid overwhelming the service
            sleep 0.1
        fi
    done <<< "$schedule_ids"
    
    # Step 5: Final verification
    log_colored "$BLUE" "=========================================="
    log_colored "$BLUE" "步骤 4: 验证清理结果"
    log_colored "$BLUE" "=========================================="
    
    info_log "验证是否还有剩余用户日程..."
    local remaining_schedules_json
    if remaining_schedules_json=$(get_all_schedules); then
        local remaining_user_ids
        remaining_user_ids=$(extract_user_schedule_ids "$remaining_schedules_json")
        local remaining_user_count=0
        if [ -n "$remaining_user_ids" ]; then
            remaining_user_count=$(echo "$remaining_user_ids" | wc -l)
        fi
        
        if [ "$remaining_user_count" -eq 0 ]; then
            success_log "验证成功 - 所有用户日程已清理完毕 (系统日程已保留)"
        else
            error_log "验证失败 - 仍有 $remaining_user_count 个用户日程未删除"
        fi
    else
        error_log "无法验证清理结果"
    fi
    
    # Final report
    log_colored "$PURPLE" "=========================================="
    log_colored "$PURPLE" "清理完成 - 最终统计"
    log_colored "$PURPLE" "=========================================="
    
    log "用户日程清理统计:"
    log "  - 总日程数: $total_found"
    log "  - 用户日程: $user_schedule_count"
    log "  - 成功删除: $total_deleted"
    log "  - 删除失败: $total_failed"
    if [ "$user_schedule_count" -gt 0 ]; then
        log "  - 删除成功率: $(( total_deleted * 100 / user_schedule_count ))%"
    else
        log "  - 删除成功率: N/A (无用户日程)"
    fi
    
    if [ "$total_failed" -eq 0 ]; then
        success_log "🎉 所有用户日程清理完成！系统日程已保留。"
    else
        error_log "⚠️  有 $total_failed 个用户日程删除失败，但清理已尽力完成"
    fi
}

# Trap for cleanup on exit
cleanup_on_exit() {
    info_log "脚本执行完毕"
}

trap cleanup_on_exit EXIT

# Execute main function
main "$@"
