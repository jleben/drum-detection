root_dir=$1
detector=$2

if [ -z $root_dir ] || [ -z $detector ]; then
  echo "Required arguments: <samples dir> <detector program>"
  exit 1
fi

shopt -s globstar
for file in $root_dir/**
do
  #echo "File: $file"
  if [[ $file =~ .*\.wav ]]
  then
    dir=`dirname "$file"`
    #echo "Dir: $dir"
    if [[ $dir =~ $root_dir/(.*) ]]
    then
      out_dir=${BASH_REMATCH[1]}
    else
      echo "Could not get relative dir"
      exit 1
    fi
    #echo "Output dir: $out_dir"

    mkdir -p "$out_dir"
    out_file="$out_dir/`basename "$file" .wav`.onsets"
    cmd="$detector \"$file\" \"$out_file\""
    echo $cmd
    $detector "$file" "$out_file"
  fi
done
