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

#ifndef BACONPAUL_CLAP_THREAD_DEMO_CTD_CLAP_ENTRY_IMPL_H
#define BACONPAUL_CLAP_THREAD_DEMO_CTD_CLAP_ENTRY_IMPL_H

namespace baconpaul::ctd
{
const void *get_factory(const char *factory_id);
bool clap_init(const char *p);
void clap_deinit();
} // namespace baconpaul::ctd

#endif
