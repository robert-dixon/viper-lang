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

curl "http://fatquack.net/viper-src.zip" -o "./viper-src.zip" &> /dev/null

if [ -f "./viper-src.zip" ]
then
	echo "\t[Download Successful]"
else
	echo "\t[Error: could not download viper]"
	exit 1
fi

echo " - Installing:"

if [ -d "/usr/share/viper" ]
then
	sudo rm -R /usr/share/viper/* &> /dev/null
else
	sudo mkdir -p "/usr/share/viper"  &> /dev/null
fi

sudo mv ./viper-src.zip /usr/share/viper/viper-src.zip  &> /dev/null

OLDDIR=$PWD

cd /usr/share/viper/ &> /dev/null
sudo unzip viper-src.zip &> /dev/null
sudo rm viper-src.zip  &> /dev/null

if [ -f "./Makefile" ]
then
	sudo make &> /dev/null
	if [ -f "./Makefile" ]
	then
		echo "\t[Install Successful]"
	else
		echo "\t[Error: viper could not be compiled on your machine]"
		exit 1
	fi
else
	echo "\t[Error: the viper download was invalid or corrupted]"
	exit 1
fi

cd $OLDDIR

/usr/share/viper/envsetup.sh > /dev/null

osascript -e 'display notification "The Viper compiler is successfully downloaded and installed." with title "Viper Installed"'

echo "\t[Configuration Complete]"
echo "============================"
echo "      INSTALL COMPLETE      "
echo "============================"
echo "To use the compiler, type 'viper'.."