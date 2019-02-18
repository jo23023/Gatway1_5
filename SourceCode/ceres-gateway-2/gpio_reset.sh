
# Save pid itself
echo $$ > /var/run/gpio_reset.sh.pid

while [ 1 ] 
do
	./gpio_reset
done
