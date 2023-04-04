PROJECT_ROOT=$(shell realpath .)

OUT_DIR = build-cache
GEN_DIRS += $(OUT_DIR)

PTXPROJ = ptxproj
PTXPROJ_PATH=$(PROJECT_ROOT)/$(PTXPROJ)
GEN_DIRS += $(PTXPROJ)

USERID ?= $(shell id -u)
USERID := $(USERID)
DOCKER_BUILDFLAGS += --build-arg 'USERID=$(USERID)'

ifeq ("$(USERID)","0")
$(error Do not run as root)
endif

.PHONY: all
all: builder

$(GEN_DIRS):
	mkdir -p $@

.PHONY: builder
builder: $(OUT_DIR)/tp-builder

$(OUT_DIR)/tp-builder: Dockerfile docker/build.sh Makefile | $(OUT_DIR)
	docker build $(DOCKER_BUILDFLAGS) --iidfile $@ --file $< --tag tp-builder docker

.PHONY: bash
bash: builder | $(PTXPROJ)
	docker run --rm -it --user "$(USERID)" -v "$(PTXPROJ_PATH):/home/user/ptxproj" tp-builder bash

.PHONY: get
get: builder | $(PTXPROJ)
	docker run --rm -it --user "$(USERID)" -v "$(PTXPROJ_PATH):/home/user/ptxproj" tp-builder build get

.PHONY: offline-get
offline-get: builder | $(PTXPROJ)
	docker run --rm -i --network none --user "$(USERID)" -v "$(PTXPROJ_PATH):/home/user/ptxproj" tp-builder build get

.PHONY: init
init: builder | $(PTXPROJ)
	docker run --rm -i --user "$(USERID)" -v "$(PTXPROJ_PATH):/home/user/ptxproj" tp-builder build init

.PHONY: build
build: builder | $(PTXPROJ)
	docker run --rm -it --user "$(USERID)" -v "$(PTXPROJ_PATH):/home/user/ptxproj" tp-builder build

.PHONY: offline-build
offline-build: builder | $(PTXPROJ)
	docker run --rm -it --network none --user "$(USERID)" -v "$(PTXPROJ_PATH):/home/user/ptxproj" tp-builder build

.PHONY: images
images: builder | $(PTXPROJ)
	docker run --rm -it --network none --user "$(USERID)" -v "$(PTXPROJ_PATH):/home/user/ptxproj" tp-builder build images

.PHONY: wup
wup: builder | $(PTXPROJ)
	docker run --rm -it --network none --user "$(USERID)" -v "$(PTXPROJ_PATH):/home/user/ptxproj" tp-builder build wup
