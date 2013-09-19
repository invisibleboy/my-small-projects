#TOOL=/home/qali/my-small-projects/Shell/simulate.sh
TOOL=/home/qali/my-small-projects/pwl/MemoryAllocator
#TOOL=/home/qali/my-small-projects/pwl/WearCompute
#TOOL=/home/qali/my-small-projects/Shell/tool.sh
	

for DIR in *; do
	if [ "$DIR" == "." -o "$DIR" == ".." ]; then
		echo $DIR
		continue
	fi


	echo "<< Deal with: $DIR"
	echo "<< Deal with: $DIR" >>/home/qali/log
	cd $DIR	

		
	##2. memory allocator	
	echo "$TOOL $1 $2 5 $3"
	echo "$TOOL $1 $2 5 $3" >>/home/qali/log
	$TOOL $1 $2 5 $3 2>>/home/qali/log	
	
	##3. pin simulator
	#$TOOL $1
	#chmod a+x *.sh
		
	##4. WearCompute
	#echo "$TOOL $DIR 0"
	#$TOOL $DIR 0
	#echo "$TOOL $DIR 1"
	#$TOOL $DIR 1
	#echo "$TOOL $DIR 2"
	#$TOOL $DIR 2

	##3. memory allocator
	#$TOOL 1024 5 results.txt
	
	if [ $? != 0 ]; then
		echo "Error in $DIR"
			exit
	fi
	cd ..
done

if [ $? -ne 0 ]; then
	exit 
fi

