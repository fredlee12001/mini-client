# Top level Makefile for mFCC

include $(SA_PV_TOP)/devenv/build-common/locations.mk
include $(SA_PV_TOP)/devenv/build-common/gen.mk
include $(SA_PV_TOP)/devenv/build-common/armcc_licenses.mk
include $(SA_PV_TOP)/devenv/build-common/valgrind.mk

$(info === Compiling in SA_PV_TOP=$(SA_PV_TOP) ===)

# This is done so that the pipe will fail if a failed command is piped
# Used only for warnings_as_errors.mk
SHELL=/bin/bash -o pipefail

# Set target `all` as default target
.DEFAULT_GOAL := all

#.PHONY: help help-components
#help:
	#@cat $(SA_PV_TOP)/devenv/help/help.txt
#help-components:
	#@cat $(SA_PV_TOP)/devenv/help/help-components.txt
	
# Create out directory
$(PV_OUT_DIR):
	@echo [MKDIR] $@
	@mkdir -p $@
	@echo [MKDIR] $(PV_OUT_UNIT_TESTS_DIR)
	@mkdir -p $(PV_OUT_UNIT_TESTS_DIR)
	@echo [MKDIR] $(PV_OUT_APPS_DIR)
	@mkdir -p $(PV_OUT_APPS_DIR)

ifeq ($(SA_PV_PLAT),)
$(error SPV environment is not set up. Source pv_env_setup.sh first.)
endif
PV_MK_PATH = devenv/build-platform/$(PV_PLAT_VECTOR).mk
ifeq ($(wildcard $(PV_MK_PATH)),)
  $(error Unsupported platform: $(SA_PV_PLAT) - $(SA_PV_HW)/$(SA_PV_HLOS)/$(SA_PV_TOS))
endif

export PATH_WITHOU_SPV_TOP_STEM = $(patsubst $(SA_PV_TOP)/%,%,$(1))

.PHONY: all
all: $(PV_OUT_DIR)
	@make -f $(PV_MK_PATH) $@

.PHONY: install_license
install_license: import_arm_licenses

.PHONY: gen_testdata
gen_testdata: $(TESTS_TWO_TARGETS)

.PHONY: rebuild
rebuild:
	@# Using explicit make runs here to guarantee correct ordering of dependencies.
	@make clean
	@make -f $(PV_MK_PATH) deploy
	@make all

# More (platform specific) clean should be added as dependency in platform specific mk file
.PHONY: clean
clean:
	@echo [CLN] $(PV_OUT_DIR)
	@rm -rf $(PV_OUT_DIR)
	@make -f $(PV_MK_PATH) $@

# Removes all artifacts (for all platforms).
# NOTE: This doesn't clean platform-specific artifacts which aren't in "out" (primarily,
# ndk-build aritfacts such as "libs" and "obj"), which is a shame.
.PHONY: deepclean_all
deepclean: clean
	@echo [DEEPCLN]
	@python $(SA_PV_TOP)/env_setup.py clean --repos

.PHONY: check
check:
	@make -f $(PV_MK_PATH) valg_cmd="" $@

.PHONY: valgrind
valgrind:
	make -f $(PV_MK_PATH) valg_cmd="$(VALGRIND_CMD_LINE)" check

# Dummy target for targets that should always be built
.PHONY: FORCE
FORCE:

# Check that none of the files in agent_tee contain forbidden functions (for example, because they
# don't exist and TZInfra must be used instead).
# The script runs from the agent_tee directory, and scans everything - including symlinks - so it gets to all
# files compiled in trusted environment.
.PHONY: check_forbidden_functions
check_forbidden_functions: gen
	@echo [CHK] Checking for forbidden functions.
	@cd $(SA_PV_TOP) && python $(SA_PV_TOP)/devenv/find-forbidden-functions/find_forbidden_functions.py $(SA_PV_TOP)/devenv/find-forbidden-functions/find_forbidden_functions-conf.yaml

PRE_SUBMIT_SCRIPT := /bin/bash devenv/pre-submit/pre_submit.sh

# pre_submit targets may be invoked even if environment was not set
.PHONY: pre_submit
pre_submit:
	$(PRE_SUBMIT_SCRIPT) --profile all

### pre_submit Profiles ###

# Code Coverage Profile
#.PHONY: code_coverage
#code_coverage:
#	$(PRE_SUBMIT_SCRIPT) --profile code_coverage

#Doxygen builder
.PHONY: doxygen
doxygen:
	/bin/bash devenv/doxygen/doxygen.sh

.PHONY: doxygen_rebuild
doxygen_rebuild:
	/bin/bash devenv/doxygen/doxygen.sh rebuild