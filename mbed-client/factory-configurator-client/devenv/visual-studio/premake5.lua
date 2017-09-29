-- ----------------------------------------------------------------------------
-- Copyright 2016-2017 ARM Ltd.
--  
-- Licensed under the Apache License, Version 2.0 (the "License");
-- you may not use this file except in compliance with the License.
-- You may obtain a copy of the License at
--  
--     http://www.apache.org/licenses/LICENSE-2.0
--  
-- Unless required by applicable law or agreed to in writing, software
-- distributed under the License is distributed on an "AS IS" BASIS,
-- WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
-- See the License for the specific language governing permissions and
-- limitations under the License.
-- ----------------------------------------------------------------------------

solution "factory-configurator-client"
   configurations { "Debug", "Release" }
   location "../../"

project "factory-configurator-client"
   kind "ConsoleApp"
   language "C"
   targetdir "bin/%{cfg.buildcfg}"
   location "../../"

   files {
        "../../crypto/**.*",
        "../../crypto-service/**.*",
        "../../devenv/**.*",
        "../../ftcd-comm-base/**.*",
        "../../ftcd-comm-socket/**.*",
        "../../ftcd-comm-serial/**.*",
        "../../key-config-manager/**.*",
        "../../logger/**.*",
        "../../factory-configurator-client/**.*",
        "../../mbed-trace-helper/**.*",
        "../../fcc-bundle-handler/**.*",
        "../../fcc-output-info-handler/**.*",
        "../../out/gen/testdata/*.*",
        "../../secsrv-cbor/**.*",        
        "../../storage/**.*",        
        "../../TESTS/**.*",
        "../../unity/*.*",
        "../../utils/**.*",
        "../../Makefile",
        "../../Readme.md",
        "../../env_setup.json",
        "../../env_setup.py",
        "../../CMakeLists.txt",
		"../../*.cmake",
		"../../Jenkinsfile"
        }

    removefiles { "../../**/.git/**" }