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
    echo "${SCRIPT} nginx will install to ${INSTALL_DIR}."
    echo "${SCRIPT} Type 'yes|y' or 'dir path which you want to install'."
    read answer
    case $answer in
        yes)
            echo -e "${SCRIPT} [yes]\n"
            echo -e "${SCRIPT} nginx will install to ${INSTALL_DIR}.\n"
            break
            ;;
        y)
            echo -e "${SCRIPT} [y]\n"
            echo -e "${SCRIPT} nginx will install to ${INSTALL_DIR}.\n"
            break
            ;;
        *)
            echo -e "${SCRIPT} [$answer]\n"
            INSTALL_DIR=$answer
            echo -e "${SCRIPT} instll dir is changed.\n"
            ;;
    esac
done

echo "${SCRIPT} cd to tmp dir"
mkdir ${TMPDIR}
cd ${TMPDIR}

wget http://nginx.org/download/${NGX_DIR_NAME}.tar.gz
tar xfvz ./${NGX_DIR_NAME}.tar.gz
cd ${NGX_DIR_NAME}

./configure --add-module=${BASEDIR}/../ --prefix=${INSTALL_DIR}
${BASEDIR}/../bin/fix_makefile.pl ./objs/Makefile
make
sudo make install

echo "${SCRIPT} nginx with msgpack_rpc_module can start to exec =${INSTALL_DIR}/sbin/nginx"
echo "Usage :"
echo "  Start                     : ${INSTALL_DIR}/sbin/nginx"
echo "  Stop                      : ${INSTALL_DIR}/sbin/nginx -s stop"
echo "  Quit after fetch request  : ${INSTALL_DIR}/sbin/nginx -s quit"
echo "  Reopen the logfiles       : ${INSTALL_DIR}/sbin/nginx -s reopen"
echo "  Reload nginx.conf         : ${INSTALL_DIR}/sbin/nginx -s reloqd"
