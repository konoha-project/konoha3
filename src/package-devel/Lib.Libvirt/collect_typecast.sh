cat libvirt_glue.c | grep  "To_*" | grep "sfp" | \
        grep -v "^static" | grep -v "_Public" | grep -v "//" | \
        cut -d ' ' -f1 | \
        sort -n | uniq
