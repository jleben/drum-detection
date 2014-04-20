evaluator=$1
ref_dir=$2
eval_dir=$3
map_file=$4

output=evaluation

echo "" > evaluation

if [ -z $ref_dir ] || [ -z $eval_dir ] || [ -z $map_file ] || [ -z $evaluator ]; then
  echo "Required arguments: <evaluator program> <samples dir> <evaluated dir> <map file>"
  exit 1
fi

process_root() {
  for reference in `find $ref_dir -name *.mid`
  do process_reference $reference
  done
}

process_reference() {
  reference=$1
  base=`basename $reference .mid`
  object=$eval_dir/$base.onsets
  cmd="$evaluator -m $map_file -o 40 -d 20 $reference $object"
  echo $cmd
  echo "" >> $output
  echo "$reference <> $object" >> $output
  echo ""  >> $output
  $cmd >> $output
}

process_root
