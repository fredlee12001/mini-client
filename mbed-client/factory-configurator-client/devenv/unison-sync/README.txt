Overview
========

This script is used to synchronize two directories, one in Windows and another in 
Linux (obviously, other variations are also possible).

The script uses the Unison File Synchronizer (http://www.cis.upenn.edu/~bcpierce/unison/),
which does very efficient file transfers (based on the rdiff protocol), with active
file monitoring (every changed file is immediately synchronized).

Installation
============

The sample configuration file, unison_sync_conf.yaml, should be copied to the %APPDATA%
directory in Windows.
IMPORTANT - In %APPDATA%, the file should be UPDATED, to reflect local user settings (required directories
to synchronize, name of SSH binary).
For seamless operation, a password-less login to the target server using SSH is
desired. Achieving that is outside the scope of this README file, but instructions
can be found easily. See http://linuxconfig.org/passwordless-ssh for example.

Usage
=====

Simply run unison_sync_loop.py.
This script tries to ping the development server (fw-srv1).
If the ping is successful, the script starts unison_sync.py, which does active 
synchronization.
If the ping fails, the script waits for 10 seconds and tries again.

Ignored Files
=============

Some files should not be synchronized.
For example, most ".a" and ".o" files are of no interest in Windows, so there is no
reason to waste time copying them every time a build is performed in Linux.
The unison_sync_ignores.txt file contains a list of ignored files and directories,
in Unison format (for details, see 
http://www.cis.upenn.edu/~bcpierce/unison/download/releases/stable/unison-manual.html#pathspec).
This list should be edited when new ignored files are needed. It currently has
relatively good settings for the SPV project.

Pitfalls
========

Known issues -
1. If a file that should have an executable bit in Linux is created in Windows, it wouldn't
   have the executable bit on Linux. One way around this is to make sure new files are
   created only on Linux. Another is to run a secondary script that identifies such
   files and handles them. No such script exists yet.
2. If a change is made to the Linux file-system, from Windows, through the SMB share
   (drive U), it *will not be detected* by the active monitor on Linux or on Windows.
   Such changes, if made, should be followed by a manual run of the script.
   Best is to not perform such changes...

Running on Boot
===============

The script doesn't automatically run on boot. It can be placed in the Windows Startup
folder to achieve that.

Running in Tray
===============

The script opens a window, which stays open.
There are various ways to run the script minimized to tray. This is left as an
exercise to the reader...

Unison Binaries
===============

Unison binaries in both Windows and Linux must be compiled with exactly the same OCaml
version, or weird problems would occur during synchronziation.

Windows binaries are in the tree, under "unison_windows_bin".

Linux binaries are already installed on prov-dev, under /usr/local/bin.
To compile a brand new version, the build_unison.sh script under unison_linux_bin
directory can be used. It utilizes Docker for the compilation.
