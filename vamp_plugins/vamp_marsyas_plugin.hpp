#ifndef VAMP_MARSYAS_PLUGIN_INCLUDED
#define VAMP_MARSYAS_PLUGIN_INCLUDED

#include <vamp-sdk/Plugin.h>
#include <marsyas/system/MarSystem.h>

namespace Marsyas {

class VampPlugin : public Vamp::Plugin
{

public:
    VampPlugin(const std::string & script, float inputSampleRate);
    virtual ~VampPlugin();

    bool initialise(size_t channels, size_t stepSize, size_t blockSize);
    void reset();

    std::string getIdentifier() const;
    std::string getName() const;
    std::string getDescription() const;
    std::string getMaker() const;
    std::string getCopyright() const;
    int getPluginVersion() const;

    //ParameterList getParameterDescriptors() const;

    InputDomain getInputDomain() const;
    size_t getMinChannelCount() const;
    size_t getMaxChannelCount() const;
    size_t getPreferredBlockSize() const;
    size_t getPreferredStepSize() const;

    OutputList getOutputDescriptors() const;

    //float getParameter(std::string) const;
    //void setParameter(std::string, float);

    FeatureSet process(const float *const *inputBuffers,
                       Vamp::RealTime timestamp);

    FeatureSet getRemainingFeatures();

private:
    std::string m_script_filename;
    float m_input_sample_rate;
    size_t m_channels;
    size_t m_block_size;
    size_t m_step_size;
    MarSystem *m_system;
    realvec m_input;
    realvec m_output;
};

} // namespace Marsyas

#endif // VAMP_MARSYAS_PLUGIN_INCLUDED
