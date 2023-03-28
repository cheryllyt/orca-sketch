import sys
import itertools

if len(sys.argv) < 3:
  sys.exit("Missing args - argv[1]: bucket_size, argv[2]: number_of_bucket_counters")

bucket_size = int(sys.argv[1])
number_of_bucket_counters = int(sys.argv[2])

table = itertools.combinations(range(bucket_size), number_of_bucket_counters)
table = [combo for combo in table]

f = open('lookup_table.txt', 'w')

number_of_options = len(table)
f.write(str(number_of_options) + '\n')

for combo in table:
  vector = [1 if i in combo else 0 for i in range(bucket_size)]
  formatted = str(vector)[1:-1].replace(', ', '')
  f.write(formatted)

f.write("\n")

for combo in table:
  formatted = str(combo)[1:-1].replace(' ', '')
  f.write(formatted + ',')

f.close()
