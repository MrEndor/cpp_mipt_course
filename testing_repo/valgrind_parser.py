from collections import deque
import sys

with open("log.txt") as file:
    [last_line] = deque(file, maxlen=1) or ['']
    errors = last_line.split('ERROR SUMMARY: ')[1]
    errors_count = int(errors.split(' errors')[0])
    if errors_count:
        sys.stderr.write("Errors founded by valgrind: {0}\n".format(errors_count))
        sys.exit(1)
    else:
        "All good in valgrind log"
