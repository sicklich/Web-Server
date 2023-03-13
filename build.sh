set -x

mkdir -p `pwd`/Log
rm -rf `pwd`/build/*
cd `pwd`/build &&
	cmake .. &&
	make