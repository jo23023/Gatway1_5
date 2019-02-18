
# Save pid itself
echo $$ > /var/run/ipc_handler.sh.pid

while [ 1 ] 
do
	./ipc_handler
done
