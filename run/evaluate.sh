evaluator=$1
ref_dir=$2
eval_dir=$3
map_file=$4

out_file=evaluation

echo "" > evaluation

if [ -z $ref_dir ] || [ -z $eval_dir ] || [ -z $map_file ] || [ -z $evaluator ]; then
  echo "Required arguments: <evaluator program> <samples dir> <evaluated dir> <map file>"
  exit 1
fi

shopt -s globstar

for ref_file in $ref_dir/**
do
  echo "File: $ref_file"
  if [[ $ref_file =~ .*\.mid ]]
  then
    dir=`dirname "$ref_file"`
    #echo "Dir: $dir"
    if [[ $dir =~ $ref_dir/(.*) ]]
    then
      rel_dir=${BASH_REMATCH[1]}
    else
      echo "Could not get relative dir"
      exit 1
    fi

    file_base=`basename "$ref_file" .mid`
    object_file="$eval_dir/$rel_dir/$file_base.onsets"

    echo "" >> $out_file
    echo "$ref_file <> $object_file" >> $out_file
    echo ""  >> $out_file
    "$evaluator" -m "$map_file" -o 40 -d 20 "$ref_file" "$object_file" >> $out_file
  fi
done
