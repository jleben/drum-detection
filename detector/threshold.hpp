#ifndef DRUM_DETECTOR_THRESHOLD_INCLUDED
#define DRUM_DETECTOR_THRESHOLD_INCLUDED

#include <marsyas/system/MarSystem.h>

namespace drum_detector {

using namespace Marsyas;
using std::string;

class Threshold : public MarSystem
{
public:
  Threshold(const string & name);
  Threshold(const Threshold & other);
  MarSystem *clone() const { return new Threshold(*this); }

private:
  void myUpdate( MarControlPtr );
  void myProcess( realvec & in, realvec & out );

  MarControlPtr m_threshold_ctl;
};

} // namespace drum_detector

#endif // DRUM_DETECTOR_THRESHOLD_INCLUDED
