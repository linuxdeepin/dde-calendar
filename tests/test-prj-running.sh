#!/bin/bash
export builddir=build
export reportdir=build-ut
export scriptdir="$(cd "$(dirname "$0")" && pwd)"
export projectdir="$(cd "${scriptdir}/.." && pwd)"
rm -rf $builddir
rm -rf ../$builddir
rm -rf $reportdir
rm -rf ../$reportdir
mkdir ../$builddir
mkdir ../$reportdir
cd ../$builddir
#编译
# BUILD_LEGACY_CALENDAR_TESTS=ON: 编入 client/service 全量单测（默认 OFF 时 client 仅 2 个用例、
# service 仅固定列表用例参与统计，覆盖率与用例数会严重失真）
cmake -DCMAKE_BUILD_TYPE=Debug -DBUILD_TESTING=ON -DBUILD_LEGACY_CALENDAR_TESTS=ON \
      -DCMAKE_SAFETYTEST_ARG="CMAKE_SAFETYTEST_ARG_ON" ..
make -j8
#生成asan日志和ut测试xml结果
# 运行全部 5 个测试目标；gen-ut-summary.py 会汇总 report/ 下所有 XML。
# 每个二进制加 timeout 保护，避免个别交互类用例在无人值守环境下卡住整个流水线。
mkdir -p report
set +e
timeout 900 ./tests/dde-calendar-client-test/dde-calendar-test --gtest_output=xml:./report/report_dde-calendar-client.xml
test_exit_code=$?
timeout 1200 ./tests/dde-calendar-service-test/dde-calendar-service-test --gtest_output=xml:./report/report_dde-calendar-service.xml
test_exit_code=$((test_exit_code || $?))
timeout 600 ./tests/dde-calendar-schedule-test/dde-calendar-schedule-test --gtest_output=xml:./report/report_dde-calendar-schedule.xml
test_exit_code=$((test_exit_code || $?))
timeout 600 ./tests/dde-calendar-schedule-plugin-test/dde-calendar-schedule-plugin-test --gtest_output=xml:./report/report_dde-calendar-schedule-plugin.xml
test_exit_code=$((test_exit_code || $?))
timeout 300 ./tests/dde-schedule-dbus-test/dde-schedule-dbus-test --gtest_output=xml:./report/report_dde-schedule-dbus.xml
test_exit_code=$((test_exit_code || $?))
set -e

workdir=$(cd ../$(dirname $0)/$builddir; pwd)

mkdir -p report
#统计代码覆盖率并生成html报告
lcov -d $workdir -c -o ./coverage.info

lcov --extract ./coverage.info '*/src/*' -o ./coverage.info

lcov --remove ./coverage.info '*/tests/*' -o ./coverage.info

genhtml -o ./html ./coverage.info

mv ./html/index.html ./html/cov_dde-calendar.html
#对asan、ut、代码覆盖率结果收集至指定文件夹
cp -r html ../$reportdir/
cp -r report ../$reportdir/
cp -r asan*.log* ../$reportdir/asan_dde-calendar.log 2>/dev/null || true

# 生成摘要 JSON
echo "==> Generating summary JSON: ../$reportdir/ut-summary.json"

python3 "${scriptdir}/gen-ut-summary.py"

exit $test_exit_code
