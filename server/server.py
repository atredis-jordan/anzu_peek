# uses mitmproxy library to intercept the traffic by PID and serve our own
# run this with `mitmproxy --mode local:<PID> -s ./server/server.py` using the harness's PID

import requests
import logging
import base64
import random
import time
import json
import os

from mitmproxy import http


AD_JSON = "./ad_inject.json"
AD_PATH = os.path.dirname(os.path.realpath(__file__)) + AD_JSON

HOURS_OFF = 1
SEC_OFF = 60 * 60 * HOURS_OFF

class AnzuInspector:
    def __init__(self):
        self.prefix = "AZ: "
        # get the gateway response
        with open(AD_PATH, "r") as fp:
            self.ads = json.load(fp)

        # could serve the media locally here too

        self.update_campaigns()

    def update_campaigns(self):
        # replace a few things
        # should I replace the campaign id and name each time?
        
        old_campaigns = self.ads["campaigns"]
        self.ads["campaigns"] = {}
        for c in old_campaigns:
            campid = random.randbytes(12).hex()
            biddid = str(base64.b64encode(random.randbytes(18)), "utf8")
            
            self.lg(f"Creating campaign {campid} {biddid}")
            
            o = old_campaigns[c]
            o["start"] = int(time.time()) - SEC_OFF,
            o["end"] = int(time.time()) + SEC_OFF,
            o["id"] = campid
            o["bid_id"] = f"bidder__{biddid}"
    
            self.ads["campaigns"][f"{campid}_bidder__{biddid}"] = o

        # update the size and sha256 of the specified images
        # we could also 

        old_imgs = self.ads["images"]
        self.ads["images"] = {}
        for i in old_imgs:
            self.update_info(old_imgs[i], "images")

        old_videos = self.ads["videos"]
        self.ads["videos"] = {}
        for v in old_videos:
            self.update_info(old_videos[v], "videos")

    def update_info(self, o, mtype):

        resp = requests.get(o["url"])

        if resp.status_code != 200:
            raise RuntimeError("Unable to get specified resource")

        size = len(resp.content)
        #sha = hashlib.sha256(resp.content).hexdigest()
        sha = random.randbytes(32).hex()

        o["size"] = size
        o["sha256"] = sha
        self.lg(f"Recalculated media sha = {sha}, size = {size}")

        # add to ads, update any campaigns
        oldid = o["id"]
        newid = f"{mtype[:-1]}.rtb.1337{random.randrange(10**12, 10**18)}"

        o["id"] = newid

        self.ads[mtype][newid] = o
        
        for c in self.ads["campaigns"]:
            for p in self.ads["campaigns"][c]["videotextures"]:
                assets = self.ads["campaigns"][c]["videotextures"][p]
                if oldid in assets:
                    assets.remove(oldid)
                    assets.append(newid)

    def lg(self, msg):
        logging.info(self.prefix + msg)

    def tcp_start(self, tflow):
        self.lg("TCP Start")
    
    def tls_start_client(self, tlsdata):
        self.lg(f"TLS Start: {tlsdata.conn.sni}")

    def tls_start_server(self, tlsdata):
        self.lg("TLS Server Start")

    def client_connected(self, client):
        self.lg("Client Connect")

    def server_connect(self, serverdata):
        self.lg("Server Connecting")

    def request(self, flow):
        self.lg(f"Request: {flow.request.method} {flow.request.pretty_url}")

        if flow.request.path.startswith("/multi_gateway/"):
            # give our own response
            #self.update_campaigns()
            resp = self.ads
            self.lg(f"Responding")

            flow.response = http.Response.make(
                200,
                bytes(json.dumps(resp), "utf8"),
            )


logging.info("Loading Anzu Addon")
addons = [AnzuInspector()]