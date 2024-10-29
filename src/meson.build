cc = meson.get_compiler('c')

executable('explode-generator', [
  'util/string.c',
  'gif_save.c',
  'gif_load.c',
  'resize.c',
  'explode.c',
  'main.c',
], dependencies : [
  dependency('raylib'),
  dependency('MagickWand'),
  cc.find_library('m'),
], install : true)
