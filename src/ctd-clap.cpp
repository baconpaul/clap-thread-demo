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
#include <thread>

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

struct CTDClap : public plugHelper_t
{
    std::unique_ptr<std::thread> runner;
    std::atomic<bool> runnerRuns{true};
    std::atomic<bool> makeBeep{false};

    static void doRun(CTDClap *that)
    {
        int ct{0};
        CTDLOG("Starting Thread Loop");
        while (that->runnerRuns)
        {
            using namespace std::chrono_literals;
            std::this_thread::sleep_for(50ms);
            if (ct++ == 20)
            {
                that->_host.requestCallback();
                ct = 0;
            }
        }
        CTDLOG("Bailing from Thread Loop");
    }

    CTDClap(const clap_host *h) : plugHelper_t(getDescriptor(), h) { CTDLOG("Creating CTDCLap"); }
    virtual ~CTDClap()
    {
        CTDLOG("Ending CTD Clap");
        if (runner)
        {
            runnerRuns = false;
            runner->join();
        }
        CTDLOG("Runner is joined");
        runner.reset();
        CTDLOG("Runner is Reset");
    };

  protected:
    bool init() noexcept override
    {
        if (!runner)
        {
            runner = std::make_unique<std::thread>(doRun, this);
        }
        return true;
    }
    bool activate(double sampleRate, uint32_t minFrameCount,
                  uint32_t maxFrameCount) noexcept override
    {
        return true;
    }

    void onMainThread() noexcept override
    {
        CTDLOG("Callback");
        makeBeep = true;
    }

    bool implementsAudioPorts() const noexcept override { return true; }
    uint32_t audioPortsCount(bool isInput) const noexcept override { return isInput ? 0 : 1; }
    bool audioPortsInfo(uint32_t index, bool isInput,
                        clap_audio_port_info *info) const noexcept override
    {
        assert(!isInput);

        CTDLOG("Setting up audio port " << isInput << " " << index);
        info->id = 8424;
        info->in_place_pair = CLAP_INVALID_ID;
        strncpy(info->name, "Main Out", sizeof(info->name));
        info->flags = CLAP_AUDIO_PORT_IS_MAIN;
        info->channel_count = 2;
        info->port_type = CLAP_PORT_STEREO;
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

    bool priorBeep{false};
    int beepSample{0};
    float phase{0}, dphase{440.0 / 48000};
    clap_process_status process(const clap_process *process) noexcept override
    {
        // I should really use the proper exchange thing here
        static constexpr int beepLen = 4800;
        if (priorBeep != makeBeep && makeBeep)
        {
            beepSample = beepLen;
            phase = 0;
            CTDLOG("Initiating Beep");
        }

        priorBeep = makeBeep;

        if (beepSample > 0)
        {
            for (int i = 0; i < process->frames_count; ++i)
            {
                auto pb = beepSample * 1.0 / beepLen;
                double amp = 1.0;
                if (pb > 0.95)
                {
                    auto tl = 1.0 - (pb - 0.95) * 20;
                    amp = tl * tl * tl;
                }
                if (pb < 0.1)
                {
                    auto tl = pb * 10;
                    amp = tl * tl * tl;
                }
                if (beepSample < 0)
                    amp = 0;
                process->audio_outputs->data32[0][i] = amp * std::sin(2.0 * M_PI * phase);
                process->audio_outputs->data32[1][i] = process->audio_outputs->data32[0][i];
                phase += dphase;
                beepSample--;
            }
            if (beepSample <= 0)
                makeBeep = false;
        }
        else
        {
            for (int i = 0; i < process->frames_count; ++i)
            {
                process->audio_outputs->data32[0][i] = 0.f;
                process->audio_outputs->data32[1][i] = process->audio_outputs->data32[0][i];
            }
        }
        return CLAP_PROCESS_CONTINUE;
    }

    void reset() noexcept override {}

    bool implementsState() const noexcept override { return true; }
    bool stateSave(const clap_ostream *ostream) noexcept override { return true; }
    bool stateLoad(const clap_istream *istream) noexcept override { return true; }

    bool implementsParams() const noexcept override { return true; }
    uint32_t paramsCount() const noexcept override { return 1; }
    std::atomic<double> pval{0.5};
    bool paramsInfo(uint32_t paramIndex, clap_param_info *info) const noexcept override
    {
        assert(paramIndex == 0);
        info->id = 123;
        strcpy(info->name, "TEST");
        strcpy(info->module, "Params");
        info->min_value = 0.0;
        info->max_value = 1.0;
        info->default_value = 0.5;
        info->flags = CLAP_PARAM_IS_AUTOMATABLE;
        return true;
    }
    bool paramsValue(clap_id paramId, double *value) noexcept override
    {
        *value = pval.load();
        return true;
    }
    bool paramsValueToText(clap_id paramId, double value, char *display,
                           uint32_t size) noexcept override
    {
        snprintf(display, 128, "%lf", pval.load());
        return true;
    }
    bool paramsTextToValue(clap_id paramId, const char *display, double *value) noexcept override
    {
        *value = std::clamp(std::atof(display), 0.0, 1.0);
        return true;
    }
    void paramsFlush(const clap_input_events *in, const clap_output_events *out) noexcept override
    {
    }
};

} // namespace clapimpl

const clap_plugin *makePlugin(const clap_host *h, bool multiOut)
{
    auto res = new baconpaul::ctd::clapimpl::CTDClap(h);
    return res->clapPlugin();
}
} // namespace baconpaul::ctd

namespace chlp = clap::helpers;
namespace bpss = baconpaul::ctd::clapimpl;

template class chlp::Plugin<bpss::misLevel, bpss::checkLevel>;
template class chlp::HostProxy<bpss::misLevel, bpss::checkLevel>;
