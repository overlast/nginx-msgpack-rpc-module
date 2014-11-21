#!/bin/bash

#!/usr/bin/env bash

#set -x # show executed commands
set -e # die when an error will occur

BASEDIR=`cd $(dirname $0); pwd`
USER_ID=`/usr/bin/id -u`
SCRIPT="[make_sample_nginx] : "

TMPDIR=/tmp/nginx-msgpack-rpc-tmp
NGX_VERSION=1.6.2
NGX_DIR_NAME=nginx-${NGX_VERSION}

NGX_DIR=nginx-with-msgpack-rpc-module
INSTALL_DIR=/usr/local/${NGX_DIR}

while true;do
    echo "${script} nginx will install to ${INSTALL_DIR}."
    echo "${script} Type 'yes|y' or 'dir path which you want to install'."
    read answer
    case $answer in
        yes)
            echo -e "${script} [yes]\n"
            echo -e "${script} nginx will install to ${INSTALL_DIR}.\n"
            break
            ;;
        y)
            echo -e "${script} [y]\n"
            echo -e "${script} nginx will install to ${INSTALL_DIR}.\n"
            break
            ;;
        *)
            echo -e "${script} [$answer]\n"
            INSTALL_DIR=$answer
            echo -e "${script} instll dir is changed.\n"
            ;;
    esac
done

echo "${script} cd to tmp dir"
mkdir ${TMPDIR}
cd ${TMPDIR}

wget http://nginx.org/download/${NGX_DIR_NAME}.tar.gz
tar xfvz ./${NGX_DIR_NAME}.tar.gz
cd ${NGX_DIR_NAME}

./configure --add-module=${BASEDIR}/../ --prefix=${INSTALL_DIR}
${BASEDIR}/../bin/fix_makefile.pl ./objs/Makefile
make
sudo make install
