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

import argparse
import json
import logging
import os

import sys
from raas_helper import RaasProvider


class StoreRaasJsonConfig(argparse.Action):
    def __call__(self, parser, namespace, values, option_string=None):
        prospective_file = values
        try:
            with open(prospective_file, 'r') as fh:
                data = json.load(fh)
                setattr(namespace, 'host', data['host'])
                setattr(namespace, 'port', data['port'])
                setattr(namespace, 'username', data['username'])
                setattr(namespace, 'password', data['password'])
        except KeyError as e:
            raise argparse.ArgumentTypeError('malformed JSON file "{0}" - {1} not found'.format(prospective_file, e))
        except ValueError as e:
            raise argparse.ArgumentTypeError('malformed JSON file "{0}" - {1}'.format(prospective_file, e))
        except IOError as e:
            raise argparse.ArgumentTypeError('failed to read "{0}" - '.format(prospective_file, e))


def get_parser():
    parser = argparse.ArgumentParser(
        description='RAAS test runner',
        formatter_class = argparse.ArgumentDefaultsHelpFormatter
    )

    parser.add_argument(
        'image',
        help='binary image file to be flashed to a target device',
        type=argparse.FileType('r')
    )

    parser.add_argument(
        '-l',
        '--serial-log',
        metavar='FILE',
        help='file name to store log captured from the serial interface',
        type=argparse.FileType('wt')
    )

    raas_cfg = parser.add_argument_group('RAAS config')
    m_group = raas_cfg.add_mutually_exclusive_group(required=True)
    m_group.add_argument(
        '--host',
        metavar='IP',
        help='RAAS server host (IP or DNS name)',

    )
    m_group.add_argument(
        '--raas-json',
        metavar='FILE',
        action=StoreRaasJsonConfig,
        help='RAAS config JSON file with preset values. Take precedence over other RAAS config options'
    )

    raas_cfg.add_argument(
        '--port',
        default=8000,
        metavar='N',
        type=int,
        help='RAAS server network port',
    )

    raas_cfg.add_argument(
        '--username',
        metavar='NAME',
        default='user',
        help='RAAS server login username',
    )

    raas_cfg.add_argument(
        '--password',
        metavar='PWD',
        default='user',
        help='RAAS server login password',
    )

    dut_cfg = parser.add_argument_group('DUT config')
    dut_cfg.add_argument(
        '-p',
        '--platform-type',
        metavar='NAME',
        default='K64F',
        help='DUT platform type'
    )
    dut_cfg.add_argument(
        '-b',
        '--baud-rate',
        type=int,
        metavar='N',
        default=115200,
        help='DUT serial connection baud-rate'
    )

    parser.add_argument(
        '-v',
        '--verbose',
        action='store_true',
        help='set verbose mode'
    )

    parser.add_argument(
        '--no-color',
        action='store_true',
        help='disable color printing'
    )
    return parser


def main():
    parser = get_parser()
    args = parser.parse_args()
    alloc_timeout = 5000

    logging.basicConfig(
        level=logging.DEBUG if args.verbose else logging.INFO,
        format='%(asctime)s - %(name)s - %(levelname)s - %(message)s',
        stream=sys.stdout
    )

    if not args.no_color:
        logging.addLevelName(logging.WARNING, '\033[1;35m{}\033[0m'.format(logging.getLevelName(logging.WARNING)))
        logging.addLevelName(logging.ERROR, '\033[1;31m{}\033[0m'.format(logging.getLevelName(logging.ERROR)))
        logging.addLevelName(logging.INFO, '\033[1;36m{}\033[0m'.format(logging.getLevelName(logging.INFO)))

    if not args.serial_log:
        args.serial_log = open(os.path.splitext(args.image.name)[0] + '.log', 'wt')
        logging.getLogger('raas-test-runner').info('serial log file name %s', args.serial_log.name)

    with RaasProvider(args.host, args.port, args.username, args.password) as raas:
        with raas.allocate_device(platform_type=args.platform_type, alloc_timeout=alloc_timeout) as dut:
            dut.load_bin(args.image.name)
            with dut.open_connection(args.baud_rate, args.serial_log) as dut_connection:
                dut.reset()
                dut_connection.wait()

if __name__ == '__main__':
    main()