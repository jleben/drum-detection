#include "delta_ratio.hpp"

#include <cmath>

namespace drum_detector {

DeltaRatio::DeltaRatio(const string & name):
  MarSystem("DeltaRatio", name)
{
  m_memory.create(inObservations_, 1);
}

DeltaRatio::DeltaRatio(const DeltaRatio & other):
  MarSystem(other)
{
  m_memory.create(inObservations_, 1);
}

void DeltaRatio::myUpdate( MarControlPtr )
{
  ctrl_onObservations_->setValue(inObservations_, NOUPDATE);
  ctrl_onSamples_->setValue(inSamples_, NOUPDATE);
  ctrl_osrate_->setValue(israte_, NOUPDATE);

  if (m_memory.getSize() != inObservations_)
  {
    m_memory.create(inObservations_, 1);
  }
}

void DeltaRatio::myProcess( realvec & in, realvec & out )
{
  if (!inSamples_)
    return;

  mrs_natural last_sample = inSamples_ - 1;

  for(mrs_natural o = 0; o < inObservations_; o++)
  {
    out(o, 0) = std::log10( in(o, 0) / m_memory(o) );
  }

  for(mrs_natural s = 1; s < inSamples_; ++s)
  {
    for(mrs_natural o=0; o < inObservations_; o++)
    {
      out(o, s) = std::log10( in(o, s) / in(o, s-1) );
    }
  }

  for(mrs_natural o=0; o < inObservations_; o++)
  {
    m_memory(o) = in(o, last_sample);
  }
}

}
