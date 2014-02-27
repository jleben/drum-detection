#include <marsyas/system/MarSystemManager.h>
#include <marsyas/script/script.h>

#include <iostream>

using namespace Marsyas;
using namespace std;

int main(int argc, char *argv[])
{
  if (argc < 2)
  {
    cerr << "Missing argument: input file" << endl;
    return 1;
  }

  char *input_filename = argv[1];

  MarSystemManager mng;

  MarSystem *system = system_from_script("detector.mrs", &mng);
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
  output_control->setValue(string("detector.out"));

  while(!done_control->to<bool>())
  {
    system->tick();
  }

  return 0;
}
