# Factory configurator client

## Quick start

### Clone

To clone the Factory Client repository -
```
git clone git@github.com:ARMmbed/factory-configurator-client
```

### Compile

Compile for mbed OS (using mbed-cli) as follows --
```
cd factory-tool
source devenv/pv_env_setup.sh K64F_MBEDOS_ARM_GCC
make
```

## Compilation flags

For proper compilation of factory client code, some flags should be used.

### mbed OS flags

Currently, the code is dircetly using **mbedTLS** and **CFStore** components of mbed OS. 
CFStore is activated in asynchronous mode.  

The following mbed OS flags should be used -  
`-DMBEDTLS_CMAC_C -DMBEDTLS_PEM_WRITE_C -DMBEDTLS_CTR_DRBG_MAX_REQUEST=2048 -DSTORAGE_DRIVER_CONFIG_HARDWARE_MTD_ASYNC_OPS=0 -DSTORAGE_CONFIG_HARDWARE_MTD_K64F_ASYNC_OPS=0`

### Factory client flags

Following is list of Factory Client flags that should be used:

#### Log level

Set log level to INFO -  
`-DMBED_TRACE_MAX_LEVEL=TRACE_LEVEL_DEBUG`  

The possible trace levels are -

* `TRACE_LEVEL_DEBUG` - this print is some deep information for debug purpose .this is default. If the flag wasn't defined at all, this is what will be used.  
* `TRACE_LEVEL_INFO`
* `TRACE_LEVEL_WARN` 
* `TRACE_LEVEL_ERROR`
* `TRACE_LEVEL_CMD`  

#### Toolchain selection

Currently, only mbed OS compilation is supported -  
`-DSA_PV_OS_MBEDOS`

For GCC toolchain -  
 `-DSA_PV_TOOLCHAIN_ARM_GCC` 

For ARMCC toolchain -  
`-DSA_PV_TOOLCHAIN_ARMCC`
