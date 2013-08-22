TOOL=/home/qali/Shell/simulate.sh
	

for DIR in *; do
	if [ "$DIR" == "." -o "$DIR" == ".." -o ! -d $DIR ]; then
		echo $DIR
		continue
	fi


	echo "<< Deal with: $DIR"
	cd $DIR	
	$TOOL
	cd ..
done

if [ $? -ne 0 ]; then
	exit 
fi

