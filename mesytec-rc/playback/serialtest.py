import serial, sys, time

ser = serial.Serial('/dev/ttyUSB0', 57600, bytesize = serial.EIGHTBITS,
                    parity=serial.PARITY_NONE, stopbits = 1, timeout = 0.05,
                    xonxoff=0,rtscts=0)
print ser.portstr
ser.flushInput()
ser.flushOutput()
time.sleep(0.1)

def read():
    reply = []
    while True:
        l = ser.read(2)
        if l == "mr":
            ll = ser.read(4)
            reply.append(l+ll)
            break
        ll = ser.readline()
        x = ser.read(1)
        reply.append(l+ll)
        
    reply = [r.strip('\r\n').lower() for r in reply]
    return reply

for l in open(sys.argv[1], 'r'):
    # read logged command from file
    lc, lr = l.split(' --- ')
    lr = eval(lr)
    print lc, len(lr)
    
    # send command again, comparing the output
    ser.write(lc+'\r')
    r = read()
    equal = True
    for x, y in zip(lr, r):
        equal &= ( x == y )
    if not equal:        
        print "lc='%s' => lr='%s' != '%s'" % (lc, lr, r)
    time.sleep(0.05)

ser.close()
