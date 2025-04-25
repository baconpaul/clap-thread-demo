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

#include "ctd-clap-entry-impl.h"
#include "clap/plugin.h"
#include "clapwrapper/vst3.h"
#include "clapwrapper/auv2.h"

#include <iostream>
#include <cstring>
#include <string.h>
#include <clap/clap.h>

namespace baconpaul::ctd
{

extern const clap_plugin *makePlugin(const clap_host *, bool);

/*
 * Clap Factory API
 */
const clap_plugin_descriptor *getDescriptor()
{
    static const char *features[] = {CLAP_PLUGIN_FEATURE_INSTRUMENT,
                                     CLAP_PLUGIN_FEATURE_SYNTHESIZER, "Free and Open Source",
                                     "Audio Rate Modulation", nullptr};

    static char versionNum[1024];

    static clap_plugin_descriptor desc = {CLAP_VERSION,
                                          "org.baconpaul.clap-thread-demo",
                                          PRODUCT_NAME,
                                          "BaconPaul",
                                          "https://baconpaul.org",
                                          "",
                                          "",
                                          "1.0",
                                          "Threads are Hard",
                                          &features[0]};
    return &desc;
}

#define CLAP_PLUGIN_COUNT 1

uint32_t clap_get_plugin_count(const clap_plugin_factory *) { return CLAP_PLUGIN_COUNT; };
const clap_plugin_descriptor *clap_get_plugin_descriptor(const clap_plugin_factory *f, uint32_t w)
{
    if (w == 0)
    {
        return getDescriptor();
    }

    return nullptr;
}

const clap_plugin *clap_create_plugin(const clap_plugin_factory *f, const clap_host *host,
                                      const char *plugin_id)
{

    if (strcmp(plugin_id, getDescriptor()->id) == 0)
    {
        return makePlugin(host, false);
    }

    return nullptr;
}

/*
 * Clap Wrapper AUV2 Factory API
 */

static bool clap_get_auv2_info(const clap_plugin_factory_as_auv2 *factory, uint32_t index,
                               clap_plugin_info_as_auv2_t *info)
{
    if (index > 1)
        return false;

    if (index == 0)
    {
        strncpy(info->au_type, "aumu", 5); // use the features to determine the type
        strncpy(info->au_subt, "ctdD", 5);
    }

    return true;
}

/*
 * Clap Wrapper VST3 Factory API
 */
static const clap_plugin_info_as_vst3 *clap_get_vst3_info(const clap_plugin_factory_as_vst3 *f,
                                                          uint32_t index)
{
    return nullptr;
}

const void *get_factory(const char *factory_id)
{
    if (strcmp(factory_id, CLAP_PLUGIN_FACTORY_ID) == 0)
    {
        static const struct clap_plugin_factory ctd_clap_factory = {
            clap_get_plugin_count,
            clap_get_plugin_descriptor,
            clap_create_plugin,
        };
        return &ctd_clap_factory;
    }
    if (strcmp(factory_id, CLAP_PLUGIN_FACTORY_INFO_AUV2) == 0)
    {
        static const struct clap_plugin_factory_as_auv2 ctd_auv2_factory = {
            "BcPL",      // manu
            "BaconPaul", // manu name
            clap_get_auv2_info};
        return &ctd_auv2_factory;
    }
    if (strcmp(factory_id, CLAP_PLUGIN_FACTORY_INFO_VST3) == 0)
    {
        static const struct clap_plugin_factory_as_vst3 ctd_vst3_factory = {
            "BaconPaul", "https://baconpaul.org", "", clap_get_vst3_info};

        return &ctd_vst3_factory;
    }

    return nullptr;
}
bool clap_init(const char *p) { return true; }
void clap_deinit() {}
} // namespace baconpaul::ctd