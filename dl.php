<?php

	if ($_GET['f'] == 'viper.tmLanguage')
	{
		$size = filesize("viper.tmLanguage");
		header('Content-Description: File Transfer');
		header('Content-Type: text/tmLanguage');
		header('Content-Disposition: attachment; filename="viper.tmLanguage"');
		header('Content-Transfer-Encoding: binary');
		header('Expires: 0');
		header('Cache-Control: must-revalidate, post-check=0, pre-check=0');
		header('Pragma: public');
		header('Content-Length: ' . $size);

	    echo file_get_contents("viper.tmLanguage");
	} else {
		$size = filesize("viper-install.sh");
		header('Content-Description: File Transfer');
		header('Content-Type: text/sh');
		header('Content-Disposition: attachment; filename="viper-install.sh"');
		header('Content-Transfer-Encoding: binary');
		header('Expires: 0');
		header('Cache-Control: must-revalidate, post-check=0, pre-check=0');
		header('Pragma: public');
		header('Content-Length: ' . $size);

		echo file_get_contents("viper-install.sh");
	}
?>