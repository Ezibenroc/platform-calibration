import subprocess
import sys


def deadlocked(mpi_opt, size):
    args = ['mpirun', *mpi_opt, 'bp_search1', str(size)]
    print(' '.join(args))
    try:
        subprocess.run(args, timeout=3, check=True, stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
    except subprocess.TimeoutExpired:
        return True
    else:
        return False


def corrupted(mpi_opt, size):
    args = ['mpirun', *mpi_opt, 'bp_search2', str(size)]
    print(' '.join(args))
    output = subprocess.run(args, check=True, stdout=subprocess.PIPE, stderr=subprocess.DEVNULL)
    output = output.stdout.decode('ascii').strip()
    return 'corrupted' in output


class SearchError(Exception):
    pass


def run_search(mpi_opt, test_func, min_size=1, max_size=1000000):
    if test_func(mpi_opt, min_size) or not test_func(mpi_opt, max_size):
        raise SearchError('no breakpoint found in the interval [%d, %d]' % (min_size, max_size))

    def find_max():  # used to speed-up the search
        size = min_size*2
        while size < max_size and not test_func(mpi_opt, size):
            size *= 2
        return size

    max_size = min(find_max(), max_size)
    min_size = max_size // 2
    # we search bp such that test_func returns False for bp and True for bp+1
    # invariant: bp is in [min_size, max_size]
    while min_size != max_size:
        size = (min_size + max_size)//2
        if test_func(mpi_opt, size):
            max_size = size - 1
        else:
            if min_size == size:
                assert max_size == min_size + 1
                if test_func(mpi_opt, max_size):
                    return min_size
                else:
                    return max_size
            else:
                min_size = size
    return size


if __name__ == '__main__':
    if len(sys.argv) == 1:
        sys.exit('Syntax: %s <mpi_options>' % sys.argv[0])
    try:
        bp1 = run_search(sys.argv[1:], corrupted)
    except SearchError as e:
        bp1 = str(e)
    try:
        bp2 = run_search(sys.argv[1:], deadlocked)
    except SearchError as e:
        bp2 = str(e)
    print()
    print('First breakpoint:  %s' % bp1)
    print('Second breakpoint: %s' % bp2)
