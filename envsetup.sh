if [ "$(whoami)" = "root" ]; then
	exit 0
fi

STARTDIR=$PWD
cd `dirname $0`
MYDIR=$PWD
cd $STARTDIR

if [ ! -e ~/.bash_profile ]
then
	touch ~/.bash_profile
fi

if [ ! -w ~/.bash_profile ]
then
	sudo chown $USER ~/.bash_profile
fi
grep -v "VPRDIR" ~/.bash_profile > temp
touch temp
rm ~/.bash_profile
mv temp ~/.bash_profile


echo "export VPRDIR=$MYDIR/build" >> ~/.bash_profile
echo "export PATH=\$PATH:\$VPRDIR" >> ~/.bash_profile


if [ ! -e ~/.profile ]
then
	touch ~/.profile
fi

if [ ! -w ~/.profile ]
then
	sudo chown $USER ~/.profile
fi

grep -v "VPRDIR" ~/.profile > temp
touch temp
rm ~/.profile
mv temp ~/.profile

echo "export VPRDIR=$MYDIR/build" >> ~/.profile
echo "export PATH=\$PATH:\$VPRDIR" >> ~/.profile

source ~/.profile
