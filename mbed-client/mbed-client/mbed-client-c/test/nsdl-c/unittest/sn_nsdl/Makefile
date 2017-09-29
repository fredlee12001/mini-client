include ../makefile_defines.txt

COMPONENT_NAME = sn_nsdl_unit

#This must be changed manually
SRC_FILES = \
        ../../../../source/sn_nsdl.c

TEST_SRC_FILES = \
        main.cpp \
        sn_nsdltest.cpp \
        test_sn_nsdl.c \
        ../../../../../../unittests/stub/sn_coap_protocol_stub.c \
        ../../../../../../unittests/stub/sn_grs_stub.c \
        ../../../../../../unittests/stub/sn_coap_parser_stub.c \
        ../../../../../../unittests/stub/sn_coap_builder_stub.c \

include ../MakefileWorker.mk

CPPUTESTFLAGS += -DYOTTA_CFG_COAP_MAX_BLOCKWISE_PAYLOAD_SIZE=16

