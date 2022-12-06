# Testing bash script

./app/dns_receiver example.com receive/ 2> /dev/null & receiver=$!;

for i in {1..15};
do
  ./app/dns_sender -s 0 -u 127.0.0.1 example.com small/"$i" small 2> /dev/null;
done;
for i in {1..15};
do
  ./app/dns_sender -s 0 -u 127.0.0.1 example.com medium/"$i" medium 2> /dev/null;
done;
for i in {1..15};
do
  ./app/dns_sender -s 0 -u 127.0.0.1 example.com large/"$i" large 2> /dev/null;
done;

sleep 1;

output="";

for i in {1..15};
do
  output+=$(diff small receive/small/"$i" 2>&1 > /dev/null)
done;
for i in {1..15};
do
  output+=$(diff medium receive/medium/"$i" 2>&1 > /dev/null)
done;
for i in {1..15};
do
  output+=$(diff large receive/large/"$i" 2>&1 > /dev/null)
done;

kill $receiver > /dev/null;

if [ "$output" = "" ]
then
  echo "OK"
else
  echo "FAILED"
fi