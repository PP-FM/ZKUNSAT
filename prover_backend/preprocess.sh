FILES="$1/*"
for f in $FILES
do
  echo "processing $f"
 # ./ex1 $f
  instance="$(basename $f)"
  tmp=${instance%".cnf"}
  echo $tmp
  ./picosat-965/ex1 $f
  python3 Sort.py  "proof.TRACECHECK" > "$2/$tmp.prf"
  python3 unfold_proof.py "$2/$tmp.prf"
done
