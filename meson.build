project('lchat', 'cpp')

gtk_dep = dependency('gtkmm-4.0')

asio = subproject('asio')
asio_dep = asio.get_variable('asio_dep')

includes = [
    'include'
]
src_server = [
    'main_server.cpp'
]
src = [
    'main.cpp'
]
executable('server',
    src_server,
            include_directories: include_directories(includes),
            dependencies: [gtk_dep,asio_dep], 
            override_options: ['cpp_std=c++20'])
executable('client',
    src,
            include_directories: include_directories(includes),
            dependencies: [gtk_dep,asio_dep], 
            override_options: ['cpp_std=c++20'])
