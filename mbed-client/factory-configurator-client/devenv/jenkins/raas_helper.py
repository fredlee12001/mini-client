#!/usr/bin/env python
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

import logging
import threading
import time
from datetime import datetime

from raas_client import RaasClient, SerialParameters
from raas_client.resources import Resource
from typing import Callable, IO, AnyStr

__all__ = ["RaasConnection", "RaasDevice", "RaasProvider"]


class RaasConnection(object):
    test_start_pattern = '----< Test - Start >----'
    test_end_pattern = '----< Test - End >----'
    polling_interval_in_sec = 1

    def __init__(self, device, baud_rate, custom_data_cb, serial_log_fd):
        # type: (RaasDevice, int, Callable[[str], None], IO[AnyStr]) -> None
        self.logger = logging.getLogger('dut-raas-connection')
        self.device = device
        self.baud_rate = baud_rate
        self.is_test_started = False
        self.last_data_rx_time = datetime.now()
        self.test_finished_event = threading.Event()
        self.custom_data_cb = custom_data_cb
        self.serial_log_fd = serial_log_fd

    def __enter__(self):
        self.logger.debug('connecting to %s with buad_rate=%d', self.device.raas_resource.resource_id, self.baud_rate)
        self.device.raas_resource.openConnection(SerialParameters(baudrate=self.baud_rate))
        self.logger.info('connected to %s',  self.device.raas_resource.resource_id)

        # # flush the serial input
        # time.sleep(2)
        # data = self.device.raas_resource.read()
        # if data:
        #     self.logger.debug('flushed %d bytes from serial input', len(data))

        self.is_test_started = False
        self.test_finished_event.clear()
        self.device.raas_resource.set_data_callback(self.data_callback)
        return self

    def __exit__(self, exc_type, exc_val, exc_tb):
        self.test_finished_event.set()
        self.logger.debug('closing connection to %s', self.device.raas_resource.resource_id)
        self.device.raas_resource.closeConnection()
        self.logger.info('closed connection to %s', self.device.raas_resource.resource_id)
        self.device.raas_resource.set_data_callback(None)

    def data_callback(self):
        should_send_event = False
        self.logger.debug('in data_callback')
        self.last_data_rx_time = datetime.now()
        data = self.device.raas_resource.read()

        if self.test_start_pattern in data:
            self.logger.info('test start detected')
            self.is_test_started = True
            self.serial_log_fd.seek(0)
            self.serial_log_fd.truncate()
            # discard everything BEFORE the test start pattern
            data = data.split(self.test_start_pattern)[1]

        if self.is_test_started and self.test_end_pattern in data:
            self.logger.info('test end detected')
            self.is_test_started = False
            should_send_event = True
            # discard everything AFTER the test end pattern
            data = data.split(self.test_end_pattern)[0]

        self.serial_log_fd.write(data)

        if self.custom_data_cb:
            self.custom_data_cb(data)

        if should_send_event:
            self.test_finished_event.set()

    def wait(self, timeout=1000, serial_idle=20):
        # type: (int, int) -> None
        self.logger.info('waiting for %s', self.device.raas_resource.resource_id)
        start_time = datetime.now()
        while not self.test_finished_event.is_set():
            self.test_finished_event.wait(self.polling_interval_in_sec)
            if not self.test_finished_event.is_set():
                now = datetime.now()
                if (now - self.last_data_rx_time).total_seconds() > serial_idle:
                    self.logger.error('wait stop - device is idle for too long')
                    break
                if (now - start_time).total_seconds() > timeout:
                    self.logger.error('wait stop - waiting for completion for too long')
                    break

    def write_line(self, line):
        # type: (str) -> None
        self.device.raas_resource.writeline(line)


class RaasDevice(object):
    def __init__(self, raas_resource):
        # type: (Resource) -> None
        assert raas_resource
        self.raas_resource = raas_resource
        self.logger = logging.getLogger('dut-raas-device')

    def __enter__(self):
        # type: () -> RaasDevice
        self.logger.debug('allocated device %s', self.raas_resource.resource_id)
        return self

    def __exit__(self, exc_type, exc_val, exc_tb):
        self.logger.debug('releasing device %s', self.raas_resource.resource_id)
        self.raas_resource.release()
        self.logger.info('released device %s', self.raas_resource.resource_id)

    def open_connection(self, baud_rate, log_file_object, user_callback=None):
        # type: (int, IO[AnyStr], Callable[[str], None]) -> RaasConnection
        return RaasConnection(self, baud_rate=baud_rate, custom_data_cb=user_callback, serial_log_fd=log_file_object)

    def load_bin(self, bin_file_name):
        retry_count_max = 2
        retry_count = 0
        while retry_count < retry_count_max:
            retry_count += 1
            self.logger.debug('flashing %s', bin_file_name)
            success = self.raas_resource.flash(bin_file_name)
            if success:
                self.logger.info('flashed device successfully')
                break
            else:
                self.logger.warning('failed to flash binary. retry_count=%d', retry_count)

        if retry_count >= retry_count_max:
            raise Exception('Failed to flash after {} attempts'.format(retry_count_max))

    def reset(self):
        self.logger.info('reset device')
        self.raas_resource.reset()
        self.logger.debug('reset device - DONE')


class RaasProvider(object):
    def __init__(self, host, port, username, password):
        self.logger = logging.getLogger('raas-provider')
        self.host = host
        self.port = port
        self.username = username
        self.password = password

    def __enter__(self):
        self.logger.debug('connecting to %s:%d', self.host, self.port)
        self.client = RaasClient(
            host=self.host,
            port=self.port,
            user=self.username,
            passwd=self.password,
            # logger=logging.getLogger('raas-client')
        )
        self.logger.info('connected to %s:%d', self.host, self.port)
        return self

    def __exit__(self, exc_type, exc_val, exc_tb):
        self.logger.debug('disconnecting')
        self.client.disconnect()
        self.logger.info('disconnected')

    def allocate_device(self, platform_type, alloc_timeout):
        # type: (str, int) -> RaasDevice

        resource_list = self.client.allocate_multiple(
            [{'platform_name': platform_type}],
            timeout=alloc_timeout,
            local_allocation=False
        )
        assert resource_list, 'RAAS client returned an empty device list'
        return RaasDevice(next(iter(resource_list)))
