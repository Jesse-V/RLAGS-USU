import time
big = []
for i in range(0,100):
	big.append(i)
	for k in range(0,100):
		big.append(i+k)
		print "help!"
	print "one done!!!!!!!!!!!!!!!!!!1"
	time.sleep(0.1)
for i in range(0,100):
        for k in range(0,100):
		print big[i+k]
                print "help!"
        print "one done!!!!!!!!!!!!!!!!!!1"
        time.sleep(0.1)

