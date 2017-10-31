#!/bin/sh

# ServerAntAgentService 信息
SAAS_SRC_TAR="ServerAntAgent.tar.gz"
SAAS_SRC_PWD="$(pwd)/ServerAntAgent"
SAAS_SRC_CFG="ServerAntAgent.cfg"

# Agent依赖boot库, boost库无需安装,直接解压, 编译时使用, 运行时不需要. 
# 通过目录检测
THIRD_BOOST_NAME="boost"
THIRD_BOOST_TAR="boost_1_62_0.tar.bz2"
THIRD_BOOST_DIR="${SAAS_SRC_PWD}/third_group/boost_1_62_0"

# Agent依赖curl库, 用于向远端Server提交http的post请求, 并接收应答
# 通过pkg-config检测
THIRD_CURL_NAME="libcurl"
THIRD_CURL_TAR="curl-7.50.3.tar.gz"
THIRD_CURL_DIR="${SAAS_SRC_PWD}/third_group/curl-7.50.3"

# Agent依赖microhttpd库, 用于接收远端发起的http请求, 并发送应答
# 通过pkg-config检测
THIRD_MICROHTTPD_NAME="libmicrohttpd"
THIRD_MICROHTTPD_TAR="libmicrohttpd-0.9.51.tar.gz"
THIRD_MICROHTTPD_DIR="${SAAS_SRC_PWD}/third_group/libmicrohttpd-0.9.51"

# Agent依赖rdkafka库, 用于向kafka发送消息.
# 通过pkg-config检测
THIRD_RDKAFKA_NAME="rdkafka"
THIRD_RDKAFKA_TAR="librdkafka-master.tar.gz"
THIRD_RDKAFKA_DIR="${SAAS_SRC_PWD}/third_group/librdkafka-master"

# 检查echo是否可用
ECHO=$(which echo)

# 完成解压, 安装, 更新 ServerAntAgent服务操作.
# 删除旧目录, 避免旧文件时间不同步告警.
if [ -d "${SAAS_SRC_PWD}" ] ;then
    rm ${SAAS_SRC_PWD} -r
fi

${ECHO}  "Extract ${SAAS_SRC_TAR} in Agent ${Agent_ConnectInfo_IP} ... "
tar -xf ${SAAS_SRC_TAR}
${ECHO}  "Extract ${SAAS_SRC_TAR} success"

${ECHO}  "Entering directory ${SAAS_SRC_PWD}"
cd ${SAAS_SRC_PWD}

# 检查boost是否已经解压. boost无需安装, 只需解压即可, 直接检查目录.
${ECHO} "Checking ${THIRD_BOOST_NAME} ..."
if [ -d "${THIRD_BOOST_DIR}" ]
then
    ${ECHO} "Found ${THIRD_BOOST_NAME}"
else
    ${ECHO} "Install ${THIRD_BOOST_NAME} start"
    ${ECHO} "Entering directory ${SAAS_SRC_PWD}/third_group"
    cd "${SAAS_SRC_PWD}/third_group"
    if test -e "./${THIRD_BOOST_TAR}" ; then
        ${ECHO} "Extract ${THIRD_BOOST_TAR} ..."
        tar -xf ${THIRD_BOOST_TAR}
        ${ECHO}  "Extract ${SAAS_SRC_TAR} success"
    else
        ${ECHO} "Error: Can not find ${THIRD_BOOST_TAR} in $(pwd)" >&2
        exit -1
    fi
    ${ECHO}  "Leaving directory ${SAAS_SRC_PWD}/third_group"
    ${ECHO} "Install ${THIRD_BOOST_NAME} success"
    cd "${SAAS_SRC_PWD}"
fi

# 检查pkg-config是否可用
PKG_CONFIG=$(which pkg-config)
if test "x$PKG_CONFIG" != x; then
    # 使用pkg-config检查libcurl
    ${ECHO} "Checking  ${THIRD_CURL_NAME} ..."
    ${PKG_CONFIG} --exists ${THIRD_CURL_NAME}
    if [ 0 -eq $? ]
    then
        ${ECHO} "Found ${THIRD_CURL_NAME}"
    else
        ${ECHO} "Install ${THIRD_CURL_NAME} start"
        ${ECHO}  "Entering directory ${SAAS_SRC_PWD}/third_group"
        cd "${SAAS_SRC_PWD}/third_group"
        if test -e "./${THIRD_CURL_TAR}" ; then
            ${ECHO} "Extract ${THIRD_CURL_TAR} ..."
            tar -xf ${THIRD_CURL_TAR}
            ${ECHO}  "Extract ${THIRD_CURL_TAR} success"
            
            cd  "${THIRD_CURL_DIR}"
            ./configure
            if [ 0 -ne $? ]; then 
                ${ECHO} "Configure for  ${THIRD_CURL_TAR} Failed" >&2
                exit -1
            fi
            make
            if [ 0 -ne $? ]; then 
                ${ECHO} "Make for  ${THIRD_CURL_TAR} Failed" >&2
                exit -1
            fi
            make install
            if [ 0 -ne $? ]; then 
                ${ECHO} "Make install for  ${THIRD_CURL_TAR} Failed" >&2
                exit -1
            fi
            # 不关心ldconfig的错误输出.
            ldconfig > /dev/null 2>&1
        else
            ${ECHO} "Error: Can not find ${THIRD_CURL_TAR} in $(pwd)" >&2
            exit -1
        fi
        ${ECHO}  "Leaving directory ${SAAS_SRC_PWD}/third_group"
        ${ECHO} "Install lib ${THIRD_CURL_NAME} success"
        cd "${SAAS_SRC_PWD}"
    fi
    
    # 使用pkg-config检查libmicrohttpd
    ${ECHO} "Checking  ${THIRD_MICROHTTPD_NAME} ..."
    ${PKG_CONFIG} --exists ${THIRD_MICROHTTPD_NAME}
    if [ 0 -eq $? ] 
    then
        ${ECHO} "Found ${THIRD_MICROHTTPD_NAME}"
    else
        ${ECHO} "Install ${THIRD_MICROHTTPD_NAME} start"
        ${ECHO}  "Entering directory ${SAAS_SRC_PWD}/third_group"
        cd "${SAAS_SRC_PWD}/third_group"
        if test -e "./${THIRD_MICROHTTPD_TAR}" ; then
            ${ECHO} "Extract ${THIRD_MICROHTTPD_TAR} ..."
            tar -xf ${THIRD_MICROHTTPD_TAR}
            ${ECHO}  "Extract ${THIRD_MICROHTTPD_TAR} success"
            
            cd  "${THIRD_MICROHTTPD_DIR}"
            ./configure
            if [ 0 -ne $? ]; then 
                ${ECHO} "Configure for  ${THIRD_MICROHTTPD_DIR} Failed" >&2
                exit -1
            fi
            make
            if [ 0 -ne $? ]; then 
                ${ECHO} "Make for  ${THIRD_MICROHTTPD_DIR} Failed" >&2
                exit -1
            fi
            make install
            if [ 0 -ne $? ]; then 
                ${ECHO} "Make install for  ${THIRD_MICROHTTPD_DIR} Failed" >&2
                exit -1
            fi
            # 不关心ldconfig的错误输出.
            ldconfig > /dev/null 2>&1
        else
            ${ECHO} "Error: Can not find ${THIRD_MICROHTTPD_TAR} in $(pwd)" >&2
            exit -1
        fi
        ${ECHO}  "Leaving directory ${SAAS_SRC_PWD}/third_group"
        ${ECHO} "Install lib ${THIRD_MICROHTTPD_NAME} success"
        cd "${SAAS_SRC_PWD}"
    fi
    
    # 使用pkg-config检查rdkafka
    ${ECHO} "Checking  ${THIRD_RDKAFKA_NAME} ..."
    ${PKG_CONFIG} --exists ${THIRD_RDKAFKA_NAME}
    if [ 0 -eq $? ] 
    then
        ${ECHO} "Found ${THIRD_RDKAFKA_NAME}"
    else
        ${ECHO} "Install ${THIRD_RDKAFKA_NAME} start"
        ${ECHO}  "Entering directory ${SAAS_SRC_PWD}/third_group"
        cd "${SAAS_SRC_PWD}/third_group"
        if test -e "./${THIRD_RDKAFKA_TAR}" ; then
            ${ECHO} "Extract ${THIRD_RDKAFKA_TAR} ..."
            tar -xf ${THIRD_RDKAFKA_TAR}
            ${ECHO}  "Extract ${THIRD_RDKAFKA_TAR} success"
            
            cd  "${THIRD_RDKAFKA_DIR}"
            ./configure
            if [ 0 -ne $? ]; then 
                ${ECHO} "Configure for  ${THIRD_RDKAFKA_DIR} Failed" >&2
                exit -1
            fi
            make
            if [ 0 -ne $? ]; then 
                ${ECHO} "Make for  ${THIRD_RDKAFKA_DIR} Failed" >&2
                exit -1
            fi
            make install
            if [ 0 -ne $? ]; then 
                ${ECHO} "Make install for  ${THIRD_RDKAFKA_DIR} Failed" >&2
                exit -1
            fi
            # 不关心ldconfig的错误输出.
            ldconfig > /dev/null 2>&1
        else
            ${ECHO} "Error: Can not find ${THIRD_RDKAFKA_TAR} in $(pwd)" >&2
            exit -1
        fi
        ${ECHO}  "Leaving directory ${SAAS_SRC_PWD}/third_group"
        ${ECHO} "Install lib ${THIRD_RDKAFKA_NAME} success"
        cd "${SAAS_SRC_PWD}"
    fi
fi

${ECHO} "Checking  ${SAAS_SRC_CFG} ..."
if [ -e ${SAAS_SRC_CFG} ]
then
    ${ECHO} "Found ${SAAS_SRC_CFG}"
    ${ECHO} "Update AgentIP in ${SAAS_SRC_CFG} to ${Agent_ConnectInfo_IP}"
    
    # Agent IP默认以0.0.0.0表示, 此处替换成Agent的实际管理IP
    sed -i "s/0.0.0.0/${Agent_ConnectInfo_IP}/g" ${SAAS_SRC_CFG}
    ${ECHO} "Update ${SAAS_SRC_CFG} success"
else
    ${ECHO} "Error: Can not find ${SAAS_SRC_CFG} in $(pwd)" >&2
    exit -1
fi


${ECHO} "Start Compile ServerAntAgent"

make clean
if [ 0 -ne $? ]; then 
    ${ECHO} "Make clean for ServerAntAgent Failed" >&2
    exit -1
fi

make
if [ 0 -ne $? ]; then 
    ${ECHO} "Make for ServerAntAgent Failed" >&2
    exit -1
fi

${ECHO} "Start Install ServerAntAgent"
# paramiko远程启动service, 特殊格式打印会导致paramiko挂死
# 正常日志导入日志文件, 错误日志继续返回给paramiko, Manager端根据是否存在错误日志判断执行是否成功
make install > install.log
if [ 0 -eq $? ]
then 
    ${ECHO} "Make install for ServerAntAgent success" >&1
    exit 0
else
    ${ECHO} "Make install for ServerAntAgent failed! Check the install.log" >&2
    exit -1
fi
