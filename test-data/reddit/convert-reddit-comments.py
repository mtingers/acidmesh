import json

f = open('RC_2017-12-01')
LIMIT = 15000
count = 0
for line in f:
    count += 1
    j = json.loads(line)
    body = j['body']
    print(body)
    if count > LIMIT:
        break
    print(body)

f.close()

