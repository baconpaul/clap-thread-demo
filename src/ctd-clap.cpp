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

#include "configuration.h"
#include <clap/clap.h>
#include <chrono>

#include <clap/helpers/plugin.hh>
#include <clap/helpers/plugin.hxx>
#include <clap/helpers/host-proxy.hxx>

#include <memory>

#include <clapwrapper/vst3.h>

namespace baconpaul::ctd
{

extern const clap_plugin_descriptor *getDescriptor();

namespace clapimpl
{

static constexpr clap::helpers::MisbehaviourHandler misLevel =
    clap::helpers::MisbehaviourHandler::Ignore;
static constexpr clap::helpers::CheckingLevel checkLevel = clap::helpers::CheckingLevel::Maximal;

using plugHelper_t = clap::helpers::Plugin<misLevel, checkLevel>;

struct SixSinesClap : public plugHelper_t
{
    SixSinesClap(const clap_host *h) : plugHelper_t(getDescriptor(), h) {}
    virtual ~SixSinesClap(){};

  protected:
    bool activate(double sampleRate, uint32_t minFrameCount,
                  uint32_t maxFrameCount) noexcept override
    {
        return true;
    }

    void onMainThread() noexcept override {}

    bool implementsAudioPorts() const noexcept override { return true; }
    uint32_t audioPortsCount(bool isInput) const noexcept override { return isInput ? 0 : 1; }
    bool audioPortsInfo(uint32_t index, bool isInput,
                        clap_audio_port_info *info) const noexcept override
    {
        assert(!isInput);
        if (isInput || index > 1)
            return false;
        info->id = 8424;
        info->in_place_pair = CLAP_INVALID_ID;
        strncpy(info->name, "Main Out", sizeof(info->name));
        info->flags = CLAP_AUDIO_PORT_IS_MAIN;
        info->channel_count = 2;
        info->port_type = CLAP_PORT_STEREO;
        return true;
    }
    bool implementsAudioPortsActivation() const noexcept override { return true; }
    bool audioPortsActivationCanActivateWhileProcessing() const noexcept override { return true; }
    bool audioPortsActivationSetActive(bool is_input, uint32_t port_index, bool is_active,
                                       uint32_t sample_size) noexcept override
    {
        return true;
    }

    bool implementsNotePorts() const noexcept override { return true; }
    uint32_t notePortsCount(bool isInput) const noexcept override { return isInput ? 1 : 0; }
    bool notePortsInfo(uint32_t index, bool isInput,
                       clap_note_port_info *info) const noexcept override
    {
        assert(isInput);
        assert(index == 0);
        if (!isInput || index != 0)
            return false;

        info->id = 17252;
        info->supported_dialects =
            CLAP_NOTE_DIALECT_MIDI | CLAP_NOTE_DIALECT_MIDI_MPE | CLAP_NOTE_DIALECT_CLAP;
        info->preferred_dialect = CLAP_NOTE_DIALECT_CLAP;
        strncpy(info->name, "Note Input", CLAP_NAME_SIZE - 1);
        return true;
    }

    clap_process_status process(const clap_process *process) noexcept override
    {

        return CLAP_PROCESS_CONTINUE;
    }

    void reset() noexcept override {}

    bool implementsState() const noexcept override { return false; }
    bool stateSave(const clap_ostream *ostream) noexcept override { return true; }
    bool stateLoad(const clap_istream *istream) noexcept override { return true; }

    bool implementsParams() const noexcept override { return true; }
    uint32_t paramsCount() const noexcept override { return 1; }
    bool paramsInfo(uint32_t paramIndex, clap_param_info *info) const noexcept override
    {
        assert(paramIndex == 0);
        info->id = 123;
        strcpy(info->name, "TEST");
        info->min_value = 0.0;
        info->max_value = 1.0;
        info->default_value = 0.5;
    }
    bool paramsValue(clap_id paramId, double *value) noexcept override
    {
        *value = 0.5;
        return true;
    }
    bool paramsValueToText(clap_id paramId, double value, char *display,
                           uint32_t size) noexcept override
    {
        strcpy(display, "0.5");
        return true;
    }
    bool paramsTextToValue(clap_id paramId, const char *display, double *value) noexcept override
    {
        return false;
    }
    void paramsFlush(const clap_input_events *in, const clap_output_events *out) noexcept override
    {
    }
};

} // namespace clapimpl

const clap_plugin *makePlugin(const clap_host *h, bool multiOut)
{
    auto res = new baconpaul::ctd::clapimpl::SixSinesClap(h);
    return res->clapPlugin();
}
} // namespace baconpaul::ctd

namespace chlp = clap::helpers;
namespace bpss = baconpaul::ctd::clapimpl;

template class chlp::Plugin<bpss::misLevel, bpss::checkLevel>;
template class chlp::HostProxy<bpss::misLevel, bpss::checkLevel>;
