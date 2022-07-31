#!/usr/bin/env bash
#
# This script automates deployment of a MacOSX application bundle.
# It copies over necessary frameworks and libraries, then builds a .DMG image
# ready for distribution.
# A template for the DMG image is provided, which includes background image, 
# fancy icon placement, and symbolic links to useful folders within the .app
# bundle which would not be browsable from the in-game menu otherwise.
#
# The script cleans up after itself, except for the $TARGET.app bundle, 
# which should be cleaned manually, by the makefile, or an additionnal build
# command.
# 

if [[ "$#" -ne 3 ]];
then
  echo "Usage: `basename $0` <base> <destdir> <target>"
  exit 1
fi 

BASEDIR=$1
DESTDIR=$2
TARGET=$3

if [[ ! -f "$BASEDIR/PuyoVS.pro" ]];
then
    echo "MACOSX Deployment: Invalid $BASEDIR (base of Qt project, containing PuyoVS.pro)"
    exit 1
fi

if [[ ! -d "$DESTDIR/$TARGET.app" ]];
then
    echo "MACOSX Deployment: $TARGET.app missing in $DESTDIR/"
    exit 1
fi

echo "MACOSX Deployment: Providing Qt frameworks in application bundle..."
macdeployqt $DESTDIR/$TARGET.app/

echo "MACOSX Deployment: Copying additional files..."
cp -R $BASEDIR/Assets/* $DESTDIR/$TARGET.app/Contents/MacOS/ 

if [[ -f $DESTDIR/$TARGET-temp.dmg ]];
then
    echo "MACOSX Deployment: Removing previous temporary DMG image..."
    rm $DESTDIR/$TARGET-temp.dmg
fi

if [[ -f $DESTDIR/$TARGET.dmg ]];
then
    echo "MACOSX Deployment: Removing previous final DMG image..."
    rm $DESTDIR/$TARGET.dmg
fi

echo "MACOSX Deployment: Creating temporary DMG image..."
hdiutil create -size 100M -fs HFS+J -volname $TARGET $DESTDIR/$TARGET-temp.dmg 

mkdir $BASEDIR/BuildScripts/MacOSX/mnt/
hdiutil attach $DESTDIR/$TARGET-temp.dmg -readwrite -mountpoint $BASEDIR/BuildScripts/MacOSX/mnt/ -mount required 

echo "MACOSX Deployment: Copying bundle to DMG image..."
tar zxf $BASEDIR/BuildScripts/MacOSX/template.tar.gz -C $BASEDIR/BuildScripts/MacOSX/mnt/

mkdir -p $BASEDIR/BuildScripts/MacOSX/mnt/PuyoVS/ 
cp -R $DESTDIR/$TARGET.app $BASEDIR/BuildScripts/MacOSX/mnt/PuyoVS/ 

hdiutil detach $BASEDIR/BuildScripts/MacOSX/mnt/ 

echo "MACOSX Deployment: Compressing DMG image..."
hdiutil convert $DESTDIR/$TARGET-temp.dmg -format UDZO -imagekey zlib-level=9 -o $DESTDIR/$TARGET.dmg 

echo "MACOSX Deployment: Removing temporary files..."
rm $DESTDIR/$TARGET-temp.dmg
rmdir $BASEDIR/BuildScripts/MacOSX/mnt/

exit 0
