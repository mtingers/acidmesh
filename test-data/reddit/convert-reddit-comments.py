import json

def read_data():
    f = open('RC_2008-12')
    data = {}
    for line in f:
        j = json.loads(line)
        body = j['body']
        parent_id = j['parent_id']
        link_id = j['link_id']
        if '[deleted]' == body.strip():
            continue
        if not link_id in data:
            data[link_id] = {}
        if not parent_id in data[link_id]:
            data[link_id][parent_id] = []
        data[link_id][parent_id].append(j['body'])
    f.close()
    return data

def write_files(data):
    for link_id,v in data.items():
        if link_id in v:
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
            with open('reddit-dl/{}.dl'.format(link_id), 'wb') as outf:
                for k,vv in v.items():
                    for i in vv:
                        outf.write(i.encode('ascii', 'ignore'))
                        outf.write(b'\0')

def main():
    data = read_data()
    write_files(data)

if __name__ == '__main__':
    main()
