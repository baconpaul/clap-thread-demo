/*
 * Clap Thread Demo
 *
 * Testing out some threading for the wrapper
 *
 * Copyright 2024-2025, Paul Walker and Various authors, as described in the github
 * transaction log.
 *
 * This source repo is released under the MIT license,
 *
 * The source code and license are at https://github.com/baconpaul/clap-thread-demo
 */

#include <clap/clap.h>
#include "ctd-clap-entry-impl.h"

extern "C"
{

#ifdef __GNUC__
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wattributes"
#endif

    // clang-format off
    const CLAP_EXPORT struct clap_plugin_entry clap_entry = {
        CLAP_VERSION,
        baconpaul::ctd::clap_init,
        baconpaul::ctd::clap_deinit,
        baconpaul::ctd::get_factory
    };
    // clang-format on
#ifdef __GNUC__
#pragma GCC diagnostic pop
#endif
}