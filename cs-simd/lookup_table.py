#!/usr/bin/env python
# -*- coding: utf-8 -*-

import sys
import itertools

if len(sys.argv) < 3:
    sys.exit("Missing args - argv[1]: bucket_size, argv[2]: number_of_bucket_counters")

bucket_size = int(sys.argv[1])
number_of_bucket_counters = int(sys.argv[2])

table = []

if number_of_bucket_counters == 3: # default
    for i in range(8): # 0b111
        for (a,b,c) in itertools.combinations(range(bucket_size), number_of_bucket_counters):
            table.append(((a,b,c), (1-2*((i & 0b100)>>2), 1-2*((i & 0b10)>>1), 1-2*(i & 1))))

elif number_of_bucket_counters == 1:
    for i in range(2): # 0b1
        for (a) in itertools.combinations(range(bucket_size), number_of_bucket_counters):
            table.append(((a), (1-2*(i & 1),)))

elif number_of_bucket_counters == 5:
    for i in range(32): # 0b11111
        for (a,b,c,d,e) in itertools.combinations(range(bucket_size), number_of_bucket_counters):
            table.append(((a,b,c,d,e), (1-2*((i & 0b10000)>>4), 1-2*((i & 0b1000)>>3), 1-2*((i & 0b100)>>2), 1-2*((i & 0b10)>>1), 1-2*(i & 1))))

f = open('lookup_table.txt', 'w')

number_of_options = len(table)
f.write(str(number_of_options) + '\n')

for combo in table:
    indices = combo[0]
    signs = combo[1]
    vector = [0 for i in range(bucket_size)]
    
    for i in range(number_of_bucket_counters):
        index = indices[i]
        sign = signs[i]
        vector[index] = 1 if sign == 1 else 9 # 9 represents -1
    
    formatted = str(vector)[1:-1].replace(', ', '')
    f.write(formatted)

f.write("\n")

for combo in table:
    indices = combo[0]

    for i in range(number_of_bucket_counters):
        formatted = str(indices[i])
        f.write(formatted + ',')

f.write("\n")

for combo in table:
    signs = combo[1]

    for i in range(number_of_bucket_counters):
        formatted = str(signs[i] if signs[i] == 1 else 9) # 9 represents -1
        f.write(formatted + ',')

f.write("\n")

f.close()
