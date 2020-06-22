NAME := $(shell basename $(PWD))
export MODULE := M5
all: $(NAME)-64 $(NAME)-32
LDFLAGS += -lm
debug: all myrun
mysubmit: submit
ARG := /home/lpr/Downloads/M5-frecov.img
include ../Makefile
