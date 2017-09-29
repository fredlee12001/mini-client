/*
 * Copyright (c) 2015 ARM Limited. All rights reserved.
 * SPDX-License-Identifier: Apache-2.0
 * Licensed under the Apache License, Version 2.0 (the License); you may
 * not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an AS IS BASIS, WITHOUT
 * WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#include "CppUTest/TestHarness.h"
#include "test_m2mbase.h"

#include "m2mreportobserver.h"
#include "m2mreporthandler.h"
#include "m2mreporthandler_stub.h"
#include "nsdlaccesshelper_stub.h"
#include "m2mresource_stub.h"
#include "m2mobject_stub.h"
#include "m2mobjectinstance_stub.h"
#include "m2mresource_stub.h"

static bool value_update_called = false;
static void value_updated_function2(const char* name) {
    value_update_called = true;
}

class MyTest{
public:
    void value_updated_function(const char* name) {
        visited = true;
    }

    bool visited;
};


class Observer : public M2MReportObserver {
public:
    Observer(){}
    ~Observer(){}

    virtual void observation_to_be_sent(const m2m::Vector<uint16_t>&, uint16_t ,bool){}
};

Test_M2MBase::Test_M2MBase(char* path, Handler *handler)
    : M2MBase(M2MBase::ObjectInstance, 1,
     M2MBase::Static,
#ifndef DISABLE_RESOURCE_TYPE
     "type",
#endif
     path,
     false,
     false)
{
    obsHandler = handler;
}

Test_M2MBase::Test_M2MBase(uint16_t name_id,
        M2MBase::Mode mode,
#ifndef DISABLE_RESOURCE_TYPE
        const char *resource_type,
#endif
        char *path,
        bool external_blockwise_store,
        bool multiple_instance,
        M2MBase::DataType type)
: M2MBase(M2MBase::ObjectInstance,
        name_id,
        mode,
#ifndef DISABLE_RESOURCE_TYPE
        resource_type,
#endif
        path,
        external_blockwise_store,
        multiple_instance,
        type)
{
}

Test_M2MBase::~Test_M2MBase()
{
    free_resources();
}

M2MObservationHandler* Test_M2MBase::observation_handler() const
{
    return NULL;
    return obsHandler;
}

void Test_M2MBase::set_observation_handler(M2MObservationHandler *handler)
{
//    obsHandler = handler;
}


void Test_M2MBase::test_set_operation()
{
    this->_sn_resource.dynamic_resource_params.mode = M2MBase::Static;
    M2MBase::Operation test = M2MBase::GET_ALLOWED;
    set_operation(test);

    CHECK(test == this->operation());

    this->_sn_resource.dynamic_resource_params.mode = M2MBase::Dynamic;
    test = M2MBase::PUT_ALLOWED;
    set_operation(test);

    CHECK(test == this->operation());
}

void Test_M2MBase::test_base_type()
{
    CHECK(M2MBase::ObjectInstance == this->base_type());
}

void Test_M2MBase::test_set_interface_description()
{
    const char *test = "interface_description";
    set_interface_description(test);

    CHECK(test == this->interface_description());
}

void Test_M2MBase::test_uri_path()
{
    // Default value in ctor
    String test = "test";
    CHECK(test == uri_path());
}

#ifndef DISABLE_RESOURCE_TYPE
void Test_M2MBase::test_set_resource_type()
{
    const char *test = "resource_type";
    set_resource_type(test);

    CHECK(test == this->_sn_resource.dynamic_resource_params.resource_type_ptr);
}
#endif

void Test_M2MBase::test_set_coap_content_type()
{
    uint8_t test = 1;
    set_coap_content_type(test);

    CHECK(test == this->coap_content_type());
}

void Test_M2MBase::test_instance_id()
{
    // the instance id of 1 was given during construction, so test it
    CHECK(1 == this->instance_id());
}

void Test_M2MBase::test_set_observable()
{
    bool test = true;
    set_observable(test);

    CHECK(test == this->is_observable());
}

void Test_M2MBase::test_add_observation_level()
{
    Observer obs;
    this->_report_handler = new M2MReportHandler(obs);
    add_observation_level(M2MBase::R_Attribute);

    add_observation_level(M2MBase::O_Attribute);
}

void Test_M2MBase::test_remove_observation_level()
{
    Observer obs;
    this->_report_handler = new M2MReportHandler(obs);

    remove_observation_level(M2MBase::R_Attribute);

    remove_observation_level(M2MBase::O_Attribute);

    remove_observation_level(M2MBase::R_Attribute);

    remove_observation_level(M2MBase::OI_Attribute);
    remove_observation_level(M2MBase::OI_Attribute);
}

void Test_M2MBase::test_set_under_observation()
{
    bool test = true;
    set_under_observation(test,NULL);
    set_under_observation(test,obsHandler);

    set_under_observation(test,NULL);

    test = false;
    set_under_observation(test,NULL);

    test = false;
    set_under_observation(test,obsHandler);

    set_under_observation(test,obsHandler);
}

void Test_M2MBase::test_set_observation_token()
{
    String test = "token";
    Observer obs;
    this->_report_handler = new M2MReportHandler(obs);
    set_observation_token((const u_int8_t*)test.c_str(), (u_int8_t)test.size());
}

void Test_M2MBase::test_observation_level()
{
    Observer obs;
    this->_report_handler = new M2MReportHandler(obs);
    m2mreporthandler_stub::observation_level_value = M2MBase::OR_Attribute;
    CHECK(M2MBase::OR_Attribute == this->observation_level());
}

void Test_M2MBase::test_get_observation_token()
{
    u_int8_t* out_value = NULL;
    u_int32_t out_size = 0;

    Observer obs;
    this->_report_handler = new M2MReportHandler(obs);

    get_observation_token(out_value,out_size);

    CHECK(out_value == NULL);

    const u_int8_t* const_out_value = NULL;

    get_observation_token(const_out_value, out_size);

    CHECK(out_value == NULL);
}

void Test_M2MBase::test_mode()
{
    _sn_resource.dynamic_resource_params.mode = 1;
    CHECK(M2MBase::Dynamic == mode());
}

void Test_M2MBase::test_observation_number()
{
    // XXX: this can't be tested here anymore as the observation handler stuff is in inherited classes, not in M2MBase
    u_int8_t test = 1;

    CHECK(0 == observation_number());

    Observer obs;
    this->_report_handler = new M2MReportHandler(obs);
    m2mreporthandler_stub::int16_value = test;
    CHECK(test == observation_number());
}


void Test_M2MBase::test_name_id()
{
    CHECK(1 == name_id());
}

void Test_M2MBase::test_handle_observation_attribute()
{
    char *s = "wrong";
    bool ret = handle_observation_attribute(s);
    CHECK(ret == true);
    delete this->_report_handler;

    Observer obs;
    this->_report_handler = new M2MReportHandler(obs);

    m2mreporthandler_stub::bool_return = true;
    ret = handle_observation_attribute(s);
    CHECK(ret == true);

    ret = handle_observation_attribute(s);
    CHECK(ret == true);

    m2mreporthandler_stub::bool_return = false;
    ret = handle_observation_attribute(s);
    CHECK(ret == false);
}

void Test_M2MBase::test_observation_to_be_sent()
{
    // XXX: this can't be tested here anymore as the observation handler stuff is in inherited classes, not in M2MBase
#if 0
    Vector<uint16_t> list;
    observation_to_be_sent(list, observation_number());
    CHECK(obsHandler->visited == false);
    this->set_base_type(M2MBase::ObjectInstance);

    bool test = true;
    set_under_observation(test,obsHandler);
    observation_to_be_sent(list, observation_number());
    CHECK(obsHandler->visited == true);
#endif
}

void Test_M2MBase::test_handle_get_request()
{
    CHECK(this->handle_get_request(NULL,NULL,NULL) == NULL);
}

void Test_M2MBase::test_handle_put_request()
{
    bool execute = false;
    CHECK(this->handle_put_request(NULL,NULL,NULL, execute) == NULL);
}

void Test_M2MBase::test_handle_post_request()
{
    bool execute = false;
    CHECK(this->handle_post_request(NULL,NULL,NULL, execute) == NULL);
}

void Test_M2MBase::test_memory_alloc()
{
    CHECK(memory_alloc(0) == 0);
    uint8_t *ptr = 0;
    ptr = (uint8_t*)memory_alloc(sizeof(uint8_t));
    CHECK(ptr != NULL);
    memory_free(ptr);
}

void Test_M2MBase::test_memory_free()
{
    uint8_t *ptr = (uint8_t*)memory_alloc(sizeof(uint8_t));
    memory_free((void*)ptr);
    //memory leak test will fail, if there is a leak, so no need for CHECK
}

void Test_M2MBase::test_report_handler()
{
    CHECK(report_handler() == NULL);
}

void Test_M2MBase::test_id_number()
{
    char* path = (char*)malloc(3);
    strcpy(path, "10");
    M2MBase* b = new Test_M2MBase(10,
                 M2MBase::Static,
#ifndef DISABLE_RESOURCE_TYPE
                 "",
#endif
                path,
                false,
                false);

    CHECK(b->name_id() == 10);
    delete b;
}

void Test_M2MBase::test_set_register_uri()
{
    this->set_register_uri(false);
    CHECK(this->register_uri() == false);
}

void Test_M2MBase::test_set_max_age()
{
    this->set_max_age(10000);
    CHECK(this->max_age() == 10000);
}

void Test_M2MBase::test_is_under_observation()
{
    CHECK(false == is_under_observation());
    Observer obs;
    this->_report_handler = new M2MReportHandler(obs);
    m2mreporthandler_stub::bool_return = true;
    CHECK(true == is_under_observation());
}

void Test_M2MBase::test_value_updated_function()
{
    MyTest test;
    test.visited = false;

    CHECK(this->is_value_updated_function_set() == false);

    this->set_value_updated_function(value_updated_callback(&test,&MyTest::value_updated_function));
    this->execute_value_updated("test");
    CHECK(this->is_value_updated_function_set() == true);
    CHECK(test.visited == true);

    value_update_called = false;
    this->set_value_updated_function(&value_updated_function2);
    this->execute_value_updated("test");
    CHECK(value_update_called == true);
}

void Test_M2MBase::test_build_path()
{
    StringBuffer<MAX_PATH_SIZE_3> buffer1;
    CHECK(build_path(buffer1, 0, 1, 2));
    STRCMP_EQUAL(buffer1.c_str(), "0/1/2");

    StringBuffer<MAX_PATH_SIZE_3> buffer2;
    CHECK(build_path(buffer2, 0, 0, 0));
    STRCMP_EQUAL(buffer2.c_str(), "0/0/0");

    StringBuffer<MAX_PATH_SIZE_3> buffer3;
    CHECK(build_path(buffer3, 12345, 23456, 34567));
    STRCMP_EQUAL(buffer3.c_str(), "12345/23456/34567");

    StringBuffer<MAX_PATH_SIZE_3> buffer4;
    CHECK(build_path(buffer4, 0xffff, 0xffff, 0xffff));
    STRCMP_EQUAL(buffer4.c_str(), "65535/65535/65535");
}

void Test_M2MBase::test_resource_type()
{
    // Default value in ctor
    String test = "type";
    CHECK(test == resource_type());
}

void Test_M2MBase::test_get_nsdl_resource()
{
    const sn_nsdl_dynamic_resource_parameters_s &res = get_nsdl_resource();
    CHECK(&res == &_sn_resource.dynamic_resource_params);
}

void Test_M2MBase::test_create_path()
{
    char* path1 = (char*)malloc(5);
    strcpy(path1, "1");
    M2MObject* object = new M2MObject(1, path1);

    char* path2 = (char*)malloc(7);
    strcpy(path2, "1/0");
    m2mresource_stub::object_instance = new M2MObjectInstance(*object, 0, "type", path2);

    String path = "1/6";
    String res_path = "1/0/3";
    String res_path_instance = "1/0/8/9";

    char* result = create_path(*object, 6);
    CHECK(path == result);
    free(result);

    result = create_path(*m2mresource_stub::object_instance, 3);
    CHECK(res_path == result);
    free(result);

    char* path3 = (char*)malloc(9);
    strcpy(path3, "1/0/8");

    M2MResource* res = new M2MResource(*m2mresource_stub::object_instance,
                                       8,
                                       M2MBase::Dynamic,
                                       "type",
                                       M2MBase::INTEGER,
                                       false,
                                       path3);

    result = create_path(*res, 9);
    CHECK(res_path_instance == result);
    free(result);

    delete m2mresource_stub::object_instance;
    delete object;
    delete res;
}
void Test_M2MBase::test_create_path_max_size()
{
    const uint16_t max_id = 65535;
    char *obj_path = stringdup("65535");
    M2MObject *obj = new M2MObject(max_id, obj_path);
    char *inst_path = create_path(*obj, max_id);
    M2MObjectInstance *inst = new M2MObjectInstance(*obj, max_id, "type", inst_path);
    char *res_path = create_path(*inst, max_id);
    M2MResource *res = new M2MResource(*inst,
                                       max_id,
                                       M2MBase::Dynamic,
                                       "type",
                                       M2MBase::INTEGER,
                                       false,
                                       res_path);
    char *res_inst_path = create_path(*res, max_id);
    const char *max_path =  "65535/"\
                            "65535/"\
                            "65535/"\
                            "65535";
    STRCMP_EQUAL(max_path, res_inst_path);
    delete obj;
    delete inst;
    delete res;
    free(res_inst_path);
}

void Test_M2MBase::test_create_report_handler()
{
    M2MReportHandler* report_handler = create_report_handler();
    CHECK(report_handler != NULL);
}

void Test_M2MBase::test_validate_string_length()
{
    const char *test = "stringlengthabc";
    CHECK(validate_string_length(test, 1, 1) == false);
    CHECK(validate_string_length(test, 1, 20) == true);
    CHECK(validate_string_length(test, 1, 2) == false);
    CHECK(validate_string_length(test, 15, 15) == true);
    CHECK(validate_string_length(test, 16, 15) == false);
}

void Test_M2MBase::test_is_integer()
{
    CHECK(is_integer("") == false);
    CHECK(is_integer("a") == false);
    CHECK(is_integer("-") == false);
    CHECK(is_integer("-1a") == false);

    CHECK(is_integer("0") == true);
    CHECK(is_integer("10") == true);
    CHECK(is_integer("+10") == true);
    CHECK(is_integer("-10") == true);
}

void Test_M2MBase::test_alloc_copy()
{
    uint8_t* test_ptr = (uint8_t *)malloc(10);
    memset(test_ptr,'a', 10);
    uint8_t* result = alloc_copy(test_ptr, 10);
    CHECK(memcmp(test_ptr, result, 10) == 0);
    free(test_ptr);
    free(result);
}

void Test_M2MBase::test_alloc_string_copy()
{
    char* test = "testi";
    char* result = alloc_string_copy(test);
    STRCMP_EQUAL(test,result);
    free(result);
}

void Test_M2MBase::test_ctor()
{
    M2MBase* base = new Test_M2MBase(87, M2MBase::Dynamic, "res_type", M2MBase::stringdup("87"), false, true);

    STRCMP_EQUAL("87", base->uri_path());
    STRCMP_EQUAL("res_type", base->resource_type());
    delete base;
}

void Test_M2MBase::test_get_lwm2m_parameter()
{
    CHECK(&get_lwm2m_parameters() != NULL);
}

void Test_M2MBase::test_set_notification_msgid()
{
    this->set_notification_msgid(100);
    CHECK(this->get_notification_msgid() == 100);
}

void Test_M2MBase::test_get_notification_msgid()
{
    CHECK(this->get_notification_msgid() == 0);
    this->_sn_resource.dynamic_resource_params.msg_id = 50;
    CHECK(this->get_notification_msgid() == 50);
}
