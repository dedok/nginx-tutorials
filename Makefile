.PHONY = all

NGX_CONFIGURE = ./auto/configure
## Some versions of nginx have different path of the configure,
## following lines are handle it {{
ifeq ($(shell [ -e "$(NGX_PATH)/configure" ] && echo 1 || echo 0 ), 1)
NGX_CONFIGURE=./configure
endif
## }}

NGX_PATH    = nginx
PREFIX_PATH = $(PWD)/test-root
MODULE_PATH = $(PWD)
WZ2         = $(MODULE_PATH)/../wz2
PROJ_DIR    = $(PWD)/..

DEV_CFLAGS += -ggdb3 -O0 -Wall -Werror


all: build

build:
	$(MAKE) -C $(NGX_PATH)

configure:
	cd $(NGX_PATH) && \
		CFLAGS="$(DEV_CFLAGS)" $(NGX_CONFIGURE) \
						--with-http_addition_module \
						--prefix=$(PREFIX_PATH) \
						--add-module=$(MODULE_PATH) \
						--with-debug
	mkdir -p $(PREFIX_PATH)/conf $(PREFIX_PATH)/logs
	cp -Rf $(NGX_PATH)/conf/* $(PREFIX_PATH)/conf
	cp -f $(MODULE_PATH)/conf/nginx.conf $(PREFIX_PATH)/conf/nginx.conf
