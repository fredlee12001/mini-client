#!/usr/bin/env groovy
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

import groovy.json.JsonSlurperClassic 


@NonCPS
def jsonParse(def json) {
    new groovy.json.JsonSlurperClassic().parseText(json)
}

def is_empty(my_string) {
    return my_string==null || my_string.isEmpty()
}

def build(Map m) {
    def os                 = m.os
    def platform           = m.platform
    def toolchain          = m.toolchain
    def build_type         = m.build_type
    def env_vars           = m.env
    def shSetup            = m.setup
    def build_cmd          = m.cmd
    def artifact           = m.artifact
    def _scm               = m.scm
    def user_entry_refspec = (is_empty(m.user_entry_refspec)) ? _scm.branches : [[name: m.user_entry_refspec]]

    def shellCmdRunningOn = 'echo "running on $(hostname -f) in $PWD"'
    
    return {
        timeout(time: 60, unit: 'MINUTES') {
            node('prov_bld') {
                sh '''#!/bin/bash -xe
                find -name .git -type d -exec bash -c 'echo "------ cleaning $(dirname {})" && git -C $(dirname {}) reset --hard && git -C $(dirname {}) clean -fdx' \\;
                '''
                checkout(
                    [
                        $class: 'GitSCM',
                        branches: user_entry_refspec,
                        browser: _scm.browser,
                        doGenerateSubmoduleConfigurations: _scm.doGenerateSubmoduleConfigurations,
                        extensions: _scm.extensions + [
                            [
                                $class: 'CloneOption',
                                noTags: true,
                                reference: '',
                                shallow: true,
                                timeout: 25,
                                depth: 1
                            ],
                            [$class: 'CleanBeforeCheckout']
                        ],
                        submoduleCfg: [],
                        userRemoteConfigs: _scm.userRemoteConfigs
                    ]
                )
                withEnv(env_vars) {
                    try {
					    // Setup
                        if (!"".equals(shSetup)) {
                            sh """#!/bin/bash -xe
                            ${shSetup}
                            """
                        }

						// Build
                        sh """#!/bin/bash -xe
		                $shellCmdRunningOn
                        ${build_cmd}
                        """

						// Result
                        if (!is_empty(artifact)) {
                            stash name: "${platform}.${os}.${toolchain}.${build_type}", includes: "${artifact},jenkins/unity_to_junit.py"
                        }
                    } finally {
                        junit allowEmptyResults:false, healthScaleFactor:0.0, keepLongStdio:true, testResults:'*build_junit.xml'
                        publishHTML(
                            target: [ 
                                allowMissing: true,
                                alwaysLinkToLastBuild: false,
                                keepAll: true,
                                reportDir: "${coverity_report_dir}/html-report",
                                reportFiles: 'index.html',
                                reportName: "Coverity-${platform}-${toolchain}"
                            ]
                        )
                    }
                }
            }
        }
    }
}


def test(Map m){
    def os           = m.os
    def platform     = m.platform
    def toolchain    = m.toolchain
    def build_type   = m.build_type
    def test_cmd     = m.cmd

    return {
        timeout(time: 50, unit: 'MINUTES') {
            node('prov_bld') {
                dir("test-${platform}.${os}.${toolchain}.${build_type}") {
                    unstash name: "${platform}.${os}.${toolchain}.${build_type}"
                    try {
                        sh """#!/bin/bash -lxe
                        set -o pipefail
                        $test_cmd 2>&1 | tee test.log
                        """
                    } finally {
                        sh """#!/bin/bash -lxe
                        python jenkins/unity_to_junit.py -o test.${platform}.${os}.${toolchain}.${build_type}.junit.xml -s Test.${platform}.${os}.${toolchain}.${build_type} test.log
                        """
                        junit allowEmptyResults:false, healthScaleFactor:0.0, keepLongStdio:true, testResults:'*.junit.xml'
                    }
                }
            }
        }
    }
}



def run_job(Map m) {
    def _scm               = m.scm
    def user_entry_refspec = m.entry_refspec

    def currBuildStatus = hudson.model.Result.SUCCESS

    timestamps {
        ansiColor('xterm') {
            try {
                stage('Build') {
                    parallel(
                        //----------------------------------------------------------- SW Linux Debug -----------------------------------------------------------------
                        'Build-Linux-Debug': build(
                            os: 'Linux',
                            platform: 'PC',
                            toolchain: 'GCC',
                            build_type: 'Debug',
                            env: [],
                            setup: 'source devenv/pv_env_setup.sh PC && set_pv_build debug',
                            cmd: 'make rebuild',
                            artifact: 'out/pc_linux_gcc-debug/unit_tests/fcc_component_tests.elf',
                            //-----------------------------------
                            scm: _scm,
                            user_entry_refspec: user_entry_refspec,
                        ),
                    )
                }
            } catch (Exception e) {
                currBuildStatus = hudson.model.Result.FAILURE
                throw e
            } finally {
                def subjectStatus = 'SUCCESS'
                def shouldSend = true
                def prevBuildStatus = currentBuild.rawBuild.getPreviousBuild()?.getResult()
                if (currBuildStatus.equals(hudson.model.Result.SUCCESS)) {
                    if (prevBuildStatus.equals(hudson.model.Result.FAILURE) ||
                        prevBuildStatus.equals(hudson.model.Result.ABORTED) ||
                        prevBuildStatus.equals(hudson.model.Result.UNSTABLE)) {
                        subjectStatus = 'FIXED'
                    } else {
                        shouldSend = false
                    }
                } else {
                    if (prevBuildStatus.equals(hudson.model.Result.SUCCESS)) {
                        subjectStatus = 'BROKEN'
                    } else {
                        subjectStatus = 'Still FAILED'
                    }
                }
                
                if (shouldSend) {
                    node('prov_bld') {
                        def to_recipients = null
                        // we want to send emails to ALL only in case we are running on master
                        // Note: no spaces are allowed between eamils. separated by ;
                        if ("${env.BRANCH_NAME}".equals('master')){
                            to_recipients = 'motti.gondabi@arm.com'
                        }
                        emailext(
                            body: 'Please go to ${BUILD_URL} and verify the build', 
                            recipientProviders: [[$class: 'CulpritsRecipientProvider'], [$class: 'DevelopersRecipientProvider'], [$class: 'RequesterRecipientProvider']], 
                            subject: "${subjectStatus}! Job ${JOB_NAME} (${BUILD_NUMBER})", 
                            to: to_recipients
                        )
                    }
                }
            }
        }
    }
}

return this;
