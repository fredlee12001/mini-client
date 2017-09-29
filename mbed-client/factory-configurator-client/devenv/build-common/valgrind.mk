

#Valgrind command line.
#If new suppressions needs to be added, use --gen-suppressions=all flag - outputs the suppression needed
VALGRIND_CMD_LINE=\
	valgrind \
	--leak-check=full \
	--track-origins=yes \
	--read-var-info=yes \
	--show-leak-kinds=all \
	--suppressions=$(SA_PV_TOP)/devenv/build-common/valgrind_suppressions.supp \
	--errors-for-leak-kinds=all \
	--gen-suppressions=all \
	--error-exitcode=1 \
	-v

