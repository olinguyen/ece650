from subprocess import Popen, PIPE

def run():
    t = 120
    buffers = [32, 64, 128, 256]
    producers = [i for i in range(1,20)]
    consumers = [i for i in range(1,20)]
    pt = 0.0005
    rs = [10.0, 25.0, 50.0, 100.0]
    ct1 = 0.05
    ct2 = 0.005
    pi = 0.5

    log = open("log.csv", "a")
    for b in buffers:
        for p in producers:
            for c in consumers:
                for rsize in rs:
                    proc = Popen(['./producer', str(t), str(b), str(p), str(c), str(pt), str(rsize), str(ct1), str(ct2), str(pi)], stdin=PIPE, stdout=log, \
                    stderr=log, close_fds=True)
                    proc.communicate()
                    log.flush()
    log.close()

if __name__ == '__main__':
    run()
