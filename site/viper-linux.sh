echo " _  _  __  ____  ____  ____ "
echo "/ )( \\(  )(  _ \\(  __)(  _ \\"
echo "\\ \\/ / )(  ) __/ ) _)  )   /"
echo " \\__/ (__)(__)  (____)(__\\_)"
echo "============================"
echo "         INSTALLING         "
echo "============================"

if [ "$(whoami)" = "root" ]; then
	echo "Please don't run this installer as root or with sudo"
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
	sudo rm -R /usr/share/viper/* 2&>1 >/dev/null
else
	sudo mkdir -p "/usr/share/viper" 2&>1 >/dev/null
fi

sudo mv ./viper-src.zip /usr/share/viper/viper-src.zip >/dev/null

OLDDIR=$PWD

cd /usr/share/viper/

sudo unzip viper-src.zip > /dev/null
sudo rm viper-src.zip

if [ -e "./Makefile" ]
then
	sudo make > /dev/null
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

/usr/share/viper/envsetup.sh > /dev/null

echo "============================"
echo "      INSTALL COMPLETE      "
echo "============================"
echo "To use the compiler, type 'viper'.."
