import math

f = open('inc/tables.h', 'w')
f.write('static const int16_t sinetables[253][255] = {\n\t')
for i in range(3, 256):
    f.write('{ ')
    tc = 0
    for j in range(0, i):
        tcs = (2 * math.pi) / i
        sample = int(math.sin(tc) * 4096)
        f.write('%d, ' % sample)
        tc += tcs
    for k in range(i + 1, 256):
    	f.write('0, ')
    f.write('},\n\t')
f.write('};')
f.close()

#typedef struct {
#    int16_t wave[];
#    uint8_t len;
#} sinetables_t;