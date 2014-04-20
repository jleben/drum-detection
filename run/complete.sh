mode=$1
detect_cmd="bash ../run/detect.sh ../samples/ ../build/detector/detector"
evaluate_cmd="bash ../run/evaluate.sh ../build/performance/src/paa ../samples/ . map.csv"

echo "mode = $mode"

if [[ -z $mode ]]; then
  $detect_cmd
  $evaluate_cmd
else
  if [[ $mode == "detect" ]]; then
    $detect_cmd
  elif [[ $mode == "evaluate" ]]; then
    $evaluate_cmd
  fi
fi
