from subprocess import Popen, PIPE

def run():
    t = 120
    buffers = [16, 32, 64, 128, 256]
    producers = [i for i in range(10)]
    consumers = [i for i in range(10)]
    pt = 0.00001
    rs = 50.0
    ct1 = 0.5
    ct2 = 0.05
    pi = 0.5

    log = open("log.csv", "a")
    for b in buffers:
        for p in producers:
            for c in consumers:
                proc = Popen(['./producer', str(t), str(b), str(p), str(c)], stdin=PIPE, stdout=log, \
                stderr=log, close_fds=True)
                proc.communicate()
    log.flush()
    log.close()

if __name__ == '__main__':
    run()
