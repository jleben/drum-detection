#include "scripts.hpp"

#include <marsyas/system/MarSystemManager.h>
#include <marsyas/script/script.h>
#include <marsyas/script/manager.hpp>

#include <iostream>
#include <fstream>
#include <string>
#include <iomanip>

using namespace Marsyas;
using namespace std;

struct onset
{
  float time;
  int type;
  float strength;
};

int main(int argc, char *argv[])
{
  if (argc < 3)
  {
    cerr << "Usage: <input file> <output file>" << endl;
    return 1;
  }

  char *input_filename = argv[1];
  char *output_filename = argv[2];

  detector::registerScripts();

  ScriptTranslator translator;
  MarSystem *system = translator.translateRegistered("detector.mrs");
  if (!system)
  {
    cerr << "Failure loading script!" << endl;
    return 1;
  }

  MarControlPtr input_control = system->control("input");
  MarControlPtr output_control = system->control("output");
  MarControlPtr done_control = system->control("done");

  if ( input_control.isInvalid() ||
       output_control.isInvalid() ||
       done_control.isInvalid() )
  {
    cerr << "Failure: Invalid script!" << endl;
    delete system;
    return 1;
  }

  input_control->setValue(string(input_filename));
  //output_control->setValue(string("features.out"));

  mrs_real sample_rate = system->remoteControl("sndfile/osrate")->to<mrs_real>();
  mrs_natural block_size = system->remoteControl("sndfile/onSamples")->to<mrs_natural>();
  mrs_real block_duration = block_size / sample_rate;

  MarControlPtr output = system->getControl("mrs_realvec/processedData");
  assert(!output.isInvalid());

  std::vector<onset> onsets;
  int block = 0;
  const int block_offset = 5;

  while(!done_control->to<bool>())
  {
    system->tick();

    const realvec & data = output->to<realvec>();

    assert(data.getSize() == 2);

    if (!(data(0) > 0.0))
    {
      ++block;
      continue;
    }

    mrs_real centroid = data(1);

    onset o;

    o.time = (block - block_offset + 0.5) * block_duration;

    if (centroid < 0.04)
      o.type = 0;
    else if (centroid < 0.3)
      o.type = 1;
    else
      o.type = 2;

    o.strength = 1.0;

    onsets.push_back(o);

    ++block;
  }

  string separator(",");

  ofstream out_file(output_filename);
  if (!out_file.is_open())
  {
    cerr << "Failed to open output file for writing: " << output_filename << endl;
    return 1;
  }

  for (int i = 0; i < onsets.size(); ++i)
  {
    out_file << onsets[i].time << separator
             << onsets[i].type << separator
             << onsets[i].strength
             << endl;
  }

  cout << "Done." << endl;

  return 0;
}
