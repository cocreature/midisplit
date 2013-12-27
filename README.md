midisplit
=========

splits midi input by channel to different outputs

#### building
gcc -std=c99 -shared -fPIC -DPIC -Wall midisplit.c -o midisplit.so

### installing
copy manifest.ttl, midisplit.ttl and midisplit.so to ~/.lv2/midisplit.lv2/
