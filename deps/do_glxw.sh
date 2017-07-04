#!/bin/bash
# clones and installs rikusalminen/glxw

if [ ! -n "$(which python2)" ]; then
	echo "python2 is not installed, bailing"
	exit 1
fi

rm -rf $(dirname $0)/glxw

echo "Cloning repository."
git clone https://github.com/rikusalminen/glxw
[ $? -ne 0 ] && exit $?
cd glxw
echo "Downloading glxw headers."
python2 glxw_gen.py
[ $? -ne 0 ] && exit $?
echo "Copying glxw headers."
sudo cp -rv include /usr
[ $? -ne 0 ] && exit $?
echo "All done."
