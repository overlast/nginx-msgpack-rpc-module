#!/bin/bash

#!/usr/bin/env bash

#set -x # show executed commands
set -e # die when an error will occur

BASEDIR=`cd $(dirname $0); pwd`
USER_ID=`/usr/bin/id -u`
SCRIPT="[make_sample_nginx] : "

TMPDIR=/tmp/nginx-msgpack-rpc-tmp
NGX_VERSION=1.8.0
NGX_DIR_NAME=nginx-${NGX_VERSION}

NGX_DIR=nginx-with-msgpack-rpc-module
INSTALL_DIR=/usr/local/${NGX_DIR}

usage() {
    echo "Usage: $PROGRAM_NAME [OPTIONS]"
    echo "  This script is the installer of mecab-ipadic-neologd"
    echo
    echo "Options:"
    echo "  -h, --help"
    echo
    echo "  -v, --version"
    echo
    echo "  -p, --prefix /PATH/TO/INSTALL/DIRECTORY"
    echo "     Set any directory path where you want to install"
    echo
    echo "  -y, --forceyes"
    echo "     If you want to install regardless of the result of test"
    echo
    echo "  -u, --asuser"
    echo "     If you want to install to the user directory as an user"
    echo
}

IS_FORCE_YES=0
IS_AS_USER=0

for OPT in "$@"
do
    case "$OPT" in
        '-h'|'--help' )
            usage
            exit 1
            ;;
        '-v'|'--version' )
            echo "For nginx-$NGX_VERSION"
            exit 1
            ;;
        '-p'|'--prefix' )
            if [[ -z "$2" ]] || [[ "$2" =~ ^-+ ]]; then
                echo "${PROGRAM_NAME}: option requires an argument -- $1" 1>&2
                usage
                exit 1
            fi
            INSTALL_DIR="$2"
            shift 2
            ;;
        '-y'|'--forceyes' )
            IS_FORCE_YES=1
            shift 1
            ;;
        '-u'|'--asuser' )
            IS_AS_USER=1
            shift 1
            ;;
        -*)
            echo "${PROGRAM_NAME}: illegal option -- '$(echo $1 | sed 's/^-*//')'" 1>&2
            usage
            exit 1
            ;;
        *)
            if [[ ! -z "$1" ]] && [[ ! "$1" =~ ^-+ ]]; then
                #param=( ${param[@]} "$1" )
                param+=( "$1" )
                shift 1
            fi
            ;;
    esac
done

if [ ${IS_FORCE_YES} -eq 0 ]; then
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
fi

if [ ! -e /usr/local/lib/libmsgpack_rpc_client.so.0.0.1 ]; then
    echo "$SCRIPT_NAME msgpack-rpc-c must be installed.."
    $BASEDIR/../sh/make_centos_env.sh
fi

echo "${SCRIPT} cd to tmp dir"
mkdir -p ${TMPDIR}
cd ${TMPDIR}

wget http://nginx.org/download/${NGX_DIR_NAME}.tar.gz
tar xfvz ./${NGX_DIR_NAME}.tar.gz
cd ${NGX_DIR_NAME}

git clone https://github.com/openresty/echo-nginx-module.git
mv echo-nginx-module /tmp/echo-nginx-module

./configure --add-module=/tmp/echo-nginx-module --add-module=${BASEDIR}/../ --prefix=${INSTALL_DIR}
${BASEDIR}/../bin/fix_makefile.pl ./objs/Makefile
make

if [ ${IS_AS_USER} -eq 0 ]; then
    sudo make install
else
    make install
fi

echo "${SCRIPT} nginx.conf is here => ${INSTALL_DIR}/conf/nginx.conf"
echo ""
echo "${SCRIPT} nginx with msgpack_rpc_module can start to exec =${INSTALL_DIR}/sbin/nginx"
echo "Usage :"
echo "  Start                     : ${INSTALL_DIR}/sbin/nginx"
echo "  Stop                      : ${INSTALL_DIR}/sbin/nginx -s stop"
echo "  Quit after fetch request  : ${INSTALL_DIR}/sbin/nginx -s quit"
echo "  Reopen the logfiles       : ${INSTALL_DIR}/sbin/nginx -s reopen"
echo "  Reload nginx.conf         : ${INSTALL_DIR}/sbin/nginx -s reloqd"
