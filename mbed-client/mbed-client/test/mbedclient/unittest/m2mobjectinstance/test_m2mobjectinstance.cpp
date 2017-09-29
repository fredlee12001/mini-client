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
#include "test_m2mobjectinstance.h"
#include "m2mresource_stub.h"
#include "common_stub.h"
#include "m2mresourceinstance_stub.h"
#include "m2mbase_stub.h"
#include "m2mtlvdeserializer_stub.h"
#include "m2mtlvserializer_stub.h"
#include "m2mreporthandler_stub.h"
#include "m2mobject_stub.h"
#include "m2mconstants.h"

class Handler : public M2MObservationHandler {

public:

    Handler(){}
    ~Handler(){}
    virtual void observation_to_be_sent(M2MBase *, uint16_t, const m2m::Vector<uint16_t>&, bool){
        visited = true;
    }
    void send_delayed_response(M2MBase *){}
    void resource_to_be_deleted(M2MBase *){visited=true;}
    void remove_object(M2MBase *){visited = true;}
    void value_updated(M2MBase *){visited = true;}

    void clear() {visited = false;}
    bool visited;
};

class TestReportObserver :  public M2MReportObserver{
public :
    TestReportObserver() {}
    ~TestReportObserver() {}
    virtual void observation_to_be_sent(const m2m::Vector<uint16_t>&, uint16_t, bool){ }
};

Test_M2MObjectInstance::Test_M2MObjectInstance()
{
    // force the tests use real path data, not a static value in stub
    m2mbase_stub::use_real_uri_path = true;

    handler = new Handler();
    object = new M2MObject(3, "3");
    object_instance = new M2MObjectInstance(*object, 0, "name", "0", "");
}

Test_M2MObjectInstance::~Test_M2MObjectInstance()
{
    delete object_instance;
    delete object;
    delete handler;
    m2mresource_stub::clear();
    m2mbase_stub::clear();
    m2mtlvdeserializer_stub::clear();
    m2mtlvserializer_stub::clear();
}

void Test_M2MObjectInstance::test_ctor()
{
    M2MObject* obj = new M2MObject(1, "1");

    M2MObjectInstance *instance = new M2MObjectInstance(*object, 2, "obj_inst_res_type", "1/2");

    STRCMP_EQUAL(instance->uri_path(), "1/2");
    STRCMP_EQUAL(instance->resource_type(), "obj_inst_res_type");

    delete instance;
    delete obj;
}

void Test_M2MObjectInstance::test_create_static_resource()
{
    u_int8_t value[] = {"value"};

    m2mbase_stub::bool_value = true;

    m2mbase_stub::uint16_value = COAP_CONTENT_OMA_TLV_TYPE;

    m2mbase_stub::path = "3/1";
    M2MResource * res = object_instance->create_static_resource(1, "type",M2MResourceInstance::STRING,value,(u_int32_t)sizeof(value),false);
    CHECK(res != NULL);

    res = object_instance->create_static_resource(2,"type",M2MResourceInstance::STRING,value,(u_int32_t)sizeof(value),true);

    CHECK(res != NULL);
    CHECK(2 == object_instance->total_resource_count());


    res = object_instance->create_static_resource(3, "type", M2MResourceInstance::STRING, value, sizeof(value));
    CHECK(res != NULL);

    // recreate with same input data, but different type, must fail
    res = object_instance->create_static_resource(3, "type", M2MResourceInstance::INTEGER, value, sizeof(value));
    CHECK(res == NULL);

    // now create resources with each type
    res = object_instance->create_static_resource(4, "type",M2MResourceInstance::INTEGER, value, sizeof(value));
    CHECK(res != NULL);

    res = object_instance->create_static_resource(5, "", M2MResourceInstance::BOOLEAN, value, sizeof(value));
    CHECK(res != NULL);

    res = object_instance->create_static_resource(6, "",M2MResourceInstance::FLOAT, value, sizeof(value));
    CHECK(res != NULL);

    res = object_instance->create_static_resource(7, "", M2MResourceInstance::OBJLINK, value, sizeof(value));
    CHECK(res != NULL);

    res = object_instance->create_static_resource(8, "",M2MResourceInstance::OPAQUE, value, sizeof(value));
    CHECK(res != NULL);

    m2mbase_stub::bool_value = true;
    res = object_instance->create_static_resource(9, "", M2MResourceInstance::TIME, value, sizeof(value));
    CHECK(res != NULL);

    // Note: since the create_static_resource(&params3) line created a resource with
    // a lwm2m_parameters from stack, it must be freed before running out of this method.
    object_instance->remove_resource(3);
}

void Test_M2MObjectInstance::test_create_static_resource_instance()
{
    m2mbase_stub::string_value = "name";
    u_int8_t value[] = {"value"};

    m2mbase_stub::name_id_value = 1; // hardwired resource-id, used from stub
    m2mbase_stub::bool_value = true;
    m2mresource_stub::bool_value = true;
    m2mbase_stub::uint16_value = COAP_CONTENT_OMA_TLV_TYPE;

    m2mbase_stub::path = "3/1";

    M2MResourceInstance *ins0 = object_instance->create_static_resource_instance(1,"type",
                                                                M2MResourceInstance::STRING,
                                                                value,(u_int32_t)sizeof(value),
                                                                0);

    CHECK(ins0 != NULL);
    CHECK(1 == object_instance->total_resource_count());

    M2MResourceInstance *ins1 = object_instance->create_static_resource_instance(1,"type",
                                                  M2MResourceInstance::STRING,
                                                  value,(u_int32_t)sizeof(value),
                                                  1);

    CHECK(ins1 != NULL);
    CHECK(1 == object_instance->total_resource_count());

    // objects can not deleted while they are in use and referenced by a resource in object instance
    delete ins0;
    delete ins1;
}

void Test_M2MObjectInstance::test_create_dynamic_resource_instance()
{
    m2mbase_stub::name_id_value = 1;

    m2mresource_stub::bool_value = true;
    m2mbase_stub::uint16_value = COAP_CONTENT_OMA_TLV_TYPE;
    m2mbase_stub::bool_value = true;
    m2mbase_stub::path = "3/1";
    M2MResourceInstance *ins0 = object_instance->create_dynamic_resource_instance(1,"type",
                                                                         M2MResourceInstance::STRING,
                                                                         false,0);
    CHECK(ins0 != NULL);
    CHECK(1 == object_instance->total_resource_count());


    M2MResourceInstance *ins1 = object_instance->create_dynamic_resource_instance(1,"type",
                                                   M2MResourceInstance::STRING,
                                                   false,1);
    CHECK(ins1 != NULL);
    CHECK(1 == object_instance->total_resource_count());

    delete ins0;
    delete ins1;
}

void Test_M2MObjectInstance::test_create_dynamic_resource()
{
    m2mbase_stub::name_id_value = 0;
    m2mbase_stub::uint16_value = COAP_CONTENT_OMA_TLV_TYPE;
    m2mbase_stub::bool_value = true;
    M2MResource * res = object_instance->create_dynamic_resource(1,"type",M2MResourceInstance::STRING,false,false);
    CHECK(res != NULL);
    CHECK(1 == object_instance->total_resource_count());

    res = object_instance->create_dynamic_resource(2,"type",M2MResourceInstance::STRING,false,true);
    CHECK(res != NULL);
    CHECK(2 == object_instance->total_resource_count());

    M2MResource * res1 = object_instance->create_dynamic_resource(3,"type",M2MResourceInstance::STRING,false,false);
    CHECK(res1 != NULL);
    CHECK(3 == object_instance->total_resource_count());

    m2mbase_stub::name_id_value = 2;
    m2mbase_stub::bool_value = false;
    M2MResource * res2 = object_instance->create_dynamic_resource(2,"type",M2MResourceInstance::STRING,false,false);
    CHECK(res2 == NULL);
}

void Test_M2MObjectInstance::test_remove_resource()
{
    const uint16_t test_resource_id = 42;

    CHECK(object_instance->remove_resource(test_resource_id) == false);
    M2MResource *res = new M2MResource(*object_instance, test_resource_id, M2MBase::Dynamic, "type",M2MBase::STRING,false, "name");
    object_instance->add_resource(res);

    m2mbase_stub::name_id_value = 42;
    m2mbase_stub::int_value = 0;
    m2mbase_stub::void_value = malloc(20);

    m2mresource_stub::bool_value = true;
    CHECK(true == object_instance->remove_resource(test_resource_id));
    CHECK(0 == object_instance->total_resource_count());
    CHECK(NULL == object_instance->resources());

    free(m2mbase_stub::void_value);
}

void Test_M2MObjectInstance::test_remove_resource_instance()
{
    CHECK(false == object_instance->remove_resource_instance(1,2));

    m2mbase_stub::path = "3/1";
    M2MResource *res = new M2MResource(*object_instance, 1, M2MBase::Dynamic, "type",M2MBase::STRING,false, "name");
    object_instance->add_resource(res);

    m2mbase_stub::name_id_value = 1;
    m2mbase_stub::int_value = 0;
    m2mbase_stub::void_value = malloc(20);

    m2mresource_stub::bool_value = true;
    m2mbase_stub::path = "3/1/0";
    M2MResourceInstance *ins = new M2MResourceInstance(*res, 0,M2MBase::Dynamic, "type",M2MBase::STRING,"name",false, false);

    res->add_resource_instance(ins);

    CHECK(true == object_instance->remove_resource_instance(1,0));
    CHECK(0 == object_instance->total_resource_count());

    free(m2mbase_stub::void_value);

    // the ins -object is deleted by remove_resource_instance()
}


void Test_M2MObjectInstance::test_resource()
{
    M2MResource *res = new M2MResource(*object_instance, 1, M2MBase::Dynamic, "type",M2MBase::STRING,false, "name");
    object_instance->add_resource(res);

    m2mbase_stub::name_id_value = 1;
    m2mbase_stub::int_value = 0;

    M2MResource *result = object_instance->resource(1);
    CHECK(result != NULL);

    res = new M2MResource(*object_instance, 1, M2MBase::Dynamic, "type",M2MBase::STRING,false, "name");
    object_instance->add_resource(res);

    result = object_instance->resource(1);
    CHECK(result != NULL);
}

void Test_M2MObjectInstance::test_resources()
{
    M2MResource *res = new M2MResource(*object_instance, 1, M2MBase::Dynamic, "type",M2MBase::STRING,false, "name");
    m2mbase_stub::string_value = "name";
    object_instance->add_resource(res);

    res = new M2MResource(*object_instance, 2, M2MBase::Dynamic, "type",M2MBase::STRING,false, "name");
    object_instance->add_resource(res);

    CHECK(2 == object_instance->total_resource_count());
}

void Test_M2MObjectInstance::test_resource_count()
{
    // XXX: this test creates two resources with same id, and adds them to list. Why?

    M2MResource *res = new M2MResource(*object_instance, 1,M2MBase::Dynamic, "type",M2MBase::STRING,false, "name");
    object_instance->add_resource(res);

    res = new M2MResource(*object_instance, 1, M2MBase::Dynamic, "type",M2MBase::STRING,false, "name");
    object_instance->add_resource(res);

    res = new M2MResource(*object_instance, 3, M2MBase::Dynamic, "type",M2MBase::STRING,false, "name");
    object_instance->add_resource(res);

    m2mbase_stub::int_value = 0;
    m2mresource_stub::bool_value = true;
    m2mresource_stub::int_value = 1;

    // there are two resources with id 1
    CHECK(2 == object_instance->resource_count(1));

    // and one resource by id 3
    CHECK(1 == object_instance->resource_count(3));

    // search by non-existing id
    CHECK(0 == object_instance->resource_count(0));
}

void Test_M2MObjectInstance::test_total_resource_count()
{
    M2MResource *res = new M2MResource(*object_instance, 1, M2MBase::Dynamic, "type",M2MBase::STRING,false, "name");
    m2mbase_stub::string_value = "name";
    object_instance->add_resource(res);

    res = new M2MResource(*object_instance, 2, M2MBase::Dynamic, "type",M2MBase::STRING,false, "name");
    object_instance->add_resource(res);

    m2mresource_stub::bool_value = true;
    m2mresource_stub::int_value = 1;

    CHECK(2 == object_instance->resource_count());

    m2mresource_stub::bool_value = false;

    CHECK(2 == object_instance->resource_count());
}

void Test_M2MObjectInstance::test_base_type()
{
    m2mbase_stub::base_type = M2MBase::ObjectInstance;
    CHECK(M2MBase::ObjectInstance == object_instance->base_type());
}

void Test_M2MObjectInstance::test_handle_get_request()
{
    M2MResource *res = new M2MResource(*object_instance, 1,M2MBase::Dynamic, "type1",M2MBase::STRING,false, "name1");
    object_instance->add_resource(res);
    m2mbase_stub::string_value = "name1";
    uint8_t value[] = {"name"};
    sn_coap_hdr_s *coap_header = (sn_coap_hdr_s *)malloc(sizeof(sn_coap_hdr_s));
    memset(coap_header, 0, sizeof(sn_coap_hdr_s));

    coap_header->uri_path_ptr = value;
    coap_header->uri_path_len = sizeof(value);

    coap_header->msg_code = COAP_MSG_CODE_REQUEST_GET;

    common_stub::int_value = 0;

    m2mbase_stub::operation = M2MBase::GET_ALLOWED;
    m2mbase_stub::uint16_value = 200;

    common_stub::coap_header = (sn_coap_hdr_ *)malloc(sizeof(sn_coap_hdr_));
    memset(common_stub::coap_header,0,sizeof(sn_coap_hdr_));

    m2mtlvserializer_stub::uint8_value = (uint8_t*)malloc(1);

    coap_header->token_ptr = (uint8_t*)malloc(sizeof(value));
    memcpy(coap_header->token_ptr, value, sizeof(value));

    coap_header->options_list_ptr = (sn_coap_options_list_s*)malloc(sizeof(sn_coap_options_list_s));
    coap_header->options_list_ptr->observe = 0;
    coap_header->options_list_ptr->accept = sn_coap_content_format_e(110);

    CHECK(object_instance->handle_get_request(NULL,coap_header,handler) != NULL);

    if(common_stub::coap_header->options_list_ptr) {
        free(common_stub::coap_header->options_list_ptr);
        common_stub::coap_header->options_list_ptr = NULL;
    }

    // Not OMA TLV or JSON
    m2mbase_stub::uint16_value = 110;
    CHECK(object_instance->handle_get_request(NULL,coap_header,handler) != NULL);

    if(common_stub::coap_header->options_list_ptr) {
        free(common_stub::coap_header->options_list_ptr);
        common_stub::coap_header->options_list_ptr = NULL;
    }

    // Content type set CT_NONE
    common_stub::coap_header->options_list_ptr = (sn_coap_options_list_s*)malloc(sizeof(sn_coap_options_list_s));
    m2mbase_stub::uint16_value = COAP_CONTENT_OMA_TLV_TYPE;
    coap_header->options_list_ptr->accept = sn_coap_content_format_e(-1);
    CHECK(object_instance->handle_get_request(NULL,coap_header,handler) != NULL);

    common_stub::coap_header->options_list_ptr->accept = sn_coap_content_format_e(-1); // CT_NONE
    m2mbase_stub::uint16_value = 100;
    coap_header->options_list_ptr->accept = sn_coap_content_format_e(-1);
    CHECK(object_instance->handle_get_request(NULL,coap_header,handler) != NULL);

    // OMA TLV
    m2mbase_stub::uint16_value = COAP_CONTENT_OMA_TLV_TYPE;
    CHECK(object_instance->handle_get_request(NULL,coap_header,handler) != NULL);

    // COAP_MSG_CODE_RESPONSE_UNSUPPORTED_CONTENT_FORMAT
    free(m2mtlvserializer_stub::uint8_value);
    m2mtlvserializer_stub::uint8_value = NULL;
    m2mbase_stub::uint16_value = COAP_CONTENT_OMA_TLV_TYPE;
    CHECK(object_instance->handle_get_request(NULL,coap_header,handler) != NULL);

    m2mtlvserializer_stub::uint8_value = (uint8_t*)malloc(1);

    // OMA JSON
    m2mbase_stub::uint16_value = 100;
    CHECK(object_instance->handle_get_request(NULL,coap_header,handler) != NULL);

    coap_header->options_list_ptr->observe = 0;
    m2mbase_stub::uint16_value = 0x1c1c;
    m2mbase_stub::uint16_value = COAP_CONTENT_OMA_TLV_TYPE;
    m2mbase_stub::bool_value = true;

    CHECK(object_instance->handle_get_request(NULL,coap_header,handler) != NULL);

    m2mbase_stub::uint16_value = 10;

    CHECK(object_instance->handle_get_request(NULL,coap_header,handler) != NULL);
    // Not observable
    m2mbase_stub::bool_value = false;
    CHECK(object_instance->handle_get_request(NULL,coap_header,handler) != NULL);

    m2mbase_stub::bool_value = true;
    coap_header->options_list_ptr->observe = 0;

    CHECK(object_instance->handle_get_request(NULL,coap_header,handler) != NULL);
    coap_header->options_list_ptr->observe = 1;
    m2mbase_stub::uint16_value = COAP_CONTENT_OMA_TLV_TYPE;
    CHECK(object_instance->handle_get_request(NULL,coap_header,handler) != NULL);
    m2mbase_stub::operation = M2MBase::NOT_ALLOWED;
    CHECK(object_instance->handle_get_request(NULL,coap_header,handler) != NULL);

    CHECK(object_instance->handle_get_request(NULL,NULL,handler) != NULL);

    if(coap_header->token_ptr) {
        free(coap_header->token_ptr);
        coap_header->token_ptr = NULL;
    }
    if(coap_header->options_list_ptr) {
        free(coap_header->options_list_ptr);
        coap_header->options_list_ptr = NULL;
    }

    if(common_stub::coap_header){
        if(common_stub::coap_header->options_list_ptr) {
            free(common_stub::coap_header->options_list_ptr);
            common_stub::coap_header->options_list_ptr = NULL;
        }
        free(common_stub::coap_header);
        common_stub::coap_header = NULL;
    }
    free(coap_header);
    coap_header = NULL;

    if(m2mtlvserializer_stub::uint8_value) {
        free(m2mtlvserializer_stub::uint8_value);
    }
    m2mtlvserializer_stub::clear();
    common_stub::clear();
}

void Test_M2MObjectInstance::test_handle_put_request()
{
    uint8_t value[] = {"name"};
    bool execute_value_updated = false;
    sn_coap_hdr_s *coap_header = (sn_coap_hdr_s *)malloc(sizeof(sn_coap_hdr_s));
    memset(coap_header, 0, sizeof(sn_coap_hdr_s));

    coap_header->uri_path_ptr = value;
    coap_header->uri_path_len = sizeof(value);

    coap_header->msg_code = COAP_MSG_CODE_REQUEST_PUT;

    common_stub::int_value = 0;
    m2mbase_stub::string_value = "name";

    m2mbase_stub::operation = M2MBase::PUT_ALLOWED;
    m2mbase_stub::uint16_value = 200;

    common_stub::coap_header = (sn_coap_hdr_ *)malloc(sizeof(sn_coap_hdr_));
    memset(common_stub::coap_header,0,sizeof(sn_coap_hdr_));

    coap_header->payload_ptr = (uint8_t*)malloc(1);
    coap_header->content_format = sn_coap_content_format_e(-1);
    sn_coap_hdr_s *coap_response = NULL;
    m2mbase_stub::uint16_value = COAP_CONTENT_OMA_TLV_TYPE;
    coap_response = object_instance->handle_put_request(NULL,coap_header,handler,execute_value_updated);
    CHECK( coap_response != NULL);

    m2mbase_stub::uint16_value = 0;

    coap_header->options_list_ptr = (sn_coap_options_list_s*)malloc(sizeof(sn_coap_options_list_s));
    coap_header->options_list_ptr->uri_query_ptr = value;
    coap_header->options_list_ptr->uri_query_len = sizeof(value);

    coap_header->content_format = sn_coap_content_format_e(COAP_CONTENT_OMA_TLV_TYPE);
    m2mtlvdeserializer_stub::bool_value = true;
    m2mtlvdeserializer_stub::error = M2MTLVDeserializer::None;

    m2mbase_stub::bool_value = false;


    coap_response = object_instance->handle_put_request(NULL,coap_header,handler,execute_value_updated);
    CHECK( coap_response != NULL);

    m2mbase_stub::bool_value = true;

    coap_response = object_instance->handle_put_request(NULL,coap_header,handler,execute_value_updated);
    CHECK( coap_response != NULL);

    free(coap_header->options_list_ptr);
    coap_header->options_list_ptr = NULL;

    m2mbase_stub::bool_value = false;

    coap_response = object_instance->handle_put_request(NULL,coap_header,handler,execute_value_updated);
    CHECK( coap_response != NULL);

    m2mtlvdeserializer_stub::error = M2MTLVDeserializer::NotFound;

    m2mbase_stub::bool_value = false;


    coap_response = object_instance->handle_put_request(NULL,coap_header,handler,execute_value_updated);
    CHECK( coap_response != NULL);
    CHECK(coap_response->msg_code == COAP_MSG_CODE_RESPONSE_NOT_FOUND);

    m2mtlvdeserializer_stub::error = M2MTLVDeserializer::NotValid;

    m2mbase_stub::bool_value = false;


    coap_response = object_instance->handle_put_request(NULL,coap_header,handler,execute_value_updated);
    CHECK( coap_response != NULL);
    CHECK(coap_response->msg_code == COAP_MSG_CODE_RESPONSE_BAD_REQUEST);

    m2mtlvdeserializer_stub::error = M2MTLVDeserializer::NotAllowed;

    m2mbase_stub::bool_value = false;

    coap_response = object_instance->handle_put_request(NULL,coap_header,handler,execute_value_updated);
    CHECK( coap_response != NULL);
    CHECK(coap_response->msg_code == COAP_MSG_CODE_RESPONSE_METHOD_NOT_ALLOWED);

    m2mtlvdeserializer_stub::error = M2MTLVDeserializer::OutOfMemory;

    coap_response = object_instance->handle_put_request(NULL,coap_header,handler,execute_value_updated);
    CHECK( coap_response != NULL);
    CHECK(coap_response->msg_code == COAP_MSG_CODE_RESPONSE_REQUEST_ENTITY_TOO_LARGE);

    m2mtlvdeserializer_stub::bool_value = false;

    coap_response = object_instance->handle_put_request(NULL,coap_header,handler,execute_value_updated);
    CHECK( coap_response != NULL);

    coap_header->content_format = sn_coap_content_format_e(100);

    coap_response = object_instance->handle_put_request(NULL,coap_header,handler,execute_value_updated);
    CHECK( coap_response != NULL);

    free(coap_header->payload_ptr);
    coap_header->payload_ptr = NULL;

    m2mbase_stub::bool_value = true;

    coap_response = object_instance->handle_put_request(NULL,coap_header,handler,execute_value_updated);
    CHECK( coap_response != NULL);

    m2mbase_stub::bool_value = false;

    coap_response = object_instance->handle_put_request(NULL,coap_header,handler,execute_value_updated);
    CHECK( coap_response != NULL);

    m2mbase_stub::operation = M2MBase::NOT_ALLOWED;

    coap_response = object_instance->handle_put_request(NULL,coap_header,handler,execute_value_updated);
    CHECK( coap_response != NULL);


    coap_response = object_instance->handle_put_request(NULL,NULL,handler,execute_value_updated);

    CHECK( coap_response != NULL);

    //free(coap_header->options_list_ptr);
    free(common_stub::coap_header);
    free(coap_header);

    m2mtlvdeserializer_stub::clear();
    common_stub::clear();
    m2mbase_stub::clear();
}

void Test_M2MObjectInstance::test_handle_post_request()
{
    uint8_t value[] = {"name"};
    bool execute_value_updated = false;
    sn_coap_hdr_s *coap_header = (sn_coap_hdr_s *)malloc(sizeof(sn_coap_hdr_s));
    memset(coap_header, 0, sizeof(sn_coap_hdr_s));

    coap_header->uri_path_ptr = value;
    coap_header->uri_path_len = sizeof(value);

    coap_header->msg_code = COAP_MSG_CODE_REQUEST_POST;

    common_stub::int_value = 0;
    m2mbase_stub::string_value = "name";

    m2mbase_stub::operation = M2MBase::POST_ALLOWED;
    m2mbase_stub::uint16_value = 200;

    common_stub::coap_header = (sn_coap_hdr_ *)malloc(sizeof(sn_coap_hdr_));
    memset(common_stub::coap_header,0,sizeof(sn_coap_hdr_));

    m2mbase_stub::bool_value = false;

    sn_coap_hdr_s * coap_response = NULL;
    m2mbase_stub::uint16_value = COAP_CONTENT_OMA_TLV_TYPE;

    common_stub::coap_header->options_list_ptr = (sn_coap_options_list_s*)malloc(sizeof(sn_coap_options_list_s));
    coap_header->content_format = sn_coap_content_format_e(-1);
    coap_response = object_instance->handle_post_request(NULL,coap_header,handler,execute_value_updated);
    CHECK( coap_response != NULL);
    if(coap_response) {
        /*if (coap_response->options_list_ptr) {
            if (coap_response->options_list_ptr->location_path_ptr) {
                free(coap_response->options_list_ptr->location_path_ptr);
                coap_response->options_list_ptr->location_path_ptr = NULL;
            }
            free(coap_response->options_list_ptr);
            coap_response->options_list_ptr = NULL;
        }*/
    }

    m2mbase_stub::uint16_value = 100;

    coap_response = object_instance->handle_post_request(NULL,coap_header,handler,execute_value_updated);
    CHECK( coap_response != NULL);
    if(coap_response) {
        if (coap_response->options_list_ptr) {
            if (coap_response->options_list_ptr->location_path_ptr) {
                free(coap_response->options_list_ptr->location_path_ptr);
                coap_response->options_list_ptr->location_path_ptr = NULL;
            }
            free(coap_response->options_list_ptr);
            coap_response->options_list_ptr = NULL;
        }
    }
    coap_header->payload_ptr = (uint8_t*)malloc(1);

    coap_response = object_instance->handle_post_request(NULL,coap_header,handler,execute_value_updated);
    CHECK( coap_response != NULL);
    if(coap_response) {
        if (coap_response->options_list_ptr) {
            if (coap_response->options_list_ptr->location_path_ptr) {
                free(coap_response->options_list_ptr->location_path_ptr);
                coap_response->options_list_ptr->location_path_ptr = NULL;
            }
            free(coap_response->options_list_ptr);
            coap_response->options_list_ptr = NULL;
        }
    }
    m2mbase_stub::uint16_value = COAP_CONTENT_OMA_TLV_TYPE;

    coap_response = object_instance->handle_post_request(NULL,coap_header,handler,execute_value_updated);
    CHECK( coap_response != NULL);
    if(coap_response) {
        if (coap_response->options_list_ptr) {
            if (coap_response->options_list_ptr->location_path_ptr) {
                free(coap_response->options_list_ptr->location_path_ptr);
                coap_response->options_list_ptr->location_path_ptr = NULL;
            }
            free(coap_response->options_list_ptr);
            coap_response->options_list_ptr = NULL;
        }
    }

    m2mbase_stub::uint16_value = 0;

    coap_header->options_list_ptr = (sn_coap_options_list_s*)malloc(sizeof(sn_coap_options_list_s));
    coap_header->options_list_ptr->uri_query_ptr = value;
    coap_header->options_list_ptr->uri_query_len = sizeof(value);

    coap_header->content_format = sn_coap_content_format_e(COAP_CONTENT_OMA_TLV_TYPE);
    m2mtlvdeserializer_stub::is_object_bool_value = true;
    m2mtlvdeserializer_stub::bool_value = false;
    m2mbase_stub::bool_value = false;

    m2mtlvdeserializer_stub::error = M2MTLVDeserializer::None;
    coap_response = object_instance->handle_post_request(NULL,coap_header,handler,execute_value_updated);

    CHECK( coap_response != NULL);
    if(coap_response) {
        if (coap_response->options_list_ptr) {
            if (coap_response->options_list_ptr->location_path_ptr) {
                free(coap_response->options_list_ptr->location_path_ptr);
                coap_response->options_list_ptr->location_path_ptr = NULL;
            }
            free(coap_response->options_list_ptr);
            coap_response->options_list_ptr = NULL;
        }
    }

    m2mbase_stub::operation = M2MBase::POST_ALLOWED;
    m2mtlvdeserializer_stub::is_object_bool_value = true;
    m2mtlvdeserializer_stub::bool_value = false;
    m2mbase_stub::bool_value = false;

    m2mtlvdeserializer_stub::error = M2MTLVDeserializer::NotAllowed;

    coap_response = object_instance->handle_post_request(NULL,coap_header,handler,execute_value_updated);

    CHECK( coap_response != NULL);
    CHECK(coap_response->msg_code == COAP_MSG_CODE_RESPONSE_METHOD_NOT_ALLOWED);
    if(coap_response) {
        if (coap_response->options_list_ptr) {
            if (coap_response->options_list_ptr->location_path_ptr) {
                free(coap_response->options_list_ptr->location_path_ptr);
                coap_response->options_list_ptr->location_path_ptr = NULL;
            }
            free(coap_response->options_list_ptr);
            coap_response->options_list_ptr = NULL;
        }
    }

    m2mtlvdeserializer_stub::error = M2MTLVDeserializer::NotValid;

    coap_response = object_instance->handle_post_request(NULL,coap_header,handler,execute_value_updated);

    CHECK( coap_response != NULL);
    CHECK(coap_response->msg_code == COAP_MSG_CODE_RESPONSE_BAD_REQUEST);
    if(coap_response) {
        if (coap_response->options_list_ptr) {
            if (coap_response->options_list_ptr->location_path_ptr) {
                free(coap_response->options_list_ptr->location_path_ptr);
                coap_response->options_list_ptr->location_path_ptr = NULL;
            }
            free(coap_response->options_list_ptr);
            coap_response->options_list_ptr = NULL;
        }
    }


    m2mbase_stub::operation = M2MBase::POST_ALLOWED;
    m2mtlvdeserializer_stub::is_object_bool_value = false;
    m2mtlvdeserializer_stub::bool_value = true;
    m2mbase_stub::bool_value = false;
    m2mtlvdeserializer_stub::error = M2MTLVDeserializer::None;

    coap_response = object_instance->handle_post_request(NULL,coap_header,handler,execute_value_updated);

    CHECK( coap_response != NULL);
    if(coap_response) {
        if (coap_response->options_list_ptr) {
            if (coap_response->options_list_ptr->location_path_ptr) {
                free(coap_response->options_list_ptr->location_path_ptr);
                coap_response->options_list_ptr->location_path_ptr = NULL;
            }
            free(coap_response->options_list_ptr);
            coap_response->options_list_ptr = NULL;
        }
    }

    m2mbase_stub::operation = M2MBase::POST_ALLOWED;
    m2mtlvdeserializer_stub::is_object_bool_value = false;
    m2mtlvdeserializer_stub::bool_value = true;
    m2mbase_stub::bool_value = false;
    m2mtlvdeserializer_stub::error = M2MTLVDeserializer::NotFound;

    coap_response = object_instance->handle_post_request(NULL,coap_header,handler,execute_value_updated);

    CHECK( coap_response != NULL);
    CHECK(coap_response->msg_code == COAP_MSG_CODE_RESPONSE_NOT_FOUND);
    if(coap_response) {
        if (coap_response->options_list_ptr) {
            if (coap_response->options_list_ptr->location_path_ptr) {
                free(coap_response->options_list_ptr->location_path_ptr);
                coap_response->options_list_ptr->location_path_ptr = NULL;
            }
            free(coap_response->options_list_ptr);
            coap_response->options_list_ptr = NULL;
        }
    }


    m2mbase_stub::operation = M2MBase::POST_ALLOWED;
    m2mtlvdeserializer_stub::is_object_bool_value = false;
    m2mtlvdeserializer_stub::bool_value = true;
    m2mbase_stub::bool_value = false;
    m2mtlvdeserializer_stub::error = M2MTLVDeserializer::OutOfMemory;

    coap_response = object_instance->handle_post_request(NULL,coap_header,handler,execute_value_updated);

    CHECK( coap_response != NULL);
    CHECK(coap_response->msg_code == COAP_MSG_CODE_RESPONSE_REQUEST_ENTITY_TOO_LARGE);
    if(coap_response) {
        if (coap_response->options_list_ptr) {
            if (coap_response->options_list_ptr->location_path_ptr) {
                free(coap_response->options_list_ptr->location_path_ptr);
                coap_response->options_list_ptr->location_path_ptr = NULL;
            }
            free(coap_response->options_list_ptr);
            coap_response->options_list_ptr = NULL;
        }
    }

    m2mbase_stub::operation = M2MBase::POST_ALLOWED;
    m2mtlvdeserializer_stub::bool_value = false;

    coap_response = object_instance->handle_post_request(NULL,coap_header,handler,execute_value_updated);

    CHECK( coap_response != NULL);
    if(coap_response) {
        if (coap_response->options_list_ptr) {
            if (coap_response->options_list_ptr->location_path_ptr) {
                free(coap_response->options_list_ptr->location_path_ptr);
                coap_response->options_list_ptr->location_path_ptr = NULL;
            }
            free(coap_response->options_list_ptr);
            coap_response->options_list_ptr = NULL;
        }
    }


    coap_header->content_format = sn_coap_content_format_e(100);

    coap_response = object_instance->handle_post_request(NULL,coap_header,handler,execute_value_updated);

    CHECK( coap_response != NULL);
    if(coap_response) {
        if (coap_response->options_list_ptr) {
            if (coap_response->options_list_ptr->location_path_ptr) {
                free(coap_response->options_list_ptr->location_path_ptr);
                coap_response->options_list_ptr->location_path_ptr = NULL;
            }
            free(coap_response->options_list_ptr);
            coap_response->options_list_ptr = NULL;
        }
    }


    m2mbase_stub::bool_value = true;

    coap_response = object_instance->handle_post_request(NULL,coap_header,handler,execute_value_updated);

    CHECK( coap_response != NULL);
    if(coap_response) {
        if (coap_response->options_list_ptr) {
            if (coap_response->options_list_ptr->location_path_ptr) {
                free(coap_response->options_list_ptr->location_path_ptr);
                coap_response->options_list_ptr->location_path_ptr = NULL;
            }
            free(coap_response->options_list_ptr);
            coap_response->options_list_ptr = NULL;
        }
    }


    m2mbase_stub::operation = M2MBase::NOT_ALLOWED;

    coap_response = object_instance->handle_post_request(NULL,coap_header,handler,execute_value_updated);

    CHECK( coap_response != NULL);
    if(coap_response) {
        if (coap_response->options_list_ptr) {
            if (coap_response->options_list_ptr->location_path_ptr) {
                free(coap_response->options_list_ptr->location_path_ptr);
                coap_response->options_list_ptr->location_path_ptr = NULL;
            }
            free(coap_response->options_list_ptr);
            coap_response->options_list_ptr = NULL;
        }
    }

    coap_response = object_instance->handle_post_request(NULL,NULL,handler,execute_value_updated);

    CHECK( coap_response != NULL);
    if(coap_response) {
        if (coap_response->options_list_ptr) {
            if (coap_response->options_list_ptr->location_path_ptr) {
                free(coap_response->options_list_ptr->location_path_ptr);
                coap_response->options_list_ptr->location_path_ptr = NULL;
            }
            free(coap_response->options_list_ptr);
            coap_response->options_list_ptr = NULL;
        }
    }

    free(coap_header->options_list_ptr);
    free(coap_header->payload_ptr);
    free(common_stub::coap_header);
    free(coap_header);
    m2mtlvdeserializer_stub::clear();
    common_stub::clear();
    m2mbase_stub::clear();

}

void Test_M2MObjectInstance::test_notification_update()
{
    M2MBase::Observation obs_level = M2MBase::O_Attribute;

    object_instance->notification_update(obs_level);
    //CHECK(callback->visited == true);

    obs_level = M2MBase::OI_Attribute;

    TestReportObserver obs;
    m2mbase_stub::report = new M2MReportHandler(obs);
    m2mbase_stub::bool_value = true;

    object_instance->notification_update(obs_level);

    delete m2mbase_stub::report;
    m2mbase_stub::report = NULL;
}

void Test_M2MObjectInstance::test_set_observation_handler()
{
    object_instance->set_observation_handler(handler);
    CHECK(object_instance->observation_handler() == handler);
}
