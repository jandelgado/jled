#!/bin/bash
# build the raspberry pi pico JLed example using a docker container.
# Run "./build.sh docker-image" first to build the docker-image,
# then run "./build.sh compile" to compile the example.
#
# Jan Delgado 02/2021
set -eou pipefail

usage() {
    echo "$0: <docker-image|compile|shell|clean>"
}

build_image() {
    docker build \
        --build-arg="TZ=$(timedatectl show -p Timezone --value)" \
        -t picosdk:latest .
}

run_cmd() {
    docker run -ti --rm \
        --user="$(id -u):$(id -g)" \
        -v "$(pwd)/../..:/src:z" \
        picosdk:latest "$@"
}

main() {
    case $action in
        docker-image) build_image ;;
        compile)
            run_cmd sh -c "cd /src/examples/raspi_pico && cmake . && make"
            local -r line=$(printf '=%.0s' {1..75})
            echo "$line"
            echo "BUILD SUCCESSFUL."
            echo "Now upload the file pico_demo.uf2 to your Pico manually."
            echo "$line"
            ;;
        shell) run_cmd bash ;;
        clean) git clean -d -x -f ;;
        *) usage ;;
    esac
}

action=${1:-""}
main action

