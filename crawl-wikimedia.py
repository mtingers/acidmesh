#
# quick script to crawl wikipedia for data
#
import os
import re
import time
import requests

OUT = 'wiki-dl'
OUT_RAW = 'wiki-dl-raw'
API_URL = 'https://en.wikipedia.org/w/api.php'
SEARCHES = (
    "linux",
    "computer",
    "bitcoin",
    "nature",
    "weather",
    "cars",
    "electric",
    "animal",
    "art",
    "earthquake",
    "history",
    "human",
    "planet",
    "politics",
    "science",
    "storm",
)

try:
    os.mkdir(OUT)
except:
    pass
try:
    os.mkdir(OUT_RAW)
except:
    pass

titles = []
for search in SEARCHES:
    sroffset = 0
    errors = 0
    print('---- {} ----'.format(search))
    while sroffset < 120:
        query = '{}?action=query&format=json&list=search&srsearch={}&sroffset={}'.format(API_URL, search, sroffset)
        print(query)
        rc = requests.get(query)
        try:
            j = rc.json()
        except:
            errors += 1
            if errors > 2:
                break
            time.sleep(4)
            continue
        results = j['query']['search']
        for r in results:
            title = r['title'].replace(' ', '_').replace('/', '-')
            if not title in titles:
                titles.append(title)
        time.sleep(0.33)
        sroffset += 10

for title in titles:
    try:
        outname = '{}/{}.dl'.format(OUT, title)
    except Exception as err:
        print(err)
        continue
    if os.path.exists(outname):
        continue
    query = '{}?action=query&prop=extracts&exintro&explaintext&format=json&titles={}'.format(API_URL, title)
    print('---- {} ----'.format(title))
    print(query)
    rc = requests.get(query)
    j = rc.json()
    results = j['query']['pages']
    try:
        txt = results[list(results.keys())[0]]['extract']
    except:
        print('--error--')
        print(rc.text)
        time.sleep(5)
        continue
    sp = re.split('(([a-z])([\.\!\?])|([0-9])([\.\!\?]\s))', txt)
    txts = []
    for i in range(0, len(sp)-1, 6):
        a = sp[i]+sp[i+1]
        a = a.strip(' ')
        txts.append(a)
    out_delimited = '\0'.join(txts)
    fname = '{}/{}.dl'.format(OUT, title)
    fname_raw = '{}/{}.dl'.format(OUT_RAW, title)
    print('Writing: {}'.format(fname))
    with open('{}'.format(fname), 'wb') as outf:
        try:
            outf.write(out_delimited.encode('ascii', 'ignore'))
        except Exception as err:
            print('write-error: {}'.format(err))
            raise
    print('Writing: {}'.format(fname_raw))
    with open('{}'.format(fname_raw), 'wb') as outf:
        try:
            outf.write(txt.encode('ascii', 'ignore'))
        except Exception as err:
            print('write-error: {}'.format(err))
    time.sleep(0.33)
