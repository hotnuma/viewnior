src_inc = include_directories('.')

viewnior_sources = [
    'main.c',
    'uni-anim-view.c',
    'uni-cache.c',
    'uni-dragger.c',
    'uni-exiv2.cpp',
    'uni-image-view.c',
    'uni-nav.c',
    'uni-scroll-win.c',
    'uni-utils.c',
    'vnr-crop.c',
    'vnr-file.c',
    'vnr-message-area.c',
    'vnr-prefs.c',
    'vnr-properties-dialog.c',
    'vnr-tools.c',
    'xfce-filename-input.c',
    'vnr-window.c',
]

marshal = 'uni-marshal'

viewnior_sources += gnome.genmarshal(
  marshal,
  sources: marshal + '.list',
  prefix: 'uni_marshal',
  internal: true
)

nautilus = executable(
  'viewnior',
  viewnior_sources,
  include_directories: viewnior_include_dirs,
  dependencies: viewnior_deps,
  install: true
)
