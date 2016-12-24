from subprocess import Popen, PIPE

def run():
    t = 10
    buffers = [256]
    producers = [i for i in range(2,4)]
    consumers = [i for i in range(2,4)]

    log = open("threads_log.csv", "a")
    log.write("b,p,c,requests_completed,producer_blocked,consumer_blocked,producer_block_time,consumer_block_time\n")
    log.flush()
    for b in buffers:
        for p in producers:
            for c in consumers:
                proc = Popen(['./main', str(t), str(b), str(p), str(c)], stdin=PIPE, stdout=log, \
                close_fds=True)#, stderr=log)
                proc.communicate()
                log.flush()
    log.close()

if __name__ == '__main__':
    run()
