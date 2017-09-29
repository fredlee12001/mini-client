# global defines and configuration goes here...
# defines here will affect all modules - not only factory-client-configurator

add_definitions(-DSA_PV_OS_FREERTOS)

add_definitions(
	-DMBED_CONF_MBED_TRACE_ENABLE=1
	-DMBED_CONF_MBED_TRACE_FEA_IPV6=0
	-DPAL_NETWORK_BRINGUP=0 # skip network bringup in PlatformBSP code
	-DPAL_TEST_MAIN_THREAD_STACK_SIZE=7168 # define stack size for test thread
)

