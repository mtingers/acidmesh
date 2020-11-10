from distutils.core import setup, Extension

def main():
    setup(
        name='wordmesh',
        version='1.0.1',
        description='WordMesh',
        author='Matth Ingersoll',
        author_email='matth@mtingers.com',
        packages=['wordmesh',],
        license='GPLv3',
        url='https://github.com/mtingers/wordmesh',
        keywords=['GPT',],
        classifiers=[
            'Intended Audience :: Developers',
            'OSI Approved :: GNU General Public License v3 or later (GPLv3+)',
            'Programming Language :: Python :: 3',
            'Programming Language :: Python :: 3.5',
            'Programming Language :: Python :: 3.6',
            'Programming Language :: Python :: 3.7',
            'Programming Language :: Python :: 3.8',
            'Programming Language :: Python :: 3.9',
        ],
        ext_modules=[
            Extension('cwordmesh',
                sources=['pywordmesh.c', 'container.c', 'context.c', 'forest.c', 'util.c', 'wordbank.c'],
            )
        ]
    )

if __name__ == '__main__':
    main()
