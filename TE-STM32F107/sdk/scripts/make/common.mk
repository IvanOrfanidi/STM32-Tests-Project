# настройка путевой системы проекта
ROOT_DIR=$(SRC_DIR)/..
OUT_DIR=$(ROOT_DIR)/out
LIB_DIR=$(ROOT_DIR)/lib
DOC_DIR=$(ROOT_DIR)/doc
PC_DIR=$(ROOT_DIR)/pc
SCRIPT_DIR=$(ROOT_DIR)/scripts

SDK_DIR=$(ROOT_DIR)/../SDK

# инструменты сборки
CC      = arm$(TOOLS_VARIIANT)-gcc
CXX     = arm$(TOOLS_VARIIANT)-g++
FC      = arm$(TOOLS_VARIIANT)-gfortran
LD      = arm$(TOOLS_VARIIANT)-ld
AR      = arm$(TOOLS_VARIIANT)-ar
AS      = arm$(TOOLS_VARIIANT)-as
CP      = arm$(TOOLS_VARIIANT)-objcopy
OD		= arm$(TOOLS_VARIIANT)-objdump
SZ      = arm$(TOOLS_VARIIANT)-size
SR      = arm$(TOOLS_VARIIANT)-strip
GDB     = arm$(TOOLS_VARIIANT)-gdb
RM		= rm
TAR     = tar
TOUCH   = touch

# название пакета
DATE		= $$(date +%Y%m%d)
PKG_NAME	= $(LIBNAME)$(PRJNAME)

# опции библиотекаря
ARFLAGS = -rcs

CPFLAGS = -O ihex
ODFLAGS	= -x --syms