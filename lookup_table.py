import sys
import itertools

if len(sys.argv) < 3:
  sys.exit("Missing args - argv[1]: bucket_size, argv[2]: number_of_bucket_counters")

bucket_size = int(sys.argv[1])
number_of_bucket_counters = int(sys.argv[2])

table = itertools.combinations(range(bucket_size), number_of_bucket_counters)
table = [combo for combo in table]

f = open('lookup_table.txt', 'w')

for combo in table:
    formatted = str(combo)[1:-1].replace(' ', '')
    single = formatted.split(',')
    for s in single:
      f.write(s + '\n')

f.close()
