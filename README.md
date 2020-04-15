# tcmalloc_crash
example for gperftools issue 1066

main.C - does a dlopen and calls into the loaded .so
shlib.C - prints function call to stdout

### examples

Compiling the .so with g++-9.2 requires the dlopen'ing main() to have at least as much tcmalloc as the .so:
```
g++-9.2 -o libcrash.so -shared shlib.C -fPIC -ltcmalloc
g++-9.2 -o loader main.C -ldl

$ ./loader ./libcrash.so "hello, world"
src/tcmalloc.cc:283] Attempt to free invalid pointer 0x23c5500 
Aborted
```

```
g++-9.2 -o libcrash.so -shared shlib.C -fPIC -ltcmalloc
g++-9.2 -o loader main.C -ldl -ltcmalloc_minimal

$ ./loader ./libcrash.so "hello, world"
src/tcmalloc.cc:283] Attempt to free invalid pointer 0x2736180
Aborted
```

```
g++-9.2 -o libcrash.so -shared shlib.C -fPIC -ltcmalloc
g++-9.2 -o loader main.C -ldl -ltcmalloc

$ ./loader ./libcrash.so "hello, world"
so_entry called with hello, world
```

Interestingly, compiling the .so with g++-7.5 (default gcc on Ubuntu 18.04) does not cause a crash, even if the loading main() is not compiled with tcmalloc. However, it appears this is because tcmalloc does not get loaded when the .so is dlopen'ed.

```
g++ -o libcrash.so -shared shlib.C -fPIC -ltcmalloc
g++-9.2 -o loader main.C -ldl

$ ./loader ./libcrash.so "hello, world"
so_entry called with hello, world
```

Breaking at malloc after the dlopen and backtracing shows that the libc malloc is used:
```
Breakpoint 2, __GI___libc_malloc (bytes=1024) at malloc.c:3028
3028    malloc.c: No such file or directory.
(gdb) bt
#0  __GI___libc_malloc (bytes=1024) at malloc.c:3028
#1  0x00007ffff6ec918c in __GI__IO_file_doallocate (fp=0x7ffff7237760 <_IO_2_1_stdout_>) at filedoalloc.c:101
#2  0x00007ffff6ed9379 in __GI__IO_doallocbuf (fp=fp@entry=0x7ffff7237760 <_IO_2_1_stdout_>) at genops.c:365
#3  0x00007ffff6ed8498 in _IO_new_file_overflow (f=0x7ffff7237760 <_IO_2_1_stdout_>, ch=-1) at fileops.c:759
#4  0x00007ffff6ed69ed in _IO_new_file_xsputn (f=0x7ffff7237760 <_IO_2_1_stdout_>, data=<optimized out>, n=21) at fileops.c:1266
#5  0x00007ffff6eca977 in __GI__IO_fwrite (buf=0x7ffff6c499fe, size=1, count=21, fp=0x7ffff7237760 <_IO_2_1_stdout_>) at iofwrite.c:39
#6  0x00007ffff791fed3 in std::basic_streambuf<char, std::char_traits<char> >::sputn (__n=21, __s=<optimized out>, this=<optimized out>) at /home/jtwang/free/gcc-9.2.0.build/x86_64-pc-linux-gnu/libstdc++-v3/include/streambuf:457
#7  std::__ostream_write<char, std::char_traits<char> > (__n=21, __s=<optimized out>, __out=...) at /home/jtwang/free/gcc-9.2.0.build/x86_64-pc-linux-gnu/libstdc++-v3/include/bits/ostream_insert.h:50
#8  std::__ostream_insert<char, std::char_traits<char> > (__out=..., __s=__s@entry=0x7ffff6c499fe "so_entry called with ", __n=21) at /home/jtwang/free/gcc-9.2.0.build/x86_64-pc-linux-gnu/libstdc++-v3/include/bits/ostream_insert.h:101
#9  0x00007ffff7920208 in std::operator<< <std::char_traits<char> > (__out=..., __s=0x7ffff6c499fe "so_entry called with ") at /home/jtwang/free/gcc-9.2.0.build/x86_64-pc-linux-gnu/libstdc++-v3/include/bits/char_traits.h:335
#10 0x00007ffff6c4996c in so_entry (arg="hello, world") at shlib.C:5
#11 0x00000000004009e6 in main (argc=3, argv=0x7fffffffd808) at main.C:15
```

