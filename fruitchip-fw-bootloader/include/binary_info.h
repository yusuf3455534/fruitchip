#pragma once

#include <pico/binary_info.h>

#include <git_version.h>

bi_decl(bi_program_name("fruitchip-fw-bootloader"));
bi_decl(bi_program_version_string(GIT_REV));
