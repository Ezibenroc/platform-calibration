import subprocess
import sys


def run_search(mpi_opt, min_size=1, max_size=1000000):
    def deadlock(size):
        args = ['mpirun', *mpi_opt, 'bp_search1', str(size)]
        try:
            subprocess.run(args, timeout=1, check=True, stdout=subprocess.DEVNULL)
        except subprocess.TimeoutExpired:
            return True
        else:
            return False

    def find_max():  # used to speed-up the search
        size = max(min_size, 128)
        while not deadlock(size):
            size *= 2
        return size

    max_size = min(find_max(), max_size)
    # we search bp such that there is no deadlock for bp and there is a deadlock for bp+1
    # invariant: bp is in [min_size, max_size]
    while min_size != max_size:
        size = (min_size + max_size)//2
        if deadlock(size):
            max_size = size - 1
        else:
            if min_size == size:
                assert max_size == min_size + 1
                if deadlock(max_size):
                    return min_size
                else:
                    return max_size
            else:
                min_size = size
    return size


if __name__ == '__main__':
    if len(sys.argv) == 1:
        sys.exit('Syntax: %s <mpi_options>' % sys.argv[0])
    bp1 = run_search(sys.argv[1:])
    print('First breakpoint: %d' % bp1)
