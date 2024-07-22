project('lchat', 'cpp')

gtk_dep = dependency('gtkmm-4.0')

asio = subproject('asio')
asio_dep = asio.get_variable('asio_dep')

includes = [
    'include'
]
src = [
    'main.cpp'
]
exe = executable('app',
    src,
            include_directories: include_directories(includes),
            dependencies: [gtk_dep,asio_dep], 
            override_options: ['cpp_std=c++20'])
