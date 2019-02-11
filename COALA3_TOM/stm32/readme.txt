
Développement du logiciel pour le MCU de la carte COALA2
========================================================

Récupérer l'outil stm32flash :
----------------------------
#git clone git://git.code.sf.net/p/stm32flash/code stm32flash
ensuite on le compile
#cd stm32flash
#make

Pour écrire le binaire coala.bin sur le microcontroller:
#connecter en serial
#sudo make flash (il faut avoir l'outil stm32flash dans le repertoire courant)

#installer les libs compat 32bit:
sudo apt-get install libc6:i386 libncurses5:i386 libstdc++6:i386

Construction de la bibliothèque stlib
-------------------------------------
(dans projet)
make stlib.a

Développer pour le MCU en passant par le Beaglebone
---------------------------------------------------
Voir dans stm32/tools pour utiliser le script flashmcu.sh qui permet de re-programmer le MCU automatiquement dés que le fichier coala.bin est écrit.

Pour cross compiler stm32flash et l'utiliser sur le Beaglebone
Il faut juste définir dans CC le chemin vers le bon gcc:
#make CC=~/beaglebone/gcc-linaro-5.1-2015.08-x86_64_arm-linux-gnueabihf/bin/arm-linux-gnueabihf-gcc

Pour info, pour récupérer la chaine de cross-compilation pour le Beaglebone:
#wget -c https://releases.linaro.org/components/toolchain/binaries/latest-5.1/arm-linux-gnueabihf/gcc-linaro-5.1-2015.08-x86_64_arm-linux-gnueabihf.tar.xz

Récupérer la chaine de cross-compilation pour STM32 :
---------------------------------------------------
On télécharge l'archive gcc-arm-none-eabi-4_8-2014q3.tar.gz qui contient la chaîne de cross-compile.
#wget https://launchpad.net/gcc-arm-embedded/4.8/4.8-2014-q3-update/+download/gcc-arm-none-eabi-4_8-2014q3-20140805-linux.tar.bz2
Décompresser cette archive dans un répertoire quelconque, ensuite faire un lien symbolique :
gcc-arm-none-eabi -> gcc-arm-none-eabi-4_8-2014q3/
Vérifier que le Makefile du projet référence bien le chemin complet vers le chaine de cross-comilation
Voir cette ligne :
TOOLCHAIN = ~/stm32/gcc-arm-none-eabi/bin/
Donc pour être compatible directement il suffit de créer un répertoire stm32 dans notre home directory et de décompresser l'archive dedans.

* pour configurer l'acces a UART et cie.
=> https://github.com/cdsteinkuehler/beaglebone-universal-io
(dts : device tree source... a compiler en dtbo avec dtc)
