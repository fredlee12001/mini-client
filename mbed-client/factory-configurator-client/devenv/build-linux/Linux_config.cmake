# global defines and configuration goes here...
# defines here will affect all modules - not only factory-client-configurator

add_definitions(-DSA_PV_OS_LINUX)

add_definitions(
	-DMBED_CONF_MBED_TRACE_ENABLE=1
	-DMBED_CONF_MBED_TRACE_FEA_IPV6=0
)

