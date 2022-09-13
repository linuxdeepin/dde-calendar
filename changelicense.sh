#!/bin/bash
COPYRIGHT="UnionTech Software Technology Co., Ltd."
LICENSE="LGPL-3.0-or-later"
DIR=$1
FILES=`find $DIR -type f -name "*.h" -o -name "*.cpp"`
for FILE in $FILES;
do
    PREPROCESSORS=`grep -n ^\# $FILE --binary-files=without-match`
    if [ -n "$PREPROCESSORS" ]; then
        # a cpp file or header file, we should check copyright first
        OLD_COPYRIGHT=`grep -i -E --color=auto "(Deepin Technology|Uniontech)" $FILE --binary-files=without-match`
        SPDX_CONTENT=$(grep -i "SPDX" $FILE)
        if [ -z "$SPDX_CONTENT" ]; then
            FIRST_LINE=$(echo `echo $PREPROCESSORS | cut -d : -f 1`)
            if [ -n "$OLD_COPYRIGHT" ]; then
                # This is our license
                # acquire year
                YEAR=$(echo $OLD_COPYRIGHT | grep -o --color=auto -P "20\d{2}")
                YEAR=$(echo $YEAR | cut -d " " -f 1)
                if [ "$YEAR" != "2022" ]; then
                    YEAR="$YEAR - 2022"
                fi
                if [ "$FIRST_LINE" != "1" ]; then
                    FIRST_LINE=`expr $FIRST_LINE - 1`
                    sed -i "1,${FIRST_LINE}d" $FILE
                fi
                reuse addheader --style "c" --year "$YEAR" --copyright "$COPYRIGHT" --license "$LICENSE" $FILE
            else
                # if no copyright
                COPYRIGHT_CONTENT=$(grep -i "Copyright" $FILE)
                if [ -z "$COPYRIGHT_CONTENT" ]; then
                    if [ "$FIRST_LINE" != "1" ]; then
                        FIRST_LINE=`expr $FIRST_LINE - 1`
                        sed -i "1,${FIRST_LINE}d" $FILE
                    fi
                    reuse addheader --style "c" --copyright "$COPYRIGHT" --license "$LICENSE" $FILE
                fi
            fi
        fi
    fi
done
#reuse lint
