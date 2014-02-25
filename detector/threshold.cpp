#include "threshold.hpp"

namespace drum_detector {

Threshold::Threshold(const string & name):
  MarSystem("Threshold", name)
{
  addControl("mrs_real/threshold", 0.0, m_threshold_ctl);
}

Threshold::Threshold(const Threshold & other):
  MarSystem(other)
{
  m_threshold_ctl = getControl("mrs_real/threshold");
}

void Threshold::myUpdate( MarControlPtr )
{
  ctrl_onObservations_->setValue(1, NOUPDATE);
  ctrl_onSamples_->setValue(inSamples_, NOUPDATE);
  ctrl_osrate_->setValue(israte_, NOUPDATE);
}

void Threshold::myProcess( realvec & in, realvec & out )
{
  if (!inSamples_ || !inObservations_)
    return;

  mrs_real threshold = m_threshold_ctl->to<mrs_real>();

  for(mrs_natural s = 0; s < inSamples_; ++s)
  {
    mrs_natural sum = 0;
    for(mrs_natural o=0; o < inObservations_; o++)
    {
      if (in(o,s) > threshold)
        ++sum;
    }
    out(0,s) = sum;
  }
}

}
