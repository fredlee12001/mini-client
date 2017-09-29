# ----------------------------------------------------------------------------
# Copyright 2016-2017 ARM Ltd.
#  
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#  
#     http://www.apache.org/licenses/LICENSE-2.0
#  
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
# ----------------------------------------------------------------------------
import cbor2 as cbor
## pip install cbor2
import hashlib
#from test.test_binascii import BinASCIITest
import binascii

class Exportable(object):
    """Class which declares all members that can be exported

    Can be represent as a dictionary including all class members recursively
    """
    def get_exported_params(self):
        return self.__get_dict_exported_params(self.__dict__)

    def __get_obj_exported_params(self, obj):
        if type(obj) is list:
            return self.__get_list_exported_params(obj)
        elif type(obj) is dict:
            return self.__get_dict_exported_params(obj)

        try:
            return obj.get_exported_params()
        except AttributeError:
            return obj

    def __get_list_exported_params(self, lst):
        data = list()
        for x in lst:
            data.append(self.__get_obj_exported_params(x))
        return data

    def __get_dict_exported_params(self, dct):
        data = dict()
        for (k, v) in dct.items():
            data[k] = self.__get_obj_exported_params(v)
        return data


def to_cbor(obj):
    """Convert object to CBOR
    :param obj: Object to serialize to cbor
    :return: byte array representation of object in cbor
    """
    try:
        return cbor.dumps(obj.get_exported_params(), value_sharing=False)
    except AttributeError:
        return cbor.dumps(obj, value_sharing=False)



def to_cbor_byte_array(obj):
    """Get an hexadecimal string representation of conversion to CBOR
    :param obj: object to serialize to cbor
    :return: string representation of to_cbor(obj)
    """
    #return ''.join('{:02x}'.format(x) for x in to_cbor(obj))
    return binascii.hexlify(to_cbor(obj))


