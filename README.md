# GWaveTool

This was once an attempt to code a wave form editor.
With the advent of audacity, and when I met my future wife,
this was prone to die really early.


## Q&D install instructions

The build harness is far from begin clean.

For a Q&D compile run, try this to get a working binary (recommented):

	cd src
	make -f Makefile.handmade
	./gwavetool

You can also try to "configure" using these steps (may be broken):

	./bootstrap.sh
	./configure --prefix=/usr/....
	make
	make install

Since there is no project specific library / pkgconfig stuff, You may safely
install and remove the binary later.

## License

See MIT-[LICENSE](LICENSE).

	
