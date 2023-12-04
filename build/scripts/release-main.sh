#!/bin/sh

set -eu

#
# Welcome.
#
echo ""
echo "Hello, this is the Suika2 Release Manager."

#
# Check macOS.
#
if [ -z "`uname -a | grep Darwin`" ]; then
    echo "Error: please run on macOS.";
    exit 1;
fi

#
# Guess the version number.
#
VERSION=`grep -a1 '<!-- BEGIN-LATEST -->' ../doc/readme-jp.html | tail -n1`
VERSION=`echo $VERSION | cut -d '>' -f 2 | cut -d ' ' -f 1`
VERSION=`echo $VERSION | cut -d '/' -f 2`

#
# Get the release notes.
#
NOTE_JP=`cat ../doc/readme-jp.html | awk '/BEGIN-LATEST/,/END-LATEST/' | tail -n +2 | ghead -n -1`
NOTE_EN=`cat ../doc/readme-en.html | awk '/BEGIN-LATEST/,/END-LATEST/' | tail -n +2 | ghead -n -1`

#
# Confirmation.
#
echo "Are you sure you want to release $VERSION?"
echo ""
echo "[Japanese Note]"
echo "$NOTE_JP"
echo ""
echo "[English Note]"
echo "$NOTE_EN"
echo ""
echo "(press enter to proceed)"
read str

#
# Load credentials from .env file.
#
echo "Checking for .env credentials."
if [ ! -e .env ]; then
    echo "Error: please create build/.env file."
    exit 1;
fi

FTP_LOCAL=""
FTP_USER=""
FTP_PASSWORD=""
FTP_URL=""

eval `cat .env`;

if [ -z "$FTP_LOCAL" ]; then
    echo "Error: please specify FTP_LOCAL in build/.env";
    FTP_LOCAL=`pwd`/ftp;
    mkdir -p "$FTP_LOCAL";
fi
if [ -z "$FTP_USER" ]; then
    echo "Error: please specify FTP_USER in build/.env";
    exit 1;
fi
if [ -z "$FTP_PASSWORD" ]; then
    echo "Error: please specify FTP_PASSWORD in build/.env";
    exit 1;
fi
if [ -z "$FTP_URL" ]; then
    echo "Error: please specify FTP_URL in build/.env";
    exit 1;
fi

#
# Make a temporary directory for release binaries.
#
RELEASETMP=`pwd`/release-tmp
echo "Creating a temporary directory release-tmp."
rm -rf $RELEASETMP && mkdir $RELEASETMP
echo "$RELEASETMP created."

#
# Update the macOS project version
#

cd all-macos
./update-version.sh $VERSION
git add suika.xcodeproj/project.pbxproj ../../doc/readme-jp.html ../../doc/readme-en.html
git commit -m "doc: update the version number to $VERSION" || true
git push github master
cd ..

#
# Build suika.exe
#
echo "Building suika.exe"
cd engine-windows-x86
rm -f *.o
if [ ! -e libroot ]; then ./build-libs.sh; fi
make -j8
cp suika.exe $RELEASETMP/
cd ../

#
# Build suika-pro.exe
#
echo "Building suika-pro.exe"
cd pro-windows-x86
rm -f *.o
if [ ! -e libroot ]; then cp -Rav ../engine-windows-x86/libroot .; fi
make -j8
cp suika-pro.exe $RELEASETMP/
cd ../

#
# Build suika-64.exe
#
echo "Building suika-64.exe"
cd engine-windows-x86_64
rm -f *.o
if [ ! -e libroot ]; then ./build-libs.sh; fi
make -j8
cp suika-64.exe $RELEASETMP/
cd ../

#
# Build suika-arm64.exe
#
echo "Building suika-arm64.exe"
cd engine-windows-arm64
rm -f *.o
if [ ! -e libroot ]; then ./build-libs.sh; fi
make -j8
cp suika-arm64.exe $RELEASETMP/
cd ../

#
# Build suika-capture.exe
#
echo "Building suika-capture.exe"
cd capture-windows-x86
rm -f *.o
if [ ! -e libroot ]; then cp -Rav ../engine-windows-x86/libroot .; fi
make -j8
cp suika-capture.exe $RELEASETMP/
cd ../

#
# Build suika-replay.exe
#
echo "Building suika-replay.exe"
cd replay-windows-x86
rm -f *.o
if [ ! -e libroot ]; then cp -Rav ../engine-windows-x86/libroot .; fi
make -j8
cp suika-replay.exe $RELEASETMP/
cd ../

#
# Build suika-linux
#
# if [ ! -z "`uname | grep Linux`" ]; then
#     echo "Building suika-linux";
#     cd engine-linux-x86_64;
#     rm -f *.o;
#     if [ ! -e libroot ]; then ./build-libs.sh; fi
#     make -j8;
#     cp suika $RELEASETMP/suika-linux;
#     cd ../;
# else
#     touch $RELEASETMP/suika-linux
# fi

#
# Build Wasm files
#
echo "Building Wasm files."
cd engine-wasm
make clean
make
cp html/index.html html/index.js html/index.wasm $RELEASETMP/
cd ../

#
# Build Android source tree
#
echo "Building Android source tree."
cd engine-android
./make-src.sh
cd ..

#
# Build iOS source tree
#
echo "Building iOS source tree."
cd engine-ios
./make-src.sh
cd ..

#
# Sign main exe files.
#
echo "Signing the Windows apps."
sign.sh "$RELEASETMP/suika-pro.exe"
sign.sh "$RELEASETMP/suika.exe"
sign.sh "$RELEASETMP/suika-64.exe"
sign.sh "$RELEASETMP/suika-arm64.exe"
sign.sh "$RELEASETMP/suika-capture.exe"
sign.sh "$RELEASETMP/suika-replay.exe"

#
# Build macOS apps.
#
echo "Building macOS apps."
cd all-macos
make main pro
cp mac.dmg mac-pro.dmg "$RELEASETMP/"
cd ..

#
# Make a main release file.
#
echo "Creating a main release file."

# Remove .DS_Store
find games -name '.*' | xargs rm

# Main ZIP
rm -rf suika2
mkdir suika2
cp ../doc/readme-jp.html suika2/readme-jp.html
cp ../doc/readme-en.html suika2/readme-en.html
mkdir suika2/.vscode
cp -v ../tools/snippets/jp-normal/plaintext.code-snippets suika2/.vscode/
cp -v "$RELEASETMP/suika-pro.exe" suika2/
cp -R ../games suika2/
mkdir suika2/tools
cp -v "$RELEASETMP/suika.exe" suika2/tools/
cp -v "$RELEASETMP/mac.dmg" suika2/tools/
cp -v "$RELEASETMP/mac-pro.dmg" suika2/tools/
cp -v "$RELEASETMP/suika-capture.exe" suika2/tools/
cp -v "$RELEASETMP/suika-replay.exe" suika2/tools/
cp -v "$RELEASETMP/suika-64.exe" suika2/tools/
cp -v "$RELEASETMP/suika-arm64.exe" suika2/tools/
#cp -v "$RELEASETMP/mac-capture.dmg" suika2/tools/
#cp -v "$RELEASETMP/mac-replay.dmg" suika2/tools/
#cp -v "$RELEASETMP/suika-linux" suika2/tools/
mkdir suika2/tools/web
cp -v "$RELEASETMP/index.html" suika2/tools/web/
cp -v "$RELEASETMP/index.js" suika2/tools/web/
cp -v "$RELEASETMP/index.wasm" suika2/tools/web/
cp -R engine-android/android-src suika2/tools/
cp -R engine-ios/ios-src suika2/tools/
cp -R ../tools/installer suika2/tools/
zip -r "$RELEASETMP/suika2-$VERSION.zip" suika2
rm -rf suika2

#
# Upload.
#
echo "Uploading files."

# Copy the release files to FTPLOCAL directory.
cp "$RELEASETMP/suika2-$VERSION.zip" $FTP_LOCAL/

# Upload.
curl -T "$RELEASETMP/suika2-$VERSION.zip" -u "$FTP_USER:$FTP_PASSWORD" "$FTP_URL/suika2-$VERSION.zip"
echo "Upload completed."

# Update the Web site.
# echo ""
# echo "Updating the Web site:"
# SAVE_DIR=`pwd`
# cd ../doc/web && ./update-templates.sh && ./update-version.sh && ./upload.sh
# cd "$SAVE_DIR"
