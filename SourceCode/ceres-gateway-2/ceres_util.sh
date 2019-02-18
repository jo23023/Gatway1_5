
# Save pid itself
echo $$ > /var/run/ceres_util.sh.pid

while [ 1 ] 
do
	./ceres_util
done