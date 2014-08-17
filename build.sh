# should have been executed by sh

set -e

if [ $(uname) != "Linux" ]; then
    if [ $(uname -o) != "Msys" ]; then
        echo "Rust breakpad glue not supported on non-Linux platforms!"
        exit 1
    fi
fi

if test $(uname) = "Linux"; then
    ./configure
    make -j4
    if test "$CXX" = ""; then
        CXX=c++
    fi
fi
    
if test $(uname) = "Linux"; then
    $CXX -fPIC -c -static -pthread -I src rust_breakpad_linux.cc
    ar -M < link.mri
    cp librust_breakpad_client.a "$OUT_DIR"
elif test $(uname -o) = "Msys"; then
    if rustc --version verbose | grep x86_64; then
        libdir=x64
    else
        libdir=x32
    fi
    cp $libdir/breakpad-dll.dll "$OUT_DIR"
fi
