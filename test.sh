for f in tests/*.txt
do
    cmake-build-debug/parse < $f > tmp
    res="$f.expected"
    echo $f
    diff $res tmp
    printf ' -------------\n'
done