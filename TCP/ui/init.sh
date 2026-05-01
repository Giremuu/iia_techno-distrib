#!/bin/bash

set -e

init () {
    if [[ ! -f "package.json" ]]; then
        npm init
        npm install socket-io-client
    fi
}