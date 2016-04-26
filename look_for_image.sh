find asset/ -iname "*.jpg" -type f -exec identify -format '%w %h %i\n' '{}' \; | grep -i "$1*" # 1 is a particular width or height (should use awk since it's wayyy better for
# this kind of thing
