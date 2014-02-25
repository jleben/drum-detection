#ifndef DRUM_DETECTOR_DELTA_RATIO_INCLUDED
#define DRUM_DETECTOR_DELTA_RATIO_INCLUDED

#include <marsyas/system/MarSystem.h>

namespace drum_detector {

using namespace Marsyas;
using std::string;

class DeltaRatio : public MarSystem
{
public:
  DeltaRatio(const string & name);
  DeltaRatio(const DeltaRatio & other);
  MarSystem *clone() const { return new DeltaRatio(*this); }

private:
  void myUpdate( MarControlPtr );
  void myProcess( realvec & in, realvec & out );

  realvec m_memory;
};

} // namespace drum_detector

#endif // DRUM_DETECTOR_DELTA_RATIO_INCLUDED
