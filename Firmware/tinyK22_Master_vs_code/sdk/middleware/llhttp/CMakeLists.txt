#
# Copyright 2024 NXP
#
# SPDX-License-Identifier: MIT
#

if (CONFIG_MCUX_COMPONENT_middleware.llhttp)
    mcux_add_source(
        SOURCES include/llhttp.h src/api.c src/http.c src/llhttp.c
    )
    mcux_add_include(
        INCLUDES include
    )
endif()
