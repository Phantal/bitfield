#!/bin/bash
# \
set -x; \
exec make --warn-undefined-variables -rR --output-sync=target ${@}

TOPDIR := $(shell cd $$(dirname $(firstword ${MAKEFILE_LIST})); pwd)

SHELL := bash
.SHELLFLAGS := -euo pipefail -c

.ONESHELL:

TARGETS := simple_test

CC := clang-6.0
CXX := clang++-6.0

all : ${TARGETS}
	$(info Finished ${@})

simple_test : simple_test.o
	${CXX} ${^} -std=gnu++17 -g -o ${@}

%.o : %.cc
	${CXX} -std=gnu++17 -g ${<} -c -o ${@}
