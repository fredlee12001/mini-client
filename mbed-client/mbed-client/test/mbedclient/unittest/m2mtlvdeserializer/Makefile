include ../makefile_defines.txt

COMPONENT_NAME = m2mtlv_unit
SRC_FILES = \
    ../../../../source/m2mtlvdeserializer.cpp\
    ../../../../source/m2mtlvserializer.cpp\
    ../../../../source/m2mstringbufferbase.cpp

TEST_SRC_FILES = \
    main.cpp\
    m2mtlvdeserializertest.cpp\
    test_m2mtlvdeserializer.cpp\
    test_m2mtlvserializer.cpp\
    ../../../../../unittests/stub/m2mstring_stub.cpp\
    ../../../../../unittests/stub/m2mbase_stub.cpp\
    ../../../../../unittests/stub/m2mresource_stub.cpp\
    ../../../../../unittests/stub/m2mresourcebase_stub.cpp\
    ../../../../../unittests/stub/m2mresourceinstance_stub.cpp\
    ../../../../../unittests/stub/m2mobjectinstance_stub.cpp\
    ../../../../../unittests/stub/m2mobject_stub.cpp\
    ../../../../../unittests/stub/nsdlaccesshelper_stub.cpp\
    ../../../../../test_modules/nanostack-libservice/source/libBits/common_functions.c\
    ../../../../../unittests/stub/m2mcorememory_stub.cpp

CPPUTEST_LDFLAGS = -Wl,-wrap,malloc
include ../MakefileWorker.mk
