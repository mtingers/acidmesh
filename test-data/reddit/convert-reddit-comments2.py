import json
from pprint import pprint

"""
 'link_id': 't3_7j6z5',
 'name': 't1_2',
 'parent_id': 't1_c06vdmu',

"""
"""
linkd_id <<< t3_7lbef
{'t1_377a': ["Next quarter's comic you mean."],
 't1_378b': ['You\'re confusing "furry" with "homoerotic".'],
>>>> 't3_7lbef': ["I look forward to next month's comic.",
              'Is it just me or is the guy who makes this often show '
              "homoerotic tendancies in his work and I'm not just talking "
              'about the jokes.',
              'VG Cats has updated?  My god, is it christmas already?',
              'RELOADING.']}

"""

f = open('RC_2008-12')
LIMIT = 150000
count = 0
ss = {}
sa = {}

par = {}

for line in f:
    count += 1
    j = json.loads(line)
    body = j['body']
    parent_id = j['parent_id']
    link_id = j['link_id']
    if '[deleted]' == body.strip():
        continue
    if not link_id in par:
        par[link_id] = {}
    if not parent_id in par[link_id]:
        par[link_id][parent_id] = []

    par[link_id][parent_id].append(j['body'])
    #if count > LIMIT:
    #    break

f.close()
for link_id,v in par.items():
    if link_id in v:
        print('_IN_')
        first = v[link_id]
        with open('reddit-dl/{}.dl'.format(link_id), 'wb') as outf:
            for i in first:
                outf.write(i.encode('ascii', 'ignore'))
                outf.write(b'\0')
            for k,vv in v.items():
                if k == link_id:
                    continue
                for i in vv:
                    outf.write(i.encode('ascii', 'ignore'))
                    outf.write(b'\0')
    else:
        print('_out_')
        with open('reddit-dl/{}.dl'.format(link_id), 'wb') as outf:
            for k,vv in v.items():
                for i in vv:
                    outf.write(i.encode('ascii', 'ignore'))
                    outf.write(b'\0')
