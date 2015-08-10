echo " _  _  __  ____  ____  ____ "
echo "/ )( \\(  )(  _ \\(  __)(  _ \\"
echo "\\ \\/ / )(  ) __/ ) _)  )   /"
echo " \\__/ (__)(__)  (____)(__\\_)"
echo "============================"
echo "         INSTALLING         "
echo "============================"

if [ "$(whoami)" != "root" ]; then
	echo "Run this as root!"
	exit 1
fi

echo " - Downloading Viper:"

wget "http://fatquack.net/viper-src.zip" -O "./viper-src.zip" 2&>1 >/dev/null

if [ -f "./viper-src.zip" ]
then
	echo -e "\t[Download Successful]"
else
	echo -e "\t[Error: could not download viper]"
	exit 1
fi

echo " - Installing:"

if [ -d "/usr/share/viper" ]
then
	rm -R /usr/share/viper/* 2&>1 >/dev/null
else
	mkdir -p "/usr/share/viper" 2&>1 >/dev/null
fi

mv ./viper-src.zip /usr/share/viper/viper-src.zip >/dev/null

OLDDIR=$PWD

cd /usr/share/viper/

unzip viper-src.zip > /dev/null
rm viper-src.zip

if [ -e "./Makefile" ]
then
	make > /dev/null
	if [ -f "./Makefile" ]
	then
		echo -e "\t[Install Successful]"
	else
		echo -e "\t[Error: viper could not be compiled on your machine]"chmod
		exit 1
	fi
else
	echo -e "\t[Error: the viper download was invalid or corrupted]"
	exit 1
fi

cd $OLDDIR

export VPRDIR=/usr/share/viper/build
export PATH=$PATH:$VPRDIR

echo "============================"
echo "      INSTALL COMPLETE      "
echo "============================"
echo "To use the compiler, type 'viper'.."
