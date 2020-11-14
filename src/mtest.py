from acidmesh import Mesh
from pprint import pprint
import glob
import sys
import os
import random
import time
import re
from random import choice

random.seed(time.time())

def fixup_reddit(txt):
    sp = re.split('(([a-z])([\.\!\?])|([0-9])([\.\!\?]\s))', txt)
    txts = []
    for i in range(0, len(sp)-1, 6):
        a = sp[i]+sp[i+1]
        a = a.strip(' ')
        txts.append(a)
    out_delimited = '\0'.join(txts)
    return out_delimited

def build(max_n):
    mesh = Mesh()

    sentences = (
        ('Where', 'are', 'we?'),
        ("Hello,", "world!"),
        ("Hello,", "how", "are", "you?"),
        ('What', 'time', 'is', 'it?'),
        ('It', 'is', 'five.'),
    )

    for sentence in sentences:
        depth = 0
        for word in sentence:
            mesh.sequence_insert(word, depth)
            depth += 1
        #mesh.link_last_contexts()

    #files = glob.glob('{}/*.dl'.format(sys.argv[1]))
    #files = glob.glob('{}/*.dl'.format('../test-data/wiki-dl'))
    dl_paths = ['../test-data/wiki-dl', '../test-data/reddit/reddit-dl']
    for dl_path in dl_paths:
        #dl_path = '../test-data/wiki-dl'
        #dl_path = '../test-data/reddit/reddit-dl'
        files = glob.glob('{}/*.dl'.format(dl_path))
        random.shuffle(files)
        files_count = len(files)
        print(files_count)
        for n, path in enumerate(files):
            bname = os.path.basename(path)
            data = open(path, 'r').read().strip() #.split('\0')
            if 'reddit' in dl_path:
                data= fixup_reddit(data)
            data = data.split('\0')
            #print(data)
            #sp = re.split('(([a-z])([\.\!\?])|([0-9])([\.\!\?]\s))', txt)
            if n % 1000 == 0:
                print('{}/{} {}'.format(n, files_count, bname))
            for d in data:
                d = d.replace('\n', ' ').strip()
                depth = 0
                for word in d.split(' '):
                    if word.strip():
                        mesh.sequence_insert(word, depth)
                        depth += 1
                mesh.link_last_contexts()
            if n > max_n:
                break

    """finds = ('Hello,', 'What', 'are', 'is')
    for find in finds:
        f1 = mesh.data_find(find);
        pprint(f1)
    """
    return mesh


# THis works but has some issues with sentences that don't quite make sense
# see test3_new below where I'm trying to figure out how to unstupid it
def test3(mesh, find):
    found = mesh.data_find(find)
    if not found:
        raise Exception('data_find failed')
    s = found['seqs']
    first = choice(s)
    if not first['ancestors']:
        pprint(s)
        raise Exception('No ancestors, this is probably a depth=0. Fixme')
    
    ancestor = choice(first['ancestors'])
    out = []
    out.append(first['data'])
    cur = first
    cur_depth = cur['depth']
    # reverse/prevs search
    while 1:
        # find a prev item with same ancestor
        prev = None
        for i in cur['prevs']:
            if ancestor in i['ancestors']:
                prev = i
                cur = prev
                break
        if not prev:
            raise Exception('noprev')
        out.append(prev['data'])
        if prev['depth'] < 1:
            print('got 0 depth')
            break
        if not prev['prevs']:
            print('no more prevs, expand search')
            #print('items:', out)
            found = mesh.data_find(prev['data'])
            s = found['seqs']
            choices = []
            for i in s:
                if ancestor in i['ancestors'] and cur_depth > i['depth']:
                    choices.append(i)
                    #prev = i
                    #cur = i
                    #break
            cur = choice(choices)
    out.reverse() 
    #print(out)
    cur = first 
    # forward/nexts search
    misses = 0
    cur_depth = first['depth']
    while 1:
        # find a next item with same ancestor
        nextx = None
        for i in cur['nexts']:
            if ancestor in i['ancestors']:
                nextx = i
                cur = nextx
                break
        if not nextx:
            raise Exception('nonext')
        out.append(nextx['data'])
        if nextx['depth'] < 1:
            print('got 0 depth')
            break
        if not nextx['nexts']:
            misses += 1
            print('no more nexts, expand search')
            #print('items:', out)
            found = mesh.data_find(nextx['data'])
            s = found['seqs']
            choices = []
            for i in s:
                if ancestor in i['ancestors'] and cur_depth < i['depth']:
                    choices.append(i)
            cur = choice(choices)
            if not cur['nexts']:
                print('Got double no nexts, break.')
                break
    print('='*80)
    print('end:')
    print(' '.join(out))
    print('')


def test3_new(mesh, find):
    # use prev_word when bruteforcing ancestors (see if in nexts/prevs)
    found = mesh.data_find(find)
    if not found:
        raise Exception('data_find failed')
    s = found['seqs']
    first = choice(s)
    if not first['ancestors']:
        pprint(s)
        raise Exception('No ancestors, this is probably a depth=0. Fixme')
    prev_word = first['data'] 
    ancestor = choice(first['ancestors'])
    out = []
    out.append(first['data'])
    cur = first
    cur_depth = cur['depth']
    # reverse/prevs search
    while 1:
        # find a prev item with same ancestor
        prev = None
        for i in cur['prevs']:
            if i['nexts']:
                if ancestor in i['ancestors'] and prev_word in [ww['data'] for ww in i['nexts']]:
                    prev = i
                    cur = prev
                    break
            else:
                if ancestor in i['ancestors']:
                    prev = i
                    cur = prev
                    break

        if not prev:
            raise Exception('noprev')
        out.append(prev['data'])
        prev_word = prev['data'] 
        if prev['depth'] < 1:
            print('got 0 depth')
            break
        if not prev['prevs']:
            print('no more prevs, expand search')
            #print('items:', out)
            found = mesh.data_find(prev['data'])
            s = found['seqs']
            choices = []
            for i in s:
                if ancestor in i['ancestors'] and cur_depth > i['depth']:
                    choices.append(i)
                    #prev = i
                    #cur = i
                    #break
            cur = choice(choices)
    out.reverse() 
    #print(out)
    cur = first 
    # forward/nexts search
    misses = 0
    cur_depth = first['depth']
    prev_word = first['data'] 
    while 1:
        # find a next item with same ancestor
        nextx = None
        for i in cur['nexts']:
            if i['prevs']:
                if ancestor in i['ancestors'] and prev_word in [ww['data'] for ww in i['prevs']]:
                    nextx = i
                    cur = nextx
                    break
            else:
                if ancestor in i['ancestors']:
                    nextx = i
                    cur = nextx
                    break
        if not nextx:
            raise Exception('nonext')
        out.append(nextx['data'])
        prev_word = nextx['data'] 
        if nextx['depth'] < 1:
            print('got 0 depth')
            break
        if not nextx['nexts']:
            misses += 1
            print('no more nexts, expand search')
            #print('items:', out)
            found = mesh.data_find(nextx['data'])
            s = found['seqs']
            choices = []
            for i in s:
                if ancestor in i['ancestors'] and cur_depth < i['depth']:
                    choices.append(i)
            cur = choice(choices)
            if not cur['nexts']:
                print('Got double no nexts, break.')
                break
    print('='*80)
    print('end:')
    print(' '.join(out))
    print('')

if __name__ == '__main__':
    mesh = build(5000)
    #test3_new(mesh, sys.argv[1]) #, do_prev=True, do_next=True))
    mesh.datatree_stats(True)
    gens = mesh.generate(sys.argv[1], sys.argv[2], 15)
    print('')
    print('-'*80)
    for g in gens:
        print('item:', g)

"""
[{'ancestors': ['What', 'How'],
'data': 'is',
'depth': 1,
'nexts': [{'ancestors': ['What'],
         'data': 'your',
         'depth': 2,
         'nexts': [{'ancestors': ['What'],
                  'data': 'problem',
                  'depth': 3,
                  'nexts': [{'ancestors': ['What'],
                           'data': 'today?',
                           'depth': 4,
                           'nexts': None}]}]},
         {'ancestors': ['What'],
         'data': 'it?',
         'depth': 2,
         'nexts': None},
         {'ancestors': ['How'],
         'data': 'he',
         'depth': 2,
         'nexts': [{'ancestors': ['How'],
                  'data': 'going',
                  'depth': 3,
                  'nexts': [{'ancestors': ['How'],
                           'data': 'to',
                           'depth': 4,
                           'nexts': [{'ancestors': ['How'],
                                    'data': 'get',
                                    'depth': 5,
                                    'nexts': [{'ancestors': ['How'],
                                             'data': 'it?.',
                                             'depth': 6,
                                             'nexts': None}]}]}]}]}],
'prevs': [{'ancestors': ['What'],
         'data': 'What',
         'depth': 0,
         'prevs': None},
         {'ancestors': ['How'],
         'data': 'How',
         'depth': 0,
         'prevs': None}]},
{'ancestors': ['What', 'The'],
'data': 'is',
'depth': 2,
'nexts': [{'ancestors': ['What'],
         'data': 'it?',
         'depth': 3,
         'nexts': None},
         {'ancestors': ['The'],
         'data': 'near.',
         'depth': 3,
         'nexts': None},
         {'ancestors': ['The'],
         'data': 'you.',
         'depth': 3,
         'nexts': None}],
'prevs': [{'ancestors': ['What'],
         'data': 'time',
         'depth': 1,
         'prevs': [{'ancestors': ['What'],
                  'data': 'What',
                  'depth': 0,
                  'prevs': None}]},
         {'ancestors': ['The'],
         'data': 'end',
         'depth': 1,
         'prevs': [{'ancestors': ['The'],
                  'data': 'The',
                  'depth': 0,
                  'prevs': None}]},
         {'ancestors': ['The'],
         'data': 'problem',
         'depth': 1,
         'prevs': [{'ancestors': ['The'],
                  'data': 'The',
                  'depth': 0,
                  'prevs': None}]}]}]
"""
