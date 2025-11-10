#!/usr/bin/env python3
"""
Holiday JSON Downloader Script

This script downloads holiday JSON files from the NateScarlet/holiday-cn GitHub repository.
It supports specifying the year, custom save paths, and handles file conflicts.

Souerce: https://github.com/NateScarlet/holiday-cn/tree/master

Usage:
    python download_holidays.py [year] [output_path]

Examples:
    python download_holidays.py 2025
    python download_holidays.py 2025 ./custom/path/
    python download_holidays.py 2025 ./custom/path/holidays.json
"""

import argparse
import json
import os
import sys
from datetime import datetime
from pathlib import Path
import urllib.request
import urllib.error


def download_holiday_json(year, output_path):
    """
    Download holiday JSON file for a specific year from GitHub.
    
    Args:
        year (int): The year to download holidays for
        output_path (str): The output path where the file should be saved
    
    Returns:
        str: The path where the file was saved
    """
    # GitHub raw file URL
    url = f"https://raw.githubusercontent.com/NateScarlet/holiday-cn/master/{year}.json"
    
    try:
        # Download the JSON file
        with urllib.request.urlopen(url) as response:
            data = response.read().decode('utf-8')
            
        # Parse and validate JSON
        holiday_data = json.loads(data)
        
        # Determine the final file path
        if os.path.isdir(output_path):
            filename = f"{year}.json"
            final_path = os.path.join(output_path, filename)
        else:
            final_path = output_path
            
        # Handle file existence
        if os.path.exists(final_path):
            print(f"文件 {final_path} 已存在！")
            choice = input("是否覆盖？(y/n): ").lower().strip()
            
            if choice != 'y':
                # Generate unique filename with timestamp
                timestamp = datetime.now().strftime("%Y%m%d_%H%M%S")
                name, ext = os.path.splitext(final_path)
                final_path = f"{name}_{timestamp}{ext}"
                print(f"文件将保存为: {final_path}")
        
        # Ensure directory exists
        os.makedirs(os.path.dirname(final_path), exist_ok=True)
        
        # Save the file
        with open(final_path, 'w', encoding='utf-8') as f:
            json.dump(holiday_data, f, ensure_ascii=False, indent=2)
            
        print(f"成功下载 {year} 年节假日数据到: {final_path}")
        return final_path
        
    except urllib.error.HTTPError as e:
        if e.code == 404:
            print(f"错误: 找不到 {year} 年的节假日数据")
        else:
            print(f"下载错误: {e}")
        sys.exit(1)
    except json.JSONDecodeError as e:
        print(f"JSON 解析错误: {e}")
        sys.exit(1)
    except Exception as e:
        print(f"未知错误: {e}")
        sys.exit(1)


def main():
    parser = argparse.ArgumentParser(
        description="""从 GitHub 下载中国节假日 JSON 数据
    Source: https://github.com/NateScarlet/holiday-cn/tree/master""",
        formatter_class=argparse.RawDescriptionHelpFormatter,
        epilog="""
示例:
  %(prog)s 2025                          # 下载 2025 年数据到默认路径
  %(prog)s 2025 ./custom/                # 下载到自定义目录
  %(prog)s 2025 ./holidays.json          # 下载到自定义文件路径
        """
    )
    
    parser.add_argument('year', type=int, help='要下载的年份 (例如: 2025)')
    parser.add_argument('output_path', nargs='?', 
                       help='输出路径 (默认: calendar-service/assets/holiday-cn/)')
    
    args = parser.parse_args()
    
    # Validate year
    current_year = datetime.now().year
    if args.year < 1990 or args.year > current_year + 10:
        print(f"警告: 年份 {args.year} 可能不在有效范围内")
        confirm = input("继续下载？(y/n): ").lower().strip()
        if confirm != 'y':
            print("已取消下载")
            sys.exit(0)
    
    # Set default output path if not provided
    if not args.output_path:
        default_path = os.path.join(os.getcwd(), 'calendar-service', 'assets', 'holiday-cn')
        output_path = default_path
    else:
        output_path = args.output_path
    
    # Convert relative path to absolute if needed
    if not os.path.isabs(output_path):
        output_path = os.path.join(os.getcwd(), output_path)
    
    print(f"准备下载 {args.year} 年节假日数据...")
    print(f"输出路径: {output_path}")
    
    # Download the holiday data
    download_holiday_json(args.year, output_path)


if __name__ == "__main__":
    main()