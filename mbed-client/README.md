# mbed-cloud-client
This repository contains ARM mbed Cloud Client: a library that connects devices to mbed Cloud Service and to mbed-enabled cloud services from our partners.

The documentation is collected under the docs directory and it is also hosted [here](http://cloud.mbed.com/docs/latest).


    Update classic lib after rebase


mbed-client-c
commit 2ad52e397cd553af16c3e23784409e62676923d1
Author: Antti Yli-Tokola <antti.yli-tokola@arm.com>
Date:   Fri Feb 10 16:45:20 2017 +0200

    Add option to include custom uri params into register and bootstrap messages.


mbed-client-classic
commit 6acacfebd333d111efd82edbd3f7d4ea77d74c40
Author: Antti Yli-Tokola <antti.yli-tokola@arm.com>
Date:   Mon May 8 10:29:40 2017 +0300

    Reduce memory footprint. (#73)

    * Reduce memory footprint.
     * Move receive buffer from member variable to local one


mbed-client-mbed-tls
commit 3e99b18395feba2baeb65b8a0afb403475558cf7
Author: Yogesh Pande <Yogesh.Pande@arm.com>
Date:   Wed Apr 19 19:04:02 2017 +0300

    Clearing certificate buffer before server validity check


mbed-cloud-client
commit f5d38ae44829258dc8c76d7588eb55178d083f9c
Author: Antti Yli-Tokola <antti.yli-tokola@arm.com>
Date:   Fri May 12 08:20:20 2017 +0300

    Update mbed-client lib after rebase
