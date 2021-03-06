import itertools
import random

op = ['Recv', 'Isend', 'PingPong', 'Wtime', 'Wtime', 'Iprobe', 'Test']
sizes = [int(10**random.uniform(0, 9)) for _ in range(100)]
exp = list(itertools.product(op, sizes))
exp *= 50
random.shuffle(exp)

print('operation, size')
for op, size in exp:
    print('%s, %d' % (op, size))
