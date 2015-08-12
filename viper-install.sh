echo " _  _  __  ____  ____  ____ "
echo "/ )( \\(  )(  _ \\(  __)(  _ \\"
echo "\\ \\/ / )(  ) __/ ) _)  )   /"
echo " \\__/ (__)(__)  (____)(__\\_)"
echo "============================"
echo "         INSTALLING         "
echo "============================"

STARTDIR=$PWD

if hash sudo 2>/dev/null; then
	SUDO="sudo"
else
	if [ $(whoami) != "root" ]; then
		echo "Please run this installer as root"
		read -p "Press any key to exit."
		exit 1;
	fi;
	SUDO=""
fi

if hash make 2>/dev/null; then
    HAS_MAKE=1
else
    HAS_MAKE=0
fi

if hash gcc 2>/dev/null; then
    HAS_GCC=1
else
    HAS_GCC=0
fi

if hash unzip 2>/dev/null; then
    HAS_ZIP=1
else
    HAS_ZIP=0
fi

if [ "$HAS_GCC" == 0 ] || [ "$HAS_MAKE" == 0 ] || [ "$HAS_ZIP" == 0 ]; then
	echo "To install viper, please make sure you have the following commands:"
	echo " - make"
	echo " - gcc"
	echo " - unzip"
	read -p "Press any key to exit."
	exit 1
fi

echo " - Downloading Viper:"

if [ -e ./viper-src.zip ]; then
	rm ./viper-src.zip
fi

if hash wget 2>/dev/null; then
	wget https://github.com/robert-dixon/viper-lang/archive/master.zip -O ./viper-src.zip &> /dev/null
else
    curl https://github.com/robert-dixon/viper-lang/archive/master.zip -o ./viper-src.zip &> /dev/null
fi

if [ ! -e ./viper-src.zip ]; then
	echo -e "\tDownload Failed"
	read -p "Press any key to exit."
	exit 1
else
	echo -e "\tDownload Complete"
fi

echo " - Installing:"

if [ -d /usr/share/viper ]; then
	$SUDO rm -r /usr/share/viper
fi

$SUDO mkdir -p /usr/share/viper

$SUDO unzip ./viper-src.zip -d /usr/share/viper &> /dev/null

$SUDO mv /usr/share/viper/viper-lang-master/* /usr/share/viper/

rm ./viper-src.zip

cd /usr/share/viper

if [ ! -e ./Makefile ]; then
	echo -e "\tThe viper download is incorrect or corrupt."
	read -p "Press any key to exit."
	exit 1
fi

$SUDO make &> /dev/null

if [ ! -e ./build/viper ]; then
	echo -e "\tViper could not be installed on this system."
	read -p "Press any key to exit."
	exit 1
fi

cd $STARTDIR

touch ~/.bash_profile
touch ~/.profile

grep -v "\/usr\/share\/viper\/build" ~/.bash_profile > temp && mv temp ~/.bash_profile

echo -e "export VPRDIR=\"/usr/share/viper/build\"" >> ~/.bash_profile
echo -e "export PATH=\$PATH:\"/usr/share/viper/build\"" >> ~/.bash_profile

grep -v "bash_profile" ~/.profile > temp && mv temp ~/.profile
echo -e ". ~/.bash_profile" >> ~/.profile

if [[ :$PATH: != *:"/usr/share/viper/build":* ]] ; then
    export PATH=$PATH:"/usr/share/viper/build"
fi

if [[ $VPRDIR != "/usr/share/viper/build" ]] ; then
    export VPRDIR="/usr/share/viper/build"
fi
	echo -e "\tInstall Complete"


echo "============================"
echo "      INSTALL COMPLETE      "
echo "============================"
echo "To use the compiler, type 'viper'.."