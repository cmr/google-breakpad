# should have been executed by sh
./configure
make -j4
cp src/libbreakpad.a $OUT_DIR
