# ----------------------------------------------------------------------------
# Copyright 2016-2017 ARM Ltd.
#  
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#  
#     http://www.apache.org/licenses/LICENSE-2.0
#  
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
# ----------------------------------------------------------------------------
# Checks the returned status of the previous command, outputting a large
# and noticeable error message on failure (with parameter as error).
# Also exits the enclosing script with an error.
check_pre_submit_error() {
	if [ $? -ne 0 ]
	then
		tput setaf 1  # Change foreground color to red.
		toilet -f standard Sorry...  # Print a large message.
		echo "========================================================================"
		echo "     "$1 | fmt -w 70
		echo "========================================================================"
		echo
		tput sgr0  # Reset changes in color.
		exit 1
	fi
}

# Profiling helpers -

PROFILE_TIMES=()
PROFILE_TOTAL_SECONDS=0

profile_start()
{
    START_SECONDS=`date +%s`
    echo $START_SECONDS
}

profile_end()
{
    START_SECONDS=$1
    TITLE=$2
    END_SECONDS=`date +%s`
    ELAPSED_SECONDS=$(( $END_SECONDS - $START_SECONDS ))
    echo $TITLE: $ELAPSED_SECONDS seconds
    PROFILE_TIMES+=("$TITLE: $ELAPSED_SECONDS seconds")
    PROFILE_TOTAL_SECONDS=$(( $PROFILE_TOTAL_SECONDS + $ELAPSED_SECONDS ))
}

profile_print_times()
{
    local MODULE=$1
    echo "=== Profiling results for $MODULE ==="
    for line in "${PROFILE_TIMES[@]}"; do
	    echo " *" $line
    done
    echo
	local PROFILE_TOTAL_MINUTES=$(( $PROFILE_TOTAL_SECONDS / 60 ))
	local PROFILE_REMAINING_SECONDS=$(( $PROFILE_TOTAL_SECONDS - ( $PROFILE_TOTAL_MINUTES * 60 ) ))
	local PROFILE_JOINED_TIME=`printf %02d:%02d $PROFILE_TOTAL_MINUTES $PROFILE_REMAINING_SECONDS`
	echo Total time for $MODULE - $PROFILE_TOTAL_SECONDS seconds \($PROFILE_JOINED_TIME minutes\).
	echo "====================================="
	echo
}

