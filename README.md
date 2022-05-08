# PNGDIB
 A DIB-PNG conversion library for Windows
By Jason Summers <jason1@pobox.com>
Jason has repositories at https://github.com/jsummers
Version 3.1.0 was published Jul. 2010
Web site: <http://entropymine.com/jason/pngdib/>

Licease:
This software is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
Permission is hereby granted to use, copy, modify, and distribute this source code for any purpose, without fee. 


PNGDIB is a C library that makes it easy to convert between DIB (or BMP) images and PNG images.
Sample programs are included which convert between BMP image files and PNG image files. Also included is a simple PNG viewer.
The API for version 3.x is very different from the one in previous versions. The new API is intended to make it easier to add new features.


<h2>Introduction</h2>

<p>
PNGDIB is a library that makes it easy to convert between PNG images
and Windows Device Independent Bitmaps (DIBs). PNGDIB is designed to be 
quick and easy to use, but it does not attempt to support every feature 
that one might want. If you wish use the PNG format to its full 
capabilities, you may need to use libpng directly, or write our own 
extensions to PNGDIB.
</p>


<p>When using PNGDIB to read PNG files,
it will create version 3.0 DIBs/BMPs. The DIB will be 
uncompressed, with either 1, 4, 8, or 24 bits-per-pixel.</p>

<p>(Note: A BMP image file is basically just a DIB copied directly from 
memory, with a 14-byte file header tacked onto the front. The terms DIB 
and BMP may sometimes be used almost interchangeably.)</p>


<p>When writing PNG files, PNGDIB supports
approximately version 3.0 DIBs, and also 
OS/2-style DIBs. It supports bit depths of 1, 4, 8, 16, 24, and 32 
bits-per-pixel. It supports 4 bpp and 8 bpp compressed DIBs (it uses 
Windows GDI functions to uncompress them).</p>

<p>The library itself currently consists of just two files: pngdib.c and
pngdib.h.</p>

<p>Two sample programs, png2bmp.c and bmp2png.c, are included which demonstrate 
the use of library functions. They can be compiled as "console mode" Win32 
applications.</p>

<p>A third sample, "<a href="#smview">smview</a>",
is a bare-bones Windows program that uses pngdib 
to read and display PNG files. This is intended to demonstrate that it is 
easy to add PNG support to an application that uses DIBs.</p>
</div>

<div class=uselib>
<h2>Using the library</h2>

<h4>Overview</h4>

<p>First call <b>pngdib_d2p_init</b>() or <b>pngdib_p2d_init</b>() to
create a PNGDIB
object. A few functions can accept either type of object, but most only work
with one of the two types. Call <b>pngdib_done()</b> when finished.
</p>

<p>The code samples below show an overview of the process.
</p>


<div style="background-color: #cc0">
Sample code for writing a PNG:
<pre style="background-color: #ff0; margin: 3px;">
void save_dib_to_png_file(char *png_filename,
      LPBITMAPINFOHEADER dib, int dib_size)
{
  PNGDIB *pngdib;

  pngdib = pngdib_d2p_init();
  pngdib_d2p_set_dib(pngdib,dib,dib_size,NULL,0);
  pngdib_d2p_set_png_filename(pngdib,png_filename);
  pngdib_d2p_run(pngdib);
  pngdib_done(pngdib);
}
</pre>

</div>

<div style="background-color: #cc0">
Sample code for reading a PNG:
<pre style="background-color: #ff0; margin: 3px;">
void read_png_from_file(char *png_filename)
{
  PNGDIB *pngdib;
  LPBITMAPINFOHEADER dib;

  pngdib = pngdib_p2d_init();
  pngdib_p2d_set_png_filename(pngdib,png_filename);
  pngdib_p2d_run(pngdib);
  pngdib_p2d_get_dib(pngdib,&dib,NULL);
  pngdib_done(pngdib);

  /* ... do something with dib ... */

  pngdib_p2d_free_dib(NULL,dib);
}
</pre>

</div>




<p>Unless otherwise specified, functions that return a value
return nonzero on success, and zero on failure.

<p>Note: If you are not using Unicode, "<b>TCHAR</b>" means exactly
the same thing as "<b>char</b>".


<h4>Functions that don't require an object, or that accept either type</h4>

<dl class=fdef>
<dt>int <b>pngdib_done</b>(PNGDIB *xx);
<dd>  Cleans up and frees any memory and other resources 
allocated by the library.
The only exception is the DIB that may have been created, which
must be freed by the caller.

<dt>TCHAR* <b>pngdib_get_version_string</b>(void);
<dd>Returns a pointer to a static string indicating the version,
  such as "6.7.8".

<dt>int <b>pngdib_get_version</b>(void);
<dd>  Returns an integer representing the version. For example,
  60708 would be version 6.7.8.


<dt>void  <b>pngdib_setcallback_malloc</b>(PNGDIB *xx,
        pngdib_malloc_cb_type   mallocfunc,
        pngdib_free_cb_type     freefunc,
        pngdib_realloc_cb_type  reallocfunc);<br>
<small>
&nbsp; typedef	void* (PNGDIB_DECL *pngdib_malloc_cb_type)(void *userdata, int memblksize);<br>
&nbsp; typedef	void  (PNGDIB_DECL *pngdib_free_cb_type)(void *userdata, void *memblk);<br>
&nbsp; typedef	void* (PNGDIB_DECL *pngdib_realloc_cb_type)(void *userdata, void *memblk, int memblksize);
</small>
<dd>Use your own memory allocation functions.
This is currently only used when converting PNG-to-DIB.
The pointer you set with <b>pngdib_set_userdata</b> will be
passed to your functions when they are called by PNGDIB.
<i>reallocfunc</i> is reserved for future use, and can be NULL.
Your malloc function should zero out the memory that it
allocates.


<dt>void  <b>pngdib_setcallback_pngptrhook</b>(PNGDIB *xx,
        pngdib_pngptrhook_cb_type pngptrhookfn);<br>
<small>
&nbsp; typedef	void  (PNGDIB_DECL *pngdib_pngptrhook_cb_type)(void *userdata, void *pngptr);
</small>
<dd>Set a callback function to be called immediately after the main libpng object
has been created. Cast the "pngptr" param to a "png_structp" to use it.
Not recommended if you can avoid using it, but sometimes you need a way to
call png_set_user_limits() or similar functions.


<dt>TCHAR* <b>pngdib_get_error_msg</b>(PNGDIB *xx);
<dd>Returns a pointer to an error message reflecting the
most recent error that occurred in pngdib_d2p_run or pndib_p2d_run.

<dt>void   <b>pngdib_set_userdata</b>(PNGDIB *xx, void *userdata);
<dd>The client application can associate a pointer with the
PNGDIB object, for its own use. This is useful in conjunction with
callback functions.

<dt>void*  <b>pngdib_get_userdata</b>(PNGDIB *xx);
<dd>Returns the pointer set with pngdib_set_userdata.

<dt>void <b>pngdib_set_dibalpha32</b>(PNGDIB *xx, int flag);
<dd>This is an experimental function that you probably shouldn't
use. If <i>flag</i> is 1, it tells PNGDIB to use 32-bit
DIBs, with 8 or the 32 bits used for an alpha channel
(0=transparent, 255=opaque). This is not a standard
type of DIB.


</dl>

<h4>DIB-to-PNG functions</h4>

<dl class=fdef>

<dt>PNGDIB* <b>pngdib_d2p_init</b>(void)
<dd>Returns a pointer to a new PNG-to-DIB object, which can be passed
to other functions. Returns NULL if memory allocation failed.
Note: This is actually implemented as a macro.

<dt>int  <b>pngdib_d2p_set_dib</b>(PNGDIB *d2p,
        const BITMAPINFOHEADER* pdib, int dibsize,
	    const void* pbits, int bitssize);
<dd>Tells PNGDIB where to find your DIB in memory.<br>
<small>
<i>pdib:</i> direct pointer to the beginning of your DIB's header.<br>
<i>dibsize:</i> the number of bytes starting with pdib that are safe to read.
Can be 0 if this is unknown.<br>
<i>pbits:</i> direct pointer to the beginning of the "bits" section
of your DIB. This can be NULL if it directly follows the DIB's header
and palette information.<br>
<i>bitssize:</i> the number of bytes starting with pbits that
are safe to read. Can be 0 if you don't know. This is ignored
if pbits is NULL.</small>



<dt>void <b>pngdib_d2p_set_interlace</b>(PNGDIB *d2p, int interlaced);
<dd><i>interlaced:</i> 0 (default) = write a noninterlaced PNG image;
1 = write an interlaced image.
<dt>int  <b>pngdib_d2p_set_png_filename</b>(PNGDIB *d2p, const TCHAR *fn);
<dd>Pointer to null-terminated filename of the PNG file to write.

<dt>void <b>pngdib_d2p_set_png_write_fn</b>(PNGDIB *d2p, pngdib_write_cb_type writefunc);
<dd>Use this instead of pngdib_d2p_set_png_filename to use a custom
<i>write</i> function. The pngdib_write_cb_type type is defined as:<br>
typedef	int (PNGDIB_DECL *pngdib_write_cb_type)(void *userdata, const void *buf, int nbytes);<br>
Your <i>write</i> function must consume all the bytes supplied to it, and
return the number of bytes written ('nbytes').

<dt>int  <b>pngdib_d2p_set_software_id</b>(PNGDIB *d2p, const TCHAR *s);
<dd>Pointer to a brief NULL-terminated string
identifying your application. Set to NULL (or don't call this function)
if you don't want your app to be identified in the PNG file.

<dt>void <b>pngdib_d2p_set_gamma_label</b>(PNGDIB *d2p, int flag, double file_gamma);
<dd><i>flag:</i> 0 = Do not write a gamma label to the PNG file;
1 (default): write a gamma label to the file;
<i>file_gamma:</i> gamma value to write, default = PNGDIB_DEFAULT_FILE_GAMMA =
1/2.20.
<dt>int  <b>pngdib_d2p_run</b>(PNGDIB *d2p);
<dd>Write the PNG file.

</dl>

<h4>PNG-to-DIB functions</h4>
<dl class=fdef>
<dt>PNGDIB* <b>pngdib_p2d_init</b>(void)
<dd>Returns a pointer to a new DIB-to-PNG object, which can be passed
to other functions. Returns NULL if memory allocation failed.
Note: This is actually implemented as a macro.

<dt>int  <b>pngdib_p2d_set_png_filename</b>(PNGDIB *p2d, const TCHAR *fn);
<dd>Set the filename of the input PNG file.
<dt>void <b>pngdib_p2d_set_png_memblk</b>(PNGDIB *p2d, const void *mem, int memsize);
<dd>Use this instead of pngdib_p2d_set_png_filename to have the PNG
"file" read from a memory block instead of a disk file.
You can set <i>memsize</i> to 0 if the size isn't known, but this
could lead to buffer overruns if the image isn't a valid PNG file.

<dt>void <b>pngdib_p2d_set_png_read_fn</b>(PNGDIB *p2d, pngdib_read_cb_type readfunc);
<dd>Use this instead of pngdib_p2d_set_png_filename to use a custom
<i>read</i> function. The pngdib_read_cb_type type is defined as:<br>
typedef	int (PNGDIB_DECL *pngdib_read_cb_type)(void *userdata, void *buf, int nbytes);<br>
Your read function must try to fill the buffer with the number of bytes requested,
and return the number of bytes written to it. (If not enough bytes are available,
this indicates that the PNG file is invalid.)

<dt>void <b>pngdib_p2d_set_use_file_bg</b>(PNGDIB *p2d, int flag);
<dd>Indicates whether the background color stored with the image
should be applied to transparent areas. Set flag to 1 to use
the image's background color if possible, 0 to ignore it.

<dt>void <b>pngdib_p2d_set_custom_bg</b>(PNGDIB *p2d, unsigned char r,
		  unsigned char g, unsigned char b);
<dd>Defines a background color to apply to tranparent parts of the
 image. If both this and pngdib_p2d_set_use_file_bg are called,
the file background color will take precedence if it exists.


<dt>void <b>pngdib_p2d_set_gamma_correction</b>(PNGDIB *p2d, int flag,
    double screen_gamma);
<dd><i>flag</i>: 0 (default) = do not do gamma correction;
1 = gamma correct colors when reading the PNG image.<br>
For the <i>screen_gamma</i> parameter, you can use the
PNGDIB_DEFAULT_SCREEN_GAMMA symbol, or set your own display gamma.
PNG files with no gamma label are assumed have a gamma of 1/2.20.

<dt>int  <b>pngdib_p2d_run</b>(PNGDIB *p2d);
<dd>Read the PNG file and create the DIB. <b>If this function is
successful, the caller is responsible for freeing the DIB.</b>
See pngdib_p2d_free_dib for details.

<dt>int  <b>pngdib_p2d_get_dib</b>(PNGDIB *p2d, BITMAPINFOHEADER **ppdib, int *pdibsize);
<dd>Retrieve a pointer to the DIB, and its size in bytes.
<dt>int  <b>pngdib_p2d_get_dibbits</b>(PNGDIB *p2d, void **ppbits, int *pbitsoffset, int *pbitssize);
<dd>Retrieve information about the "bits" (pixels) section of the DIB.

<dt>int  <b>pngdib_p2d_get_palette</b>(PNGDIB *p2d, RGBQUAD **ppal, int *ppaloffset, int *ppalnumcolors);
<dd>Returns information about the DIB palette.
<i>ppal</i> is a direct pointer to the palette, 
<i>ppaloffset</i> is the number of bytes from the beginning of the DIB
to the beginning of the palette,
<i>ppalnumcolors</i> is the number of colors in the palette.
Any of the parameters may be NULL if the information is not needed.
<dt>int  <b>pngdib_p2d_get_colortype</b>(PNGDIB *p2d);
<dd>Returns an integer representing the basic color type of the
original PNG image.<br>
<small>&nbsp; 0 = grayscale<br>
&nbsp; 2 = RGB<br>
&nbsp; 3 = palette color<br>
&nbsp; 4 = grayscale+alpha<br>
&nbsp; 6 = RGB+alpha</small>

<dt>int  <b>pngdib_p2d_get_bitspersample</b>(PNGDIB *p2d);
<dd>Returns the number of bits-per-sample stored in the original PNG
image. For paletted images, returns the number of bits-per-pixel.
<dt>int  <b>pngdib_p2d_get_bitsperpixel</b>(PNGDIB *p2d);
<dd>Returns the number of bits-per-pixel stored in the original PNG
image.
<dt>int  <b>pngdib_p2d_get_samplesperpixel</b>(PNGDIB *p2d);
<dd>Returns the number of samples-per-pixel stored in the original PNG
image. For paletted images, returns 1.
<dt>int  <b>pngdib_p2d_get_interlace</b>(PNGDIB *p2d);
<dd>Returns 0 if the PNG image was noniterlaced; nonzero if it was interlaced.
<dt>int  <b>pngdib_p2d_get_density</b>(PNGDIB *p2d, int *dens_x, int *dens_y, int *dens_units);
<dd>Retrieves the x and y density (a.k.a. "resolution" or "physical pixel size")
label of the PNG image. If the label exists, returns nonzero and sets
the parameters. The <i>units</i> parameter will be 1 meaning the x and y values
are in pixels per meter, or 0 meaning pixels per unspecified unit.
<dt>int  <b>pngdib_p2d_get_file_gamma</b>(PNGDIB *p2d, double *pgamma);
<dd>Retrieves the file gamma setting stored in the PNG file.
Returns nonzero if successful; 0 if the file did not contain
gamma information.
<dt>int  <b>pngdib_p2d_get_bgcolor</b>(PNGDIB *p2d, unsigned char *pr, unsigned char *pg, unsigned char *pb);
<dd>Retrieves the background color used when processing the image,
or returns 0 if none was used. If gamma correction was performed by PNGDIB, the
background color will be gamma corrected if it came from the PNG file.
<dt>void <b>pngdib_p2d_free_dib</b>(PNGDIB *p2d, BITMAPINFOHEADER *pdib);
<dd>Free the DIB memory block.
If you have not yet called pngdib_done, set <i>pdib</i> to NULL.
If you have called pngdib_done, you can only call this function
if you did <i>not</i> set your own custom memory allocation
functions -- in that case, set <i>p2d</i> to NULL.
If you did set your own memory allocation functions, then
use your own "free" function instead of this one.

