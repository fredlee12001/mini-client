/*
 * Copyright (c) 2015-2017 ARM Limited. All rights reserved.
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

#ifndef M2M_MAP_H
#define M2M_MAP_H

/*! \file m2mmap.h
* \brief A simple C++ Map implementation, used as replacement for std::map.
* NOTE! implemented as a vector-like class, so large maps will be very slow.
*/


namespace m2m
{

static std::string empty_key("");

/*
 *
 */
template <typename ObjectTemplate>
class KVPair
{
public:
    KVPair(std::string& first, ObjectTemplate& second) : _first(first) {

        _second = second;
    }

    KVPair() : _first(empty_key) { }

    KVPair(std::string& first) : _first(first) {}

    ObjectTemplate& second() {
        return _second;
    }

    std::string& first() {
        return _first;
    }

    const KVPair<ObjectTemplate> & operator=(const KVPair<ObjectTemplate> & rhs) {
        if(this != &rhs) {
            this->_first = rhs._first;
            this->_second = rhs._second;
        }
        return *this;
    }

    const KVPair<ObjectTemplate> & operator=(ObjectTemplate& rhs) { this->_second = rhs._second; return *this; }


    class iterator {

    public:
        // XXX: are all these really needed? at least make clear what their role is.
        iterator(KVPair<ObjectTemplate>* ptr) : _ptr(ptr) {}
        iterator(const iterator& iter) : _ptr(iter._ptr) {}
        iterator() : _ptr(this->_ptr) {}
        iterator& operator++() { ++_ptr; return *this; }
        iterator operator++(int) { iterator tmp(*this); operator++(); return tmp; }
        bool operator==(const iterator& rhs) const { return _ptr == rhs._ptr; }
        bool operator!=(const iterator& rhs) const { return _ptr != rhs._ptr; }
        ObjectTemplate& operator*() { return _ptr->second(); }
        const std::string& key() const { return _ptr->first(); }
        KVPair<ObjectTemplate>* ptr() { return _ptr; }
        iterator next() { return _ptr + 1; }

    private:
        KVPair<ObjectTemplate>* _ptr;
    };

private:
    // XXX: use our own String class or templated type as key
    std::string _first;
    ObjectTemplate _second;
};

template <typename ObjectTemplate>

class Map
{
  public:
    explicit Map( int init_size = MIN_CAPACITY)
            : _size(0),
              _capacity((init_size >= MIN_CAPACITY) ? init_size : MIN_CAPACITY) {
        _object_template = new KVPair<ObjectTemplate>[ _capacity ];
    }

    Map(const Map & rhs ): _object_template(0) {
        operator=(rhs);
    }
 
    ~Map() {
        delete [] _object_template;
    }

    const Map & operator=(const Map & rhs) {
        if(this != &rhs) {
            delete[] _object_template;
            _size = rhs.size();
            _capacity = rhs._capacity;

            _object_template = new KVPair<ObjectTemplate>[capacity()];
            for(int k = 0; k < size(); k++) {
                _object_template[k] = rhs._object_template[k];
            }
        }
        return *this;
    }

    void resize(int new_size) {
        if(new_size > _capacity) {
            reserve(new_size * 2 + 1);
        }
        _size = new_size;
    }

    void reserve(int new_capacity) {
        if(new_capacity < _size) {
            return;
        }
        KVPair<ObjectTemplate> *old_array = _object_template;

        _object_template = new KVPair<ObjectTemplate>[new_capacity];
        for(int k = 0; k < _size; k++) {
            _object_template[k] = old_array[k];
        }
        _capacity = new_capacity;
        delete [] old_array;
    }

    ObjectTemplate & operator[](int idx) {
        return _object_template[idx].second();
    }

    const ObjectTemplate& operator[](int idx) const {
        return _object_template[idx].second();
    }

    ObjectTemplate& operator[](std::string key) {

        for (iterator it = begin(); it != end(); it++) {
            if (it.key() == key) {
                return *it;
            }
        }

        // key not found; set up a new object with the key and return a reference to the value
        if(_size == _capacity) {
            reserve(2 * _capacity + 1);
        }
        _object_template[_size].first() = key;
        ObjectTemplate& ret_obj = _object_template[_size].second();
        _size++;
        return ret_obj;
    }

    // we provide a way to iterate through the map items, which are in reality stored
    // as an array anyway.
    typedef typename KVPair<ObjectTemplate>::iterator iterator;
    typedef const typename KVPair<ObjectTemplate>::iterator const_iterator;


    iterator find(const std::string& key) {
        iterator it = begin();
        for (; it != end(); it++) {
            if (it.key() == key) {
                return it;
            }
        }
        return end();
    }

    int count(const char* key) {
        const std::string& keystr(key);
        return count(keystr);
    }

    int count(const std::string& key) {
        int num_found = 0;
        for (iterator it = begin(); it != end(); it++) {
            if (it.key() == key) {
                num_found++;
            }
        }
        return num_found;
    }


    bool empty() const{
        return size() == 0;
    }

    int size() const {
        return _size;
    }

    int capacity() const {
        return _capacity;
    }

    int count() const {
        return _size;
    }

    void push_back(const ObjectTemplate& x) {
        if(_size == _capacity) {
            reserve(2 * _capacity + 1);
        }
        _object_template[_size] = x;
        _size++;
    }

    void pop_back() {
        _size--;
    }

    void clear() {
        _size = 0;
    }

    const ObjectTemplate& back() const {
        return _object_template[_size - 1];
    }



    iterator begin() {
        return &_object_template[0];
    }

    const_iterator begin() const {
        return &_object_template[0];
    }

    iterator end() {
        return &_object_template[_size];
    }

    const_iterator end() const {
        return &_object_template[_size];
    }

    void erase(std::string key) {

        // XXX: Ngh. Cleanup on aisle four, please.

        // figure out where our item is and then move the trailing items towards the front,
        // writing over the requested item.
        iterator it = begin();
        bool found = false;
        for (; it != end(); it++) {
            if (it.key() == key) {
                found = true;
                break;
            }
        }

        for (; it != end(); it++) {
            KVPair<ObjectTemplate>* curr = it.ptr();
            KVPair<ObjectTemplate>* next = it.next().ptr();
            *curr = *next;

        }
        if (found) {
            _size--;
        }
    }

    enum {
        MIN_CAPACITY = 1
    };

  private:
    int                 _size;
    int                 _capacity;
    KVPair<ObjectTemplate>*     _object_template;
};

} // namespace

#endif // M2M_MAP_H
