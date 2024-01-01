project('gtknode', 'c')

gnome = import('gnome')
pkg = import('pkgconfig')

gtkdep = dependency('gtk4')

sources = [
    'src/gtknode.c',
    #'src/gtknodesocket.c',
    #'src/gtknodeview.c',
]

lib = shared_library(
    'gtknode',
    sources,
    include_directories: include_directories('src'),
    dependencies: gtkdep,
    version: '0.1',
)
pkg.generate(
    lib,
    description: 'A GTK+ based widget library to create functional flow graphs',
    url: 'https://space.univie.ac.at',
)
gnome.generate_gir(
    lib,
    nsversion: '0.1',
    namespace: 'Nodes',
)
