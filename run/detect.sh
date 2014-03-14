root_dir=$1
detector=$2

if [ -z $root_dir ] || [ -z $detector ]; then
  echo "Required arguments: <samples dir> <detector program>"
  exit 1
fi

process_root() {
  for sample in `find $root_dir -name *.wav`
  do process_sample $sample
  done
}

process_sample() {
  sample=$1
  output=`basename $sample .wav`.onsets
  cmd="$detector $sample $output"
  echo $cmd
  $cmd
}

process_root
