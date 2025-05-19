import socket
import psutil
import re

def get_allowed(hostname, port=443):
    allowed = []
    allowed.append(hostname)

    for r in socket.getaddrinfo(host=hostname, port=port, family=socket.AF_INET):
        allowed.append(r[4][0])

    items_str = '|'.join([re.escape(x) for x in allowed])
    return f"--allow-hosts '^({items_str}):{port}$'"

def run_harness():
    # mitmproxy --mode local:anzu_harness -s .\server\server.py
    cmd = r"mitmproxy --mode local:anzu_harness -s .\server\server.py"

    return cmd

def run_game(gamename="trackmania"):
    # find PID of the game
    pid = -1
    for p in psutil.process_iter():
        if gamename.lower() in p.name().lower():
            pid = p.pid
            break

    if pid == -1:
        pid = gamename

    cmd = f"mitmproxy --mode local:{pid} -s .\\server\\server.py " + get_allowed("gateway.prod.anzu-us.com")

    return cmd

if __name__ == '__main__':
    cmd = run_game()
    print(f"Game:\n{cmd}")
    cmd = run_harness()
    print(f"Harness:\n{cmd}")