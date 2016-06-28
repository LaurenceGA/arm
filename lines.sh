# $excluded is a regex for paths to exclude from line counting
excluded="README\|lib\|docs\|backup\|lexTest.c"

countLines(){
	# $total is the total lines of code counted
	total=0
	for file in `find -name "*.[ch]" |grep -v "$excluded"`; do
		# only count lines of code that are not commented with // and blank lines
		numLines=`cat $file | sed -r '\:(^\s*//)|(^\s*$):d' | wc -l`

		total=$(($total + $numLines))
		echo "  " $numLines $file
	done
	echo "		" $total lines in total
}

echo Source code files:
countLines
