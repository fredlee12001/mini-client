#!/usr/bin/env python
import argparse
import json
import logging
import os
import re
import shutil
import subprocess
import sys
import textwrap

__project_dir = os.path.dirname(os.path.realpath(__file__))
logger = logging.getLogger('factory-client-env')

DO_RESET_BUILD_DIR = True


def source(script, update=True):
    """
    Emulate sourcing an environment script: http://pythonwise.blogspot.co.uk/2010/04/sourcing-shell-script.html
    """
    pipe = subprocess.Popen(". %s; env" % script, stdout=subprocess.PIPE, shell=True)
    data = pipe.communicate()[0]

    env = dict((line.split("=", 1) for line in data.splitlines()))
    if update:
        os.environ.update(env)

    return env


class ValidateYoctoEnvScript(argparse.Action):
    def __call__(self, parser, namespace, values, option_string=None):
        prospective_dir = values
        prospective_file = os.path.join(prospective_dir, 'environment-setup-cortexa8hf-neon-poky-linux-gnueabi')
        if not os.path.isdir(prospective_dir):
            raise argparse.ArgumentTypeError('dir: "{0}" is not a valid path'.format(prospective_dir))
        elif not os.access(prospective_dir, os.R_OK):
            raise argparse.ArgumentTypeError('dir: "{0}" is not a readable dir'.format(prospective_dir))
        if not os.path.isfile(prospective_file):
            raise argparse.ArgumentTypeError('file: "{0}" not found'.format(prospective_file))
        if not os.access(prospective_file, os.R_OK):
            raise argparse.ArgumentTypeError('file: "{0}" is not a readable file'.format(prospective_file))
        setattr(namespace, self.dest, os.path.abspath(prospective_dir))


class GitRepo(object):
    def __init__(self, proj_dir, repo_spec):
        self.url = repo_spec['git-url']

        self.custom_dir = repo_spec['custom-dir'] if 'custom-dir' in repo_spec else None
        self.sparse_checkout = repo_spec['git-sparse-checkout'] if 'git-sparse-checkout' in repo_spec else None
        self.skip_updates = 'skip-updates' in repo_spec

        if 'ref-spec' in repo_spec:
            ref_spec = repo_spec['ref-spec']
            branch_pattern = re.compile(r'refs/heads/(.*)')
            tag_pattern = re.compile(r'refs/tags/(.*)')
            branch_match = branch_pattern.match(ref_spec)
            tag_match = tag_pattern.match(ref_spec)
            if branch_match:
                self.branch = branch_match.group(1)
                self.tag = None
                self.hash = None
            elif tag_match:
                self.branch = tag_match.group(1)
                self.tag = tag_match.group(1)
                self.hash = None
            elif re.match(r'^([a-fA-F0-9]{40})$', ref_spec):
                self.branch = None
                self.tag = None
                self.hash = ref_spec
            else:
                raise Exception(
                    'Invalid ref-spec ({}). Allowed values are '
                    '/refs/heads/<branch> or /refs/tags/tag or commit hash SHA1'.format(ref_spec)
                )

        else:
            self.branch = 'master'
            self.hash = None

        if 'custom-dir' in repo_spec:
            self.dir = os.path.join(proj_dir, repo_spec['custom-dir'])
        else:
            self.dir = os.path.join(proj_dir, os.path.basename(self.url.rstrip('.git')))

        if 'ref-file-name' in repo_spec:
            self.ref_file_name = repo_spec['ref-file-name']
        else:
            self.ref_file_name = None


def check_call(cmd, cwd=None):
    logger.debug('>> %s', ' '.join(cmd))
    subprocess.check_call(cmd, cwd=cwd)


def check_output(cmd, cwd=None):
    logger.debug('>> %s', ' '.join(cmd))
    return subprocess.check_output(cmd, cwd=cwd)


def get_parser():
    parser = argparse.ArgumentParser(
        description='prepare eSFS build environment'
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

    parser.add_argument(
        '--configuration',
        type=argparse.FileType('rb'),
        default=open(os.path.join(__project_dir, 'env_setup.json'), 'rb'),
        help='load custom setup JSON file',
    )

    parser.add_argument(
        '--no-skip',
        action='store_true',
        help='do not skip update repositories with skip-updates flag',
    )
    # ------------------------------------------------------------------------------------------------------------------
    subparsers = parser.add_subparsers(dest='operation')
    # ------------------------------------------------------------------------------------------------------------------
    subparsers.add_parser(
        'linux-deploy',
        help='deploy PAL-Linux build environment',
    )
    # ------------------------------------------------------------------------------------------------------------------
    linux_generate_parser = subparsers.add_parser(
        'linux-generate',
        help='generate Linux build system files',
    )
    linux_generate_parser.add_argument(
        '-r',
        '--release',
        action='store_true',
        help='Set Release mode'
    )
    linux_generate_parser.add_argument(
        '--eclipse',
        action='store_true',
        help='generate Eclipse CDT 4.0 project files. If not specified Unix Makefiles will be generated'
    )
    # ------------------------------------------------------------------------------------------------------------------
    subparsers.add_parser(
        'linux-yocto-deploy',
        help='deploy PAL-Linux-Yocto build environment',
    )
    # ------------------------------------------------------------------------------------------------------------------
    linux_yocto_generate_parser = subparsers.add_parser(
        'linux-yocto-generate',
        help='generate Linux-Yocto build system files',
    )
    linux_yocto_generate_parser.add_argument(
        '-r',
        '--release',
        action='store_true',
        help='Set Release mode'
    )
    linux_yocto_generate_parser.add_argument(
        '--eclipse',
        action='store_true',
        help='generate Eclipse CDT 4.0 project files. If not specified Unix Makefiles will be generated'
    )
    linux_yocto_generate_parser.add_argument(
        '-e',
        '--environment-script-dir',
        action=ValidateYoctoEnvScript,
        metavar='DIR',
        required=True,
        help="specify directory containing Yocto environment setup script."
    )
    # ------------------------------------------------------------------------------------------------------------------
    subparsers.add_parser(
        'mbed-os-deploy',
        help='deploy mbed-os build environment',
    )
    # ------------------------------------------------------------------------------------------------------------------
    mbed_os_generate_parser = subparsers.add_parser(
        'mbed-os-generate',
        help='prepare the environment for mbed-os build',
    )

    mbed_os_generate_parser.add_argument(
        '-r',
        '--release',
        action='store_true',
        help='Set Release mode'
    )

    mbed_os_generate_parser.add_argument(
        '-t',
        '--toolchain',
        choices=['ARM', 'GCC_ARM'],
        required=True,
        help='Toolchain to be used'
    ),

    # execute "$ mbed target -S" to get full supported matrix
    mbed_os_generate_parser.add_argument(
        '-d',
        '--device',
        choices=['K64F'],
        # required=True,
        default='K64F',  # setting default instead of required flag since it is the only supported device
        help='target device'
    )

    mbed_os_generate_parser.add_argument(
        '--define-verbose',
        action='store_true',
        help='add -DVERBOSE to a compilation'
    )
    # ------------------------------------------------------------------------------------------------------------------
    mbed_os_ide_parser = subparsers.add_parser(
        'mbed-os-ide',
        help='generate IDE project',
    )

    # from mbed-os/tools/export/__init__.py
    mbed_os_ide_parser.add_argument(
        'ide',
        choices=['ds5_5', 'eclipse_gcc_arm', 'eclipse_iar', 'eclipse_armc5'],
        help='generate IDE project files'
    )

    # execute "$ mbed target -S" to get full supported matrix
    mbed_os_ide_parser.add_argument(
        '-d',
        '--device',
        choices=['K64F'],
        # required=True,
        default='K64F',  # setting default instead of required flag since it is the only supported device
        help='target device'
    )
    # ------------------------------------------------------------------------------------------------------------------
    subparsers.add_parser(
        'free-rtos-deploy',
        help='deploy PAL-FreeRTOS build environment',
    )

    free_rtos_generate_parser = subparsers.add_parser(
        'free-rtos-generate',
        help='generate PAL-FreeRTOS build system files',
    )
    free_rtos_generate_parser.add_argument(
        '-r',
        '--release',
        action='store_true',
        help='Set Release mode'
    )
    free_rtos_generate_parser.add_argument(
        '--eclipse',
        action='store_true',
        help='generate Eclipse CDT 4.0 project files. If not specified Unix Makefiles will be generated'
    )

    free_rtos_generate_parser.add_argument(
        '-t',
        '--toolchain',
        choices=['ARM', 'GCC_ARM'],
        required=True,
        help='Toolchain to be used'
    )
    # ------------------------------------------------------------------------------------------------------------------
    deep_clean_parser = subparsers.add_parser(
        'clean',
        help='clean all untracked files. Auto-generated *.lib, *.ref, CMake related files',
    )
    deep_clean_parser.add_argument(
        '--repos',
        action='store_true',
        help='also remove cloned Git repositories'
    )

    return parser


def is_git_pull_required(repo):
    """
    check if pull is required - in case sources are modified, pull will fail, so we check whether pull is
    required before failing the script
    http://stackoverflow.com/questions/3258243/check-if-pull-needed-in-git
    """
    logger.info('Check if git pull required for %s', os.path.basename(repo.dir))
    local_hash = check_output(['git', 'rev-parse', '@'], cwd=repo.dir)
    remote_hash = check_output(['git', 'rev-parse', '@{upstream}'], cwd=repo.dir)
    base_hash = check_output(['git', 'merge-base', '@', '@{upstream}'], cwd=repo.dir)
    if local_hash == remote_hash:
        return False
    elif local_hash == base_hash:
        return True
    elif remote_hash == base_hash:
        logger.warning('There are local commits - do not forget to push %s', os.path.basename(repo.dir))
        return False
    else:
        raise Exception('Unknown git workspace state %s', repo.dir)


def clone_repositories(project_dir, repositories_to_clone, force_repo_update):
    for repo in repositories_to_clone:
        if not os.path.isdir(repo.dir):  # need to clone the repo
            if repo.sparse_checkout:
                logger.info('Sparse Checkout for %s', repo.url)
                os.mkdir(repo.dir)
                check_call(['git', 'init'], cwd=repo.dir)
                check_call(['git', 'config', 'core.sparsecheckout', 'true'], cwd=repo.dir)
                with open(os.path.join(repo.dir, '.git/info/sparse-checkout'), 'wt') as fh:
                    for git_file in repo.sparse_checkout:
                        fh.write(git_file + '\n')
                check_call(['git', 'remote', 'add', '-f', 'origin', repo.url], cwd=repo.dir)
                branch = repo.branch if repo.branch else 'master'
                check_call(['git', 'pull', 'origin', branch], cwd=repo.dir)
                check_call(['git', 'branch', '--set-upstream-to', 'origin/' + branch], cwd=repo.dir)
                if repo.hash:
                    check_call(['git', 'reset', '--hard', repo.hash], cwd=repo.dir)
            else:
                logger.info('Cloning %s', repo.url)
                branch_args = []

                if repo.branch:
                    branch_args = ['-b', repo.branch]
                check_call(['git', 'clone'] + branch_args + [repo.url, repo.dir], project_dir)

                if repo.hash:
                    logger.info('Checkout %s#%s', repo.url, repo.hash)
                    check_call(['git', 'checkout', repo.hash], repo.dir)
        elif not force_repo_update and repo.skip_updates:
            logger.info('Skipping repository %s', os.path.basename(repo.dir))
            continue
        elif repo.tag:
            current_tags = check_output(['git', 'tag', '--points-at', 'HEAD'], cwd=repo.dir)
            if repo.tag not in current_tags:
                all_tags = check_output(['git', 'tag'], cwd=repo.dir)
                if repo.tag not in all_tags:
                    logger.info('Pull repository history - %s', os.path.basename(repo.dir))
                    check_call(['git', 'fetch', '--all'], cwd=repo.dir)
                check_call(['git', 'checkout', repo.branch], repo.dir)
        elif repo.branch:
            logger.info('Pull repository history - %s', os.path.basename(repo.dir))
            check_call(['git', 'fetch', '--all'], cwd=repo.dir)

            current_branch = check_output(['git', 'rev-parse', '--abbrev-ref', 'HEAD'], cwd=repo.dir).strip()
            if current_branch != repo.branch:
                logger.info('Current branch %s - checkout %s#%s', current_branch, repo.url, repo.branch)
                check_call(['git', 'checkout', repo.branch], repo.dir)
                logger.info('Pull latest revision  - %s', os.path.basename(repo.dir))
                check_call(['git', 'pull', '--rebase'], repo.dir)
            else:
                if is_git_pull_required(repo):
                    check_call(['git', 'pull', '--rebase'], repo.dir)
                else:
                    logger.info('Repository is up-to-date - %s', os.path.basename(repo.dir))

        elif repo.hash:
            current_hash = check_output(['git', 'rev-parse', 'HEAD'], cwd=repo.dir)
            if current_hash != repo.hash:
                logger.info('Checkout %s#%s', repo.url, repo.hash)
                check_call(['git', 'fetch', '--all'], cwd=repo.dir)
                check_call(['git', 'checkout', repo.hash], repo.dir)


def generate_refs(project_dir, files_to_generate):
    for repo in files_to_generate:
        assert repo.ref_file_name, 'malformed configuration - ref file name is missing'
        logger.info('Generating %s', repo.ref_file_name)
        if repo.hash:
            git_hash = repo.hash
        else:
            logger.info('Querying latest hash for %s@%s', repo.url, repo.branch)
            output = check_output(['git', 'ls-remote', repo.url])
            git_hash = None
            for line in output.split('\n'):
                _hash, reference = line.split()
                if reference.endswith('/' + repo.branch):
                    git_hash = _hash
                    logger.info('Found latest hash for %s@%s', repo.url, _hash)
                    break
            assert git_hash, 'Branch {b} not found in {r}'.format(b=repo.branch, r=repo.url)
        file_name = os.path.join(project_dir, repo.ref_file_name)
        file_dir = os.path.dirname(file_name)
        if not os.path.isdir(file_dir):
            os.makedirs(file_dir)
        with open(file_name, 'wt') as fh:
            fh.write(
                '{url}#{tag}\n'.format(
                    url=repo.url.replace("git@github.com:", "https://github.com/"),
                    tag=git_hash
                )
            )


def initial_deploy(project_dir, cfg, force_repo_update):
    logger.info('Deploy build environment at %s', project_dir)
    generate_refs(project_dir, cfg.files)
    clone_repositories(project_dir, cfg.repos, force_repo_update)


def pal_deploy(args, project_dir, target):

    verbose_arg = ['-v'] if args.verbose else []
    cmd = [
        'python',
        os.path.join(project_dir, 'pal-platform/pal-platform.py')
    ] + verbose_arg + [
        'deploy',
        '--target=' + target
    ]
    check_call(cmd, cwd=project_dir)


def pal_generate(args, project_dir, target, build_dir, extra_flags=None, system_defines_file=None):

    verbose_arg = ['-v'] if args.verbose else []
    cmd = [
        'python',
        os.path.join(project_dir, 'pal-platform/pal-platform.py')
    ] + verbose_arg + [
        'generate',
        '--target=' + target
    ]
    check_call(cmd, cwd=project_dir)

    toolchain_file = toolchain_cmake_mapper[args.toolchain]
    generator = 'Eclipse CDT4 - Unix Makefiles' if args.eclipse else 'Unix Makefiles'

    cmd = ['cmake', '-G', generator]
    cmd.extend(
        [
            '-DCMAKE_TOOLCHAIN_FILE=' + toolchain_file,
            '-DCMAKE_BUILD_TYPE=' + ('Release' if args.release else 'Debug')
        ]
    )
    if extra_flags:
        cmd.extend(extra_flags)

    if system_defines_file:
        cmd.append('-DEXTARNAL_DEFINE_FILE=' + system_defines_file)

    if 'Beaglebone_YoctoLinux_mbedtls' == target:
        # TODO: Change to general location
        env_file = os.path.join(args.environment_script_dir, 'environment-setup-cortexa8hf-neon-poky-linux-gnueabi')
        source(env_file, update=True)

    check_call(cmd, cwd=build_dir)


def mbed_os_deploy(project_dir):
    check_call(['mbed', 'deploy', '.'], project_dir)
    if not os.path.isfile(os.path.join(project_dir, '.mbed')):
        check_call(['mbed', 'new', '.'], project_dir)


def mbed_os_generate_ide(project_dir, ide, target, verbose):
    logger.info('Generating project files for %s IDE target %s', ide, target)
    verbose_arg = ['-v'] if verbose else []
    check_call(['mbed', 'export', '--ide', ide, '--target', target] + verbose_arg, cwd=project_dir)


class DevConfig(object):
    def __init__(self, proj_dir, raw_cfg, platform):
        try:
            plat_cfg = raw_cfg[platform]
            self.repos = [GitRepo(proj_dir, raw_cfg['repositories'][r]) for r in plat_cfg['repositories-to-clone']]
            self.files = [GitRepo(proj_dir, raw_cfg['repositories'][r]) for r in plat_cfg['files-to-generate']]
            self.middlewares = plat_cfg['middlewares'] if 'middlewares' in plat_cfg else []
        except KeyError:
            logger.exception('Malformed configuration')
            raise


def deep_clean(project_dir, remove_repositories):
    logger.info('Remove build environment files')
    output = check_output(['git', 'clean', '-fdx'], cwd=project_dir)
    if remove_repositories:
        for line in output.splitlines():
            if 'Skipping repository' in line:
                repo_dir = line.replace('Skipping repository ', '')
                full_path = os.path.normpath(os.path.join(project_dir, repo_dir))
                shutil.rmtree(full_path)


toolchain_cmake_mapper = {
    'GCC': os.path.join(__project_dir, 'pal-platform/Toolchain/GCC/GCC.cmake'),
    'GCC_ARM': os.path.join(__project_dir, 'pal-platform/Toolchain/ARMGCC/ARMGCC.cmake'),
    'ARM': os.path.join(__project_dir, 'pal-platform/Toolchain/ARMCC/ARMCC.cmake'),
    'POKY-GLIBC': os.path.join(__project_dir, 'pal-platform/Toolchain/POKY-GLIBC/POKY-GLIBC.cmake')
}


def mbed_os_generate_make(project_dir, device, verbose_define, toolchain, release):
    with open(os.path.join(project_dir, 'devenv/build-mbed/deploy-cfg.mk'), 'wt') as fh:
        fh.write(
            textwrap.dedent('''
                VERBOSE:={verbose}
                MBEDOS_COMP:={toolchain}
                MBEDOS_TARGET:={device}
                DEBUG:={not_release}
                '''.format(
                    verbose=(1 if verbose_define else 0),
                    device=device,
                    toolchain=toolchain,
                    not_release=(0 if release else 1)
                )
            )
        )


def main():
    parser = get_parser()
    args = parser.parse_args()

    logging.basicConfig(
        level=logging.DEBUG if args.verbose else logging.INFO,
        format='%(asctime)s - %(name)s - %(levelname)s - %(message)s',
        stream=sys.stdout
    )

    if not args.no_color:
        logging.addLevelName(logging.WARNING, '\033[1;35m{}\033[0m'.format(logging.getLevelName(logging.WARNING)))
        logging.addLevelName(logging.ERROR, '\033[1;31m{}\033[0m'.format(logging.getLevelName(logging.ERROR)))
        logging.addLevelName(logging.INFO, '\033[1;36m{}\033[0m'.format(logging.getLevelName(logging.INFO)))

    logging.debug('Loading %s configuration file', args.configuration.name)
    raw_cfg = json.load(args.configuration)

    if args.operation == 'mbed-os-deploy':
        dev_cfg = DevConfig(__project_dir, raw_cfg, 'mbed-os-dev')
        initial_deploy(__project_dir, dev_cfg, args.no_skip)
        mbed_os_deploy(__project_dir)
    elif args.operation == 'mbed-os-ide':
        mbed_os_generate_ide(__project_dir, args.ide, args.device, args.verbose)
    elif args.operation == 'mbed-os-generate':
        mbed_os_generate_make(__project_dir, args.device, args.define_verbose, args.toolchain, args.release)

    elif args.operation == 'linux-deploy':
        dev_cfg = DevConfig(__project_dir, raw_cfg, 'pal-dev')
        initial_deploy(__project_dir, dev_cfg, args.no_skip)
        pal_deploy(args, project_dir=__project_dir, target='x86_x64_NativeLinux_mbedtls')
    elif args.operation == 'linux-generate':
        build_dir = os.path.join(__project_dir, '__x86_x64_NativeLinux_mbedtls')
        args.toolchain = 'GCC'
        if DO_RESET_BUILD_DIR:  # delete build dir and regenerate it
            if os.path.isdir(build_dir):
                shutil.rmtree(build_dir)
        pal_generate(
            args,
            project_dir=__project_dir,
            target='x86_x64_NativeLinux_mbedtls',
            build_dir=build_dir,
            system_defines_file=os.path.join(
                __project_dir,
                'devenv/build-linux/Linux_config.cmake'  # platform build flags
            )
        )

    elif args.operation == 'linux-yocto-deploy':
        dev_cfg = DevConfig(__project_dir, raw_cfg, 'pal-dev')
        initial_deploy(__project_dir, dev_cfg, args.no_skip)
        pal_deploy(args, project_dir=__project_dir, target='Beaglebone_YoctoLinux_mbedtls')
    elif args.operation == 'linux-yocto-generate':
        build_dir = os.path.join(__project_dir, '__Beaglebone_YoctoLinux_mbedtls')
        args.toolchain = 'POKY-GLIBC'
        if DO_RESET_BUILD_DIR:  # delete build dir and regenerate it
            if os.path.isdir(build_dir):
                shutil.rmtree(build_dir)
        pal_generate(
            args,
            project_dir=__project_dir,
            target='Beaglebone_YoctoLinux_mbedtls',
            build_dir=build_dir,
            system_defines_file=os.path.join(
                __project_dir,
                'devenv/build-linux/Linux_config.cmake'  # platform build flags
            )
        )

    elif args.operation == 'free-rtos-deploy':
        dev_cfg = DevConfig(__project_dir, raw_cfg, 'pal-dev')
        initial_deploy(__project_dir, dev_cfg, args.no_skip)
        pal_deploy(args, project_dir=__project_dir, target='K64F_FreeRTOS_mbedtls')
    elif args.operation == 'free-rtos-generate':
        build_dir = os.path.join(__project_dir, '__K64F_FreeRTOS_mbedtls')
        if DO_RESET_BUILD_DIR:  # delete build dir and regenerate it
            if os.path.isdir(build_dir):
                shutil.rmtree(build_dir)
        pal_generate(
            args,
            project_dir=__project_dir,
            target='K64F_FreeRTOS_mbedtls',
            build_dir=build_dir,
            system_defines_file=os.path.join(
                __project_dir,
                'devenv/build-freertos/FreeRTOS_config.cmake'  # platform build flags
            )
        )

    elif args.operation == 'clean':
        deep_clean(__project_dir, args.repos)
    else:
        raise Exception('unsupported operation')


if __name__ == '__main__':
    main()
    logger.info('-------------------- ALL DONE --------------------')