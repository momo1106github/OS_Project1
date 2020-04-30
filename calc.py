import sys
import os
dir = 'output'
unit = 0.0
f_t = open(os.path.join(dir, 'TIME_MEASUREMENT_dmesg.txt'), 'r')
for line in f_t.readlines():
    start = float(line.split(' ')[-2])
    end  = float(line.split(' ')[-1])
    unit += end - start
unit /= 5000
print("a time unit is {} sec.".format(unit))

f_d = open(os.path.join(dir, sys.argv[1] + '_dmesg.txt'), 'r')
f_s = open(os.path.join(dir, sys.argv[1] + '_stdout.txt'), 'r')
# unit =  0.00193850560188
p_name = dict({})
for line in f_s.readlines():
    p_name[int(line.split(' ')[1])] = line.split(' ')[0]
for line in f_d.readlines():
    pid = int(line.split(' ')[-3])
    start = float(line.split(' ')[-2])
    end  = float(line.split(' ')[-1])
    print("process {}  runs {:.3f} in total.".format(p_name[pid], (end-start)/unit))

