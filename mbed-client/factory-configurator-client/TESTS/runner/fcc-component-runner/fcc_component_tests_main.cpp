// ----------------------------------------------------------------------------
// Copyright 2016-2017 ARM Ltd.
//  
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//  
//     http://www.apache.org/licenses/LICENSE-2.0
//  
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
// ----------------------------------------------------------------------------


// Note: this macro is needed on armcc to get the the PRI*32 macros
// from inttypes.h in a C++ code.
#ifndef __STDC_FORMAT_MACROS
#define __STDC_FORMAT_MACROS
#endif

#include <stdlib.h>

#include "PlatIncludes.h"
#include "pal_test_main.h"

#include "pal.h"

#include "unity_fixture.h"
#include "pv_error_handling.h"
#include "fcc_component_tests_runner.h"

#include "mbed-trace/mbed_trace.h"
#include "mbed-trace-helper.h"



#define TRACE_GROUP     "fcc"  // Maximum 4 characters

extern bool dhcp_done;
static int g_unity_status = EXIT_FAILURE;
static pal_args_t g_args = { 0 };


/**
*
* Runs all tests in a task of its own
*/
static void run_component_tests_task(pal_args_t *args)
{
    int rc = 0;
    bool success = 0;
    palStatus_t pal_status;
    bool is_mutex_used = false;

    int myargc = args->argc + 2;
    const char **myargv = (const char **)calloc(myargc, sizeof(char *));
    if (myargv == NULL) {
        goto cleanup;
    }
    myargv[0] = "fcc_component_tests";
    myargv[1] = "-v";
    for (int i = 0; i < args->argc; i++) {
        myargv[i + 2] = args->argv[i];
    }

    // Initialize PAL
    pal_status = pal_init();
    if (pal_status != PAL_SUCCESS) {
        goto cleanup;
    }

    //Initialize mbed-trace
    success = mbed_trace_helper_init(TRACE_ACTIVE_LEVEL_ALL, is_mutex_used);
    if (success != true) {
        goto cleanup;
    }

    setvbuf(stdout, (char *)NULL, _IONBF, 0); /* Avoid buffering on test output */
    SA_PV_LOG_INFO("component_tests: Starting component tests...");

    // Wait until device is connected to the world
    while (dhcp_done == 0) {
        pal_osDelay(500);
    }

    rc = UnityMain(myargc, myargv, RunAllComponentTests);
    if (rc > 0) {
        SA_PV_LOG_ERR("component_tests: Test failed.");
    } else {
        g_unity_status = EXIT_SUCCESS;
        SA_PV_LOG_INFO("component_tests: Test passed.");
    }

cleanup:
    mbed_trace_helper_finish();
    pal_destroy();
    free(myargv);
    fflush(stdout);
}

int main(int argc, char * argv[])
{
    bool success = 0;

    // Do not use argc/argv as those are not initialized
    // for armcc and may cause allocation failure.
    (void)argc;
    (void)argv;

    g_args.argc = 0;
    g_args.argv = NULL;

    success = initPlatform();
    if (success) {
        success = runProgram(&run_component_tests_task, &g_args);
    }
    return success ? g_unity_status : EXIT_FAILURE;
}
