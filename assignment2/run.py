from subprocess import Popen, PIPE

def run():
    t = 20
    buffers = [256]
    producers = [i for i in range(1,7)]
    consumers = [i for i in range(1,7)]
    pt = 0.0005
    rs = [25.0, 50.0, 100.0, 150.0]
    ct1 = 0.05
    ct2 = 0.005
    pi = 0.5

    log = open("log.csv", "a")
    log.write("b,p,c,pt,rs,ct1,ct2,pi,requests_completed, producer_blocked,consumer_blocked,producer_block_time,consumer_block_time\n")
    log.flush()
    for b in buffers:
        for p in producers:
            for c in consumers:
                for rsize in rs:
                    proc = Popen(['./main', str(t), str(b), str(p), str(c), str(pt), str(rsize), str(ct1), str(ct2), str(pi)], stdin=PIPE, stdout=log, \
                    close_fds=True)#, stderr=log)
                    proc.communicate()
                    log.flush()
    log.close()

if __name__ == '__main__':
    run()
