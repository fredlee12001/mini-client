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

#include <assert.h>
#include <stdbool.h>

#include "fcc_component_tests_runner.h"
#include "general_tests_runner.h"
#include "factory_configurator_client.h"
#include "pv_error_handling.h"


static bool storage_delete(void)
{
    fcc_status_e status;
        
    status = fcc_init();
    SA_PV_ERR_RECOVERABLE_RETURN_IF((status != FCC_STATUS_SUCCESS), false, "Failed initializing FCC (status %u)", status);
    
    status = fcc_storage_delete();
    SA_PV_ERR_RECOVERABLE_RETURN_IF((status != FCC_STATUS_SUCCESS), false, "Failed clearing storage (status %u)", status);

    status = fcc_finalize();
    SA_PV_ERR_RECOVERABLE_RETURN_IF((status != FCC_STATUS_SUCCESS), false, "Failed finalizing FCC (status %u)", status);

    return true;
}

void RunAllComponentTests(void)
{
    bool success;

    // This will clear SD/Flash store before launching our test cases.
    // It is specifically essential when switching OS's.
    success = storage_delete();
    assert(success);

#ifdef PV_TEST_REPRODUCE_RANDOM_STATE
    // The function TestSetReproducedSeed() should be run only if we want to set fixed seed to pseudo random function
    TstSetReproducedSeed();
#endif
    RunAllGeneralTests();
}
