#!/bin/sh

scriptpath=$(dirname `realpath $0`)/

echo "Generating Headers..."

outdir=
folder=

until [ $#  -eq 0 ]; do
	if [ "x$1" = "x-o" ]; then
		shift
		outdir=$1
		if [ ! -d "$outdir" ]; then
			mkdir -p "$outdir";
		fi
	elif [ "x$1" = "x-f" ]; then
		shift
		folder=$1
		if [ ! -d "$folder" ]; then
			echo "folder $1 does not exist. Exiting..."
			exit 1
		fi
	elif [ "x$1" = "x-h" ]; then
		#Todo: help text
		:
	else
		echo "Invalid Argument: $1"
	fi
	shift
done

if [ "x$outdir" = "x" ]; then
	outdir=$scriptpath
fi

if [ "x$folder" = "x" ]; then
	folder=$scriptpath
fi

generateOnly=false;
if [ "x$outdir" = "x$folder" ]; then
	generateOnly=true;
fi

function processSingleFile() {
	echo "Copying file $1"
	cp -f $1 $outdir/
	classes=`cat $1 | sed -n -E "s/^(class|struct) +[A-Z]*_EXPORT +([A-Za-z0-9_]+)( .*)?/\2/p"`
	for x in $classes; do
		echo "Generating Header for $x"
		echo "#include \"`basename $1`\"" > $outdir/$x
	done
}

for x in `ls -1 $folder/*.h`; do
	processSingleFile $x
done

echo "Header Generation Complete"

exit 0
