
# Makefile for SOC driver Library
#
ROOT_DIR = $(shell pwd)
export ROOT_DIR
BIN_INSTALL_DIR = /usr/local/bin
CFG_INSTALL_DIR = /etc/ServerAntAgent
SCRIPT_INSTALL_DIR = /etc/init.d

BIN_RELEASE_DIR = $(ROOT_DIR)/release
RELEASE_DIR_NAME = ServerAntAgentSetup

# 支持Sles 11.3 和 Sles 12
SLES_OS_NAME = Sles
SLES_RELEASE_DIR = $(BIN_RELEASE_DIR)/$(SLES_OS_NAME)
SLES_RELEASE_TAR_NAME = ServerAntAgentFor$(SLES_OS_NAME).tar.gz

# 支持Euler 2.0
EULER_OS_NAME = Euler
EULER_RELEASE_DIR = $(BIN_RELEASE_DIR)/$(EULER_OS_NAME)
EULER_RELEASE_TAR_NAME = ServerAntAgentFor$(EULER_OS_NAME).tar.gz

#默认不打印详细编译流程
export Q=@

# 需要额外安装microhttp, librdkafka

export THIRD_GROUP_PATH = $(ROOT_DIR)/third_group
export BOOSTS_PATH = $(THIRD_GROUP_PATH)/boost_1_62_0
export SCRIPT_PATH = $(ROOT_DIR)/tools/script

# 部分配置不合理的编译器(使用错误参数编译出的编译器)不会搜索/usr/local目录.
# C++编译参数
export CXXFLAGS  = -g -fPIC -shared  -I $(BOOSTS_PATH) -I /usr/local/include
#链接参数
export LDFLAGS = -lpthread -lcurl -Wl,-rpath=libs
# sles下需要安装 python-devel.x86_64 包
#Python 信息
#PYTHONINC := /usr/include/python
#CXXFLAGS += -I ${PYTHONINC}
#LDFLAGS += -lpython2.6

LOCALDIR = 
include ${ROOT_DIR}/make/Make.config

subdirs := src
include ${ROOT_DIR}/make/Make.subdirs


ALL_OBJ = $(shell find ${BUILD_DIR} -name *.o)

all::
	@$(ECHO) Linking ${ALL_OBJ} third_group/libs/Sles/libcrypto.so.1.0.0 third_group/libs/Sles/libssl.so.1.0.0
	$Q$(RM) ServerAntAgent
	$(CXX) -o ./ServerAntAgent ${ALL_OBJ} $(LDFLAGS)

ServerAntAgent:: all

release:: release_Sles release_Euler

release_Sles:: ServerAntAgent
	$Q $(ECHO) Release $(SLES_RELEASE_TAR_NAME) to $(SLES_RELEASE_DIR)/$(RELEASE_DIR_NAME)
	$Q mkdir -p $(SLES_RELEASE_DIR)/$(RELEASE_DIR_NAME)
	$Q cp ServerAntAgent $(SLES_RELEASE_DIR)/$(RELEASE_DIR_NAME)
	$Q cp ServerAntAgent.cfg $(SLES_RELEASE_DIR)/$(RELEASE_DIR_NAME)
	$Q cp $(SCRIPT_PATH)/$(SLES_OS_NAME)/* $(SLES_RELEASE_DIR)/$(RELEASE_DIR_NAME)
	$Q mkdir -p $(SLES_RELEASE_DIR)/$(RELEASE_DIR_NAME)/logs
	$Q $(ECHO) Release libs to $(SLES_RELEASE_DIR)/$(RELEASE_DIR_NAME)/libs
	$Q mkdir -p $(SLES_RELEASE_DIR)/$(RELEASE_DIR_NAME)/libs
#	$Q cp $(THIRD_GROUP_PATH)/libs/$(SLES_OS_NAME)/* $(SLES_RELEASE_DIR)/$(RELEASE_DIR_NAME)/libs
	$Q $(ECHO) Release tar to $(SLES_RELEASE_DIR)/$(SLES_RELEASE_TAR_NAME)
	$Q cd $(SLES_RELEASE_DIR); tar -acf $(SLES_RELEASE_TAR_NAME) $(RELEASE_DIR_NAME);
	$Q $(ECHO) Release tar $(SLES_RELEASE_TAR_NAME) Success

release_Euler:: ServerAntAgent
	$Q $(ECHO) Release $(EULER_RELEASE_TAR_NAME) to $(EULER_RELEASE_DIR)/$(RELEASE_DIR_NAME)
	$Q mkdir -p $(EULER_RELEASE_DIR)/$(RELEASE_DIR_NAME)
	$Q cp ServerAntAgent $(EULER_RELEASE_DIR)/$(RELEASE_DIR_NAME)
	$Q cp ServerAntAgent.cfg $(EULER_RELEASE_DIR)/$(RELEASE_DIR_NAME)
	$Q cp $(SCRIPT_PATH)/$(EULER_OS_NAME)/* $(EULER_RELEASE_DIR)/$(RELEASE_DIR_NAME)
	$Q mkdir -p $(EULER_RELEASE_DIR)/$(RELEASE_DIR_NAME)/logs
	$Q $(ECHO) Release libs to $(EULER_RELEASE_DIR)/$(RELEASE_DIR_NAME)/libs
	$Q mkdir -p $(EULER_RELEASE_DIR)/$(RELEASE_DIR_NAME)/libs
#	$Q cp $(THIRD_GROUP_PATH)/libs/$(EULER_OS_NAME)/* $(EULER_RELEASE_DIR)/$(RELEASE_DIR_NAME)/libs
	$Q $(ECHO) Release tar to $(EULER_RELEASE_DIR)/$(EULER_RELEASE_TAR_NAME)
	$Q cd $(EULER_RELEASE_DIR); tar -acf $(EULER_RELEASE_TAR_NAME) $(RELEASE_DIR_NAME);
	$Q $(ECHO) Release tar $(EULER_RELEASE_TAR_NAME) Success

clean ::
	$Q rm -fr ./ServerAntAgent ./build $(BIN_RELEASE_DIR)

