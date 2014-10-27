# Jaypack

Howdy. Jaypack is a very small and non-polished tool, made over the course of a
few hours, that helps salvage JPEG files from a very corrupted filesystem, for
example. The main idea is to bypass the filesystem altogether and read raw bytes
from the HDD surface, picking up JPEG SOI (start-of-image) and EOI
(end-of-image) markers, in hope that we will be able to capture most of the
JPEG files present on the system. (We base our assumption on the fact that
filesystems would not split these files, which is generally true.)

## What it does

Jaypack scans your hard disk surface and tries to find JPEGs there. JFIF/JPEG,
Exif and SPIFF images are supported, however you can switch some off or add new
ones fairly easily.

It also has a small serializer (the client) and deserializer (the server),
which, when piped correctly through a network, will scrape the images from one
computer and save them on another, on the network. This is very useful e.g.
when you boot from a live GNU/Linux installation and have nowhere to put the
recovered files. This is a very common scenario as you would want to use a tool
like this on computer whose hard disk/filesystem is busted, and if you're lazy
like me, you'd prefer booting kubuntu from a USB and elegantly transferring the
salvaged files over the network to connecting the HDD to another computer,
which would generally require physical labor and would involve several minutes
of standing up.

This is not a robust tool. This is not a safe tool. It worked for me and I
successfully recovered some photos from my HDD back when I originally
wrote it, but I can't guarantee it'll work for you and not torment your
pet hamster.

## Building

You need just GCC and Make. It's braindead simple. Clone this repository (via <code>git clone
...</code>), cd into that directory and just run

```
$ make
```

If you need the versions suitable for debugging (<code>jaypack-dbg</code>,
etc.) just run

```
$ make debug
```

## Usage

### Basic usage

In its simplest form, pipe some data to the jaypack utility with arguments
<code>- -</code> and it will apply the default settings and output some data
about found JPEGs to stdout:

```
$ sudo dd if=/dev/sda1 bs=8192 | ./jaypack - -
```

The first parameter to <code>jaypack</code> is the maximum size of the file jaypack will
consider, in bytes (by default 10MB), and the second one is the number of bytes
to skip at the beginning (by default 0, i.e. don't skip anything). I strongly
recommend using a number such as ~16000, it has worked quite well for me in
practice. A minus sign instead of the parameter means 'leave the
default'.

The data output will then be a sequence of lines obeying the following format:

```
<jpgtype> <offset> <size>
```

Where <code>jpgtype</code> is one of <code>jfif</code>, <code>exif</code> or
<code>spiff</code> (hint: you can pipe the output to a <code>grep exif</code> 
to get only Exif images, for instance), <code>offset</code> is the offset in
bytes from the beginning of the stream where Jaypack encountered the JPEG, and
<code>size</code> is the size in bytes of the encountered JPEG.

### Client and server

You can use these offsets and sizes to extract the real files from the
disk however you prefer. Or, you can ask the jaypack client and server to do
it for you. <code>jaypack-client</code> will take
the output of <code>jaypack</code> from stdin, extract data at those offsets
and then output this in an idiotical binary format to stdout.
<code>jaypack-server</code> will be able to take that binary format from stdin
and then save .jpg files in a folder of your choosing.

A command line could look like this:

```
$ sudo dd if=<device> skip=<offset> bs=<bs> | 
  ./jaypack - 16384 |
  sudo ./jaypack-client <device> <offset * bs> <extrasize> |
  ./jaypack-server <output-dir>/
```

Where <code>device</code> is the blockdevice you're using, <code>bs</code> is
the blocksize <code>dd</code> will be using, <code>offset</code> is an offset 
from the start of the device, measured in <code>bs</code> chunks, and
<code>offset * bs</code> is the offset in bytes obtained by first grade
arithmetics. <code>output-dir</code> is the directory where you want to output
the .jpgs; note the trailing slash in the invocation of
<code>jaypack-server</code>, it is very important as you are supplying a raw
string prefix to filenames.

#### Over the network

As mentioned earlier, this strange split between reading .jpgs from the disk
and writing them as files is so that you can start the client on a computer
where a live CD is booted, for instance, and pipe the recovered .jpgs over the
network to another computer where you can safely stash them. This can be,
especially in the case of laptops, an easier solution than getting to the hard
disk itself and connecting it somewhere else.

You can use GNU netcat to do this fairly painlessly:

**First, on the server (where you want to store the recovered .jpgs):**
```
$ nc -l -p <port> | ./jaypack-server recovered-jpegs/
```
Where <code>port</code> is the port you want to use for the connection. Doesn't
really matter as long as it's not used. You'll also want to note the LAN IP
of the server computer.

**On the client (where the data resides):**
```
$ sudo dd if=/dev/sda1 skip=131072 bs=8192 |
  ./jaypack - 16384 |
  sudo ./jaypack-client /dev/sda1 1073741824 64 |
  nc <ip> <port>
```
Where <code>ip</code> is the LAN IP of the server computer, and
<code>port</code> is the port you used earlier. If everything goes right,
you'll see your server terminal fill up with status outputs from
<code>jaypack-server</code>.

If the connection dies or something ugly happens, there is a very small chance
that this will hapen while the server is reading the offset and size bytes,
which would make it start freaking out. Don't worry, nothing horrible will
happen, you just have to remain calm and keep shooting. Kill the server, fix
the connection, and rerun the client with an appropriate offset to skip
over the .jpgs you've already processed.

### Quick note on security

Don't use this on some data that an adversary may have generated. There may be
memory leaks or buffer overflows triggered by various stuff; it's especially
stupid to publicly expose a port where <code>jaypack-server</code> is
listening. Just do it in your local LAN, without Internet access or something like
that. Up above, I also suggest running some of these commands with <code>sudo</code>.
This is also stupid and done just so it has read access to a raw blockdevice.

This is a 300 LOC project whose README probably took more time to write than
the program itself, so please take care.

## How it works

Nothing complicated, really. We scan over the bytes in a blockdevice
representing a hard disk (such as /dev/sda) or a partition (such as /dev/sda1)
in hope of collecting JPEG file SOI (start-of-image) and EOI (end-of-image)
markers, and try to save the data in between. At each point, we check for the
characteristical SOI signature: <code>FF D8 FF</code>, followed by a byte
determining the type of the image (we search only for JPEG/JFIF, SPIFF and Exif
images) and then we keep track of these, as well as EOI signatures (<code>FF
D9</code>). The algorithm sucks; it is of linear complexity and uses constant
memory but it consumes everything as a stream and acts like a braindead simple
state machine; I may fix it in the future to use some clever dynamic
programming solutions or even actually parse the JPEG header, but this is not 
necessary as in most cases the files on the HDD won't be malformed, and even if 
they are, it will be able to recover fairly successfully.

It has a limit on the maximum size of a file (by default set to 10MB, but
configurable) and a number of skip bytes, during which EOI byte sequences won't
be honored. This is to be sure we have skipped the header and gotten to image
data; because image data is guaranteed not to contain excess EOI sequences, we
are safe from cutting the image prematurely due to a bogus EOI. While skipping
bytes, if the image is small, it can happen that we entirely skip the full
JPEG, maybe even crash into another one. However, if we ever hit a new SOI
during this skip zone, we will just end the image abruptly there, output what
we've got, and start a new image from there.

The solution is not foolproof at all; in particular, it sacrifices small images
in order to catch bigger ones. The assumption is that you'd probably care a lot
more about bigger images, such as photos, than smaller ones, such as random
thumbnails buried in SQLite databases on the disk and such.

## Performance

I know it doesn't matter, but I like optimizing, so I tweaked a few parameters
to get optimal throughput on my configuration, that's around 65MB/s. I'm not
sure how good my benchmarks are; for reference, grepping through the same data
to search for the string 'JFIF' takes around the same time. All in all, it's
probably fast enough for you.

## License

MIT, see the file LICENSE.
