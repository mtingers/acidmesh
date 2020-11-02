"""
# quick script to crawl wikipedia for data
# queries: en.wikipedia.org API
# saves raw data to wiki-dl-raw and formatted (\0 separated) to wiki-dl
"""
import os
import sys
import re
import time
import requests
import random

OUT = 'wiki-dl'
OUT_RAW = 'wiki-dl-raw'
API_URL = 'https://en.wikipedia.org/w/api.php'
WORDS_ALPHA_URL = 'https://raw.githubusercontent.com/dwyl/english-words/master/words_alpha.txt'


def download_dictionary():
    if not os.path.exists(WORDS_ALPHA_URL):
        r = requests.get(WORDS_ALPHA_URL)
        open('words_alpha.txt', 'wb').write(r.content)

def build_search_list(limit=200):
    all_searches = [i.strip() for i in open('words_alpha.txt').read().strip().split('\n')]
    searches = []
    while len(searches) < limit:
        r = random.choice(all_searches)
        if not r in searches:
            searches.append(r)
    return searches

def create_wiki_dirs():
    try:
        os.mkdir(OUT)
    except:
        pass
    try:
        os.mkdir(OUT_RAW)
    except:
        pass

def gather_titles(searches, sroffset_start=0, sroffset_max=400):
    titles = []
    for search in searches:
        sroffset = sroffset_start
        errors = 0
        while sroffset < sroffset_max:
            query = '{}?action=query&format=json&list=search&srsearch={}&sroffset={}'.format(API_URL, search, sroffset)
            print('GET_TITLES:', query)
            rc = requests.get(query)
            try:
                j = rc.json()
            except:
                errors += 1
                if errors > 2:
                    break
                time.sleep(4)
                continue
            try:
                results = j['query']['search']
            except:
                print('-'*80)
                print('ERROR:', rc.text)
                print('-'*80)
                continue
            for r in results:
                title = r['title'].replace(' ', '_').replace('/', '-')
                if not title in titles:
                    titles.append(title)
            time.sleep(0.33)
            sroffset += 10
    return titles

def download_and_transform_wikipage(titles):
    print('*'*80)
    for title in titles:
        try:
            outname = '{}/{}.dl'.format(OUT, title)
        except Exception as err:
            print(err)
            continue
        if os.path.exists(outname):
            print("SKIP:", title)
            continue
        query = '{}?action=query&prop=extracts&exintro&explaintext&format=json&titles={}'.format(API_URL, title)
        print('---- {} ----'.format(title))
        print('DOWNLOAD:', query)
        rc = requests.get(query)
        j = rc.json()
        results = j['query']['pages']
        try:
            txt = results[list(results.keys())[0]]['extract']
        except:
            print('ERROR:')
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

def main(limit=30, sroffset_max=100):
    create_wiki_dirs()
    download_dictionary()
    searches = build_search_list(limit=limit)
    titles = gather_titles(searches, sroffset_start=0, sroffset_max=sroffset_max)
    download_and_transform_wikipage(titles)

if __name__ == '__main__':
    main(limit=int(sys.argv[1]), sroffset_max=int(sys.argv[2]))
