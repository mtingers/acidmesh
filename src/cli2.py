from pprint import pprint
from random import choice
import re
from acidmesh import Mesh

UNKNOWN_ANSWER = (
    'Huh?',
    'Come again?',
    'Not sure...',
)
mesh = Mesh()

question = ''
while 1:
    question = input('> ')
    question = question.strip()
    if question.startswith('learn '):
        question = question.split('learn ', 1)[1].strip()
        depth = 0
        for word in re.split('\s+', question):
            word = word.strip()
            if not word: continue
            mesh.sequence_insert(word, depth)
            depth += 1
    elif question.strip() == 'exit':
        break
    elif question.strip() == 'link':
        mesh.link_last_contexts()
    else: #if question.startswith('get '):
        question = question.strip() #split('get ', 1)[1].strip()
        response = mesh.generate_response(question)
        if response:
            print('')
            top = response[0]['rating']
            choices = []
            for c in response:
                if c['rating'] >= top:
                    choices.append(c)
            answer = choice(choices)
            for c in response[:10]:
                print('    {} -> {}'.format(c['rating'], c['data']))
            print('        < ', answer['data'])
        else:
            print(choice(UNKNOWN_ANSWER))
            print('')
            
    #else:
    #    print(choice(UNKNOWN_ANSWER))

print('Bye!')

