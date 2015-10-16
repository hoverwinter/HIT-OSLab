#/bin/sh

string1="Sunner"
string2="Richard Stallman"
string3="This is a very very long string!"

score1=10
score2=10
score3=10

expected1="Sunner"
expected2="Richard Stallman"
expected3="Richard Stallman"

echo Testing string:$string1
./iam "$string1"
result=`./whoami`
if [ "$result" = "$expected1" ]; then
	echo PASS.
else
	score1=0
	echo FAILED.
fi
score=$score1

echo Testing string:$string2
./iam "$string2"
result=`./whoami`
if [ "$result" = "$expected2" ]; then
	echo PASS.
else
	score2=0
	echo FAILED.
fi
score=$score+$score2

echo Testing string:$string3
./iam "$string3"
result=`./whoami`
if [ "$result" = "$expected3" ]; then
	echo PASS.
else
	score3=0
	echo FAILED.
fi
score=$score+$score3

let "totalscore=$score"
echo Score: $score = $totalscore%
