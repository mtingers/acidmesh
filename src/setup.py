from distutils.core import setup, Extension
# export ARCHFLAGS="-arch x86_64

def main():
    setup(
        name='acidmesh',
        version='1.0.1',
        description='AcidMesh',
        author='Matth Ingersoll',
        author_email='matth@mtingers.com',
        packages=['acidmesh',],
        license='GPLv3',
        url='https://github.com/mtingers/acidmesh',
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
            Extension('cacidmesh',
                sources=['pyacidmesh.c', 'sequence.c', 'container.c', 'context.c', 'mesh.c', 'util.c', 'datatree.c'],
            )
        ]
    )

if __name__ == '__main__':
    main()
