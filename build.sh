# should have been executed by sh

set -e

if [ $(uname) != "Linux" ]; then
    echo "Rust breakpad glue not supported on non-Linux platforms!"
    exit 1
fi

./configure
make -j4
if test "$CXX" = ""; then
    CXX=c++
fi

$CXX -fPIC -c -static -pthread -I src rust_breakpad_linux.cc
ar -M < link.mri

cp librust_breakpad_client.a "$OUT_DIR"
