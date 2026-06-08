# JLed for the Raspberry Pi Pico

This examples demonstrates how to use JLed on the Raspberry Pi Pico. The
built-in LED (GPIO 25) and a LED on GPIO 16 will be faded.

Also a Dockerfile is provided to enable a hassle-free build.

<!-- vim-markdown-toc GFM -->

* [Building the demo](#building-the-demo)
    * [Docker build](#docker-build)
    * [Local build](#local-build)
* [Deploy the sketch to the Raspberry Pi Pico](#deploy-the-sketch-to-the-raspberry-pi-pico)
* [Author](#author)

<!-- vim-markdown-toc -->
## Building the demo

You have two options to build the demo. Either use a docker image (recommended),
or setup everything yourself (consult Pico Quickstart Guide).

### Docker build

A [Dockerfile](Dockerfile) and a [build-script](build.sh) are provided to 
enable a hassle-free build of the demo. The script will first build a 
docker-image containing the build environment, then build the example.

* run `./build.sh docker-image` to build the docker image with the build
  environment (compilers, pico SDK etc.) - only needed to run once (as long
  as the Dockerfile is not changed).
* run `./build.sh compile` to compile the example. The resulting `pico_demo.uf2`
  file to be uploaded will be found in this directory afterwards.
* run `./build.sh clean` to clean up all files created during a build.

### Local build 

You need the [pico-sdk](https://github.com/raspberrypi/pico-sdk) and 
necessary tools to compile everything installed.

The `PICO_SDK_PATH` environment variable must point to the installation
directory of the SDK.

To compile the demo sketch, run `cmake . && make`.  The resulting
`pico_demo.uf2` file to be uploaded will be found in this directory afterwards.

## Deploy the sketch to the Raspberry Pi Pico

To deploy the demo sketch `pico_demo.uf2`, press and hold `BOOTSEL` on the Pico
and connect the Pico to your PC. The Pico will now be mounted as an external
drive. Copy the file `pico_demo.uf2` to the mount point. The sketch should now
start automatically.

## Author 

(c) Copyright 2021 by Jan Delgado, License: MIT

